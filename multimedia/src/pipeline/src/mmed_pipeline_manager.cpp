#include "mmed_pipeline_manager.h"

#include "ipc_shared_memory.h"
GstPadProbeReturn MmedPipelineManager::video_block_probe_callback(
    GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
    auto self = static_cast<MmedPipelineManager *>(user_data);

    g_print("Video branch pad blocked, detaching...\n");

    // 1. 发送 Flush 事件（清理队列中残留的数据）
    // 注意：flush_start 和 flush_stop 发送到 tee pad，会流向下游
    gst_pad_send_event(pad, gst_event_new_flush_start());
    gst_pad_send_event(pad, gst_event_new_flush_stop(FALSE));

    // 2. 停止下游所有 Element
    if (self->m_video_sink)
        gst_element_set_state(self->m_video_sink, GST_STATE_NULL);
    if (self->m_muxer)
        gst_element_set_state(self->m_muxer, GST_STATE_NULL);
    if (self->m_encoder)
        gst_element_set_state(self->m_encoder, GST_STATE_NULL);
    if (self->m_convert_video)
        gst_element_set_state(self->m_convert_video, GST_STATE_NULL);
    if (self->m_q_video)
        gst_element_set_state(self->m_q_video, GST_STATE_NULL);

    // 3. 解除链接
    GstPad *sink_pad = gst_element_get_static_pad(self->m_q_video, "sink");
    if (sink_pad) {
        gst_pad_unlink(pad, sink_pad);
        gst_object_unref(sink_pad);
    }

    // 4. 释放 Tee Pad
    gst_element_release_request_pad(self->m_tee, pad);
    gst_object_unref(pad); // 对应 get_request_pad 的引用
    self->m_tee_video_pad = nullptr;

    // 5. 从 Pipeline 中移除元素
    gst_bin_remove_many(GST_BIN(self->m_pipeline), self->m_q_video,
                        self->m_convert_video, self->m_encoder, self->m_muxer,
                        self->m_video_sink, NULL);

    // 6. 清空指针
    self->m_q_video = nullptr;
    self->m_convert_video = nullptr;
    self->m_encoder = nullptr;
    self->m_muxer = nullptr;
    self->m_video_sink = nullptr;

    g_print("MJPEG video branch detached cleanly inside callback\n");

    // 返回 REMOVE 以移除该 Probe
    return GST_PAD_PROBE_REMOVE;
}

std::string makePhotoPath(const std::string &dir) {
    using namespace std::chrono;

    auto now = system_clock::now();
    auto sec = system_clock::to_time_t(now);

    struct tm tm_now;
    localtime_r(&sec, &tm_now);

    auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

    char buf[64];
    snprintf(buf, sizeof(buf), "%04d_%02d_%02d_%02d_%02d_%02d_%03ld",
             tm_now.tm_year + 1900, tm_now.tm_mon + 1, tm_now.tm_mday,
             tm_now.tm_hour, tm_now.tm_min, tm_now.tm_sec, ms.count());

    return dir + buf + ".jpg";
}

// ----------------------------
// bus 回调
// ----------------------------
static gboolean on_bus_message(GstBus *bus, GstMessage *msg,
                               gpointer user_data) {
    auto self = static_cast<MmedPipelineManager *>(user_data);

    switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_EOS:
        if (self->m_photo_capturing) {
            g_print("Photo branch EOS received on bus\n");
            self->m_photo_capturing = false;
            self->detachPhotoBranch();
        }
        break;
    case GST_MESSAGE_ERROR: {
        GError *err;
        gchar *dbg;
        gst_message_parse_error(msg, &err, &dbg);
        g_printerr("GStreamer error: %s\n", err->message);
        g_error_free(err);
        g_free(dbg);
        break;
    }
    default:
        break;
    }

    return G_SOURCE_CONTINUE;
}

// ----------------------------
// pad probe 回调，抓第一帧
// ----------------------------
static GstPadProbeReturn onPhotoBuffer(GstPad *pad, GstPadProbeInfo *info,
                                       gpointer user_data) {
    auto self = static_cast<MmedPipelineManager *>(user_data);

    if (!(info->type & GST_PAD_PROBE_TYPE_BUFFER))
        return GST_PAD_PROBE_OK;

    g_print("Photo frame captured\n");

    // 发 EOS 到 photo branch
    // gst_pad_send_event(pad, gst_event_new_eos());

    // 移除 probe
    gst_pad_remove_probe(pad, self->m_photo_probe_id);
    self->m_photo_probe_id = 0;

    g_main_context_invoke(self->m_context,
                          [](gpointer data) -> gboolean {
                              auto self =
                                  static_cast<MmedPipelineManager *>(data);
                              self->detachPhotoBranch();
                              self->m_photo_capturing = false; // ⭐⭐⭐ 关键
                              return G_SOURCE_REMOVE;
                          },
                          self);

    return GST_PAD_PROBE_OK;
}

// 构造析构
MmedPipelineManager::MmedPipelineManager() {
    m_pic_path = "/data/DCIM/";
    m_shm = new IpcSharedMemory("/VideoFrameMemory", 1024 * 600 * 2, true);
}
MmedPipelineManager::~MmedPipelineManager() {
    stop();
    delete m_shm;
}

// ----------------------------
// 拍照接口
// ----------------------------
void MmedPipelineManager::takePhoto() {
    g_main_context_invoke(m_context,
                          [](gpointer data) -> gboolean {
                              auto self =
                                  static_cast<MmedPipelineManager *>(data);
                              self->attachPhotoBranch(self->m_pic_path);
                              return G_SOURCE_REMOVE;
                          },
                          this);
}

void MmedPipelineManager::startRecord() {
    g_main_context_invoke(
        m_context,
        [](gpointer data) -> gboolean {
            auto self = static_cast<MmedPipelineManager *>(data);
            if (!self->m_video_recording) {
                self->m_video_recording = true;
                self->attachVideoBranch(); // 这里不带参数，内部生成动态文件名
            }
            return G_SOURCE_REMOVE;
        },
        this);
}

// ----------------------------
// stopRecord
// ----------------------------
void MmedPipelineManager::stopRecord() {
    g_main_context_invoke(m_context,
                          [](gpointer data) -> gboolean {
                              auto self =
                                  static_cast<MmedPipelineManager *>(data);
                              if (!self->m_video_recording)
                                  return G_SOURCE_REMOVE;

                              self->m_video_recording = false;
                              self->detachVideoBranch(); // ⭐ 直接 detach
                              return G_SOURCE_REMOVE;
                          },
                          this);
}

// appsink 回调
static GstFlowReturn on_new_sample(GstAppSink *sink, gpointer user_data) {
    // std::cout << "onnewsample" << std::endl;
    // user_data: 你的共享内存对象
    auto *shm = static_cast<IpcSharedMemory *>(user_data);

    // 1. 拉一帧
    GstSample *sample = gst_app_sink_pull_sample(sink);
    if (!sample)
        return GST_FLOW_ERROR;

    // 2. 取 buffer
    GstBuffer *buffer = gst_sample_get_buffer(sample);
    GstMapInfo map;

    if (gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        // map.data  → 像素指针
        // map.size  → 字节数

        std::cout << "map.size = " << map.size << std::endl;
        shm->write(map.data, map.size);

        gst_buffer_unmap(buffer, &map);
    }

    gst_sample_unref(sample);
    return GST_FLOW_OK;
}

// 启动/停止
void MmedPipelineManager::start() {
    m_thread = std::thread(&MmedPipelineManager::gstThreadFunc, this);
}

void MmedPipelineManager::stop() {
    if (m_context) {
        g_main_context_invoke(m_context,
                              [](gpointer data) -> gboolean {
                                  auto self =
                                      static_cast<MmedPipelineManager *>(data);
                                  self->destroyPipeline();
                                  if (self->m_loop)
                                      g_main_loop_quit(self->m_loop);
                                  return G_SOURCE_REMOVE;
                              },
                              this);
    }
    if (m_thread.joinable())
        m_thread.join();
}

// pipeline 线程
void MmedPipelineManager::gstThreadFunc() {
    gst_init(nullptr, nullptr);

    m_context = g_main_context_new();
    g_main_context_push_thread_default(m_context);

    m_loop = g_main_loop_new(m_context, FALSE);

    createPipeline();
    attachPreviewBranch(); // 默认预览

    g_main_loop_run(m_loop);

    // cleanup
    destroyPipeline();
    g_main_loop_unref(m_loop);
    m_loop = nullptr;
    g_main_context_pop_thread_default(m_context);
    g_main_context_unref(m_context);
    m_context = nullptr;
}

// 创建主 pipeline
void MmedPipelineManager::createPipeline() {
    m_pipeline = gst_pipeline_new("mmed-pipeline");

    // 元素
    m_src = gst_element_factory_make("imxv4l2src", "src");
    m_caps = gst_element_factory_make("capsfilter", "caps");
    m_tee = gst_element_factory_make("tee", "tee");
    m_convert = gst_element_factory_make("videoconvert",
                                         "convert"); // 新增 videoconvert

    if (!m_pipeline || !m_src || !m_caps || !m_tee || !m_convert) {
        g_printerr("Failed to create pipeline elements\n");
        return;
    }

    // 设置属性
    g_object_set(m_src, "device", "/dev/video1", "do-timestamp", TRUE, NULL);

    GstCaps *caps = gst_caps_new_simple(
        "video/x-raw", "format", G_TYPE_STRING, "RGB16", // 改成 YUYV/YUY2
        "width", G_TYPE_INT, 1024, "height", G_TYPE_INT, 600, "framerate",
        GST_TYPE_FRACTION, 30, 1, // 改成 15fps
        NULL);

    g_object_set(m_caps, "caps", caps, NULL);
    gst_caps_unref(caps);

    // 添加到 pipeline
    gst_bin_add_many(GST_BIN(m_pipeline), m_src, m_caps, m_convert, m_tee,
                     NULL);

    // 链接元素
    if (!gst_element_link_many(m_src, m_caps, m_convert, m_tee, NULL)) {
        g_printerr("Failed to link main pipeline\n");
        return;
    }

    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(m_pipeline));
    gst_bus_add_watch(bus, on_bus_message, this);
    gst_object_unref(bus);

    // 启动 pipeline
    gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
}

// Preview branch
void MmedPipelineManager::attachPreviewBranch() {
    if (!m_pipeline || m_tee_preview_pad)
        return;

    // 创建元素
    m_q_preview = gst_element_factory_make("queue", "q_preview");
    m_crop_preview = gst_element_factory_make("videocrop", "crop_preview");
    m_videobox_preview =
        gst_element_factory_make("videobox", "videobox_preview");
    m_convert_preview =
        gst_element_factory_make("videoconvert", "convert_preview");
    m_appsink = gst_element_factory_make("appsink", "appsink");

    // m_caps_preview = gst_element_factory_make("capsfilter", "caps_preview");

    // GstCaps *caps = gst_caps_new_simple("video/x-raw", "format",
    // G_TYPE_STRING,
    //                                     "RGB16", "width", G_TYPE_INT, 1024,
    //                                     "height", G_TYPE_INT, 600, NULL);

    // g_object_set(m_caps_preview, "caps", caps, NULL);
    // gst_caps_unref(caps);

    if (!m_q_preview || !m_crop_preview || !m_videobox_preview ||
        !m_convert_preview || !m_appsink) {
        g_printerr("Failed to create preview elements\n");
        return;
    }

    // // 启动必须为 0
    // g_object_set(m_crop_preview, "left", 0, "right", 0, NULL);

    // g_object_set(m_videobox_preview, "left", 0, "right", 0, NULL);

    // 添加到 pipeline
    gst_bin_add_many(GST_BIN(m_pipeline), m_q_preview, m_convert_preview,
                     m_appsink, NULL);

    // 链接顺序：queue -> crop -> videobox -> convert -> appsink
    if (!gst_element_link_many(m_q_preview, m_convert_preview, m_appsink,
                               NULL)) {
        g_printerr("Failed to link preview branch\n");
        return;
    }

    // 获取 tee request pad
    m_tee_preview_pad = gst_element_get_request_pad(m_tee, "src_%u");
    GstPad *sink_pad = gst_element_get_static_pad(m_q_preview, "sink");
    if (gst_pad_link(m_tee_preview_pad, sink_pad) != GST_PAD_LINK_OK)
        g_printerr("Failed to link tee to preview queue\n");
    gst_object_unref(sink_pad);

    // 设置 queue 属性（防止阻塞）
    g_object_set(m_q_preview, "leaky", 2, "max-size-buffers", 1, NULL);

    // appsink 属性
    g_object_set(m_appsink, "emit-signals", TRUE, "sync", FALSE, "max-buffers",
                 1, "drop", TRUE, NULL);

    // appsink 回调
    GstAppSinkCallbacks cb = {};
    cb.new_sample = on_new_sample;
    gst_app_sink_set_callbacks(GST_APP_SINK(m_appsink), &cb, m_shm, nullptr);

    // 同步状态
    gst_element_sync_state_with_parent(m_q_preview);
    // gst_element_sync_state_with_parent(m_crop_preview);
    // gst_element_sync_state_with_parent(m_videobox_preview);
    gst_element_sync_state_with_parent(m_convert_preview);
    // gst_element_sync_state_with_parent(m_caps_preview);
    gst_element_sync_state_with_parent(m_appsink);

    g_print("Preview branch attached\n");
}
// ----------------------------
// attach photo branch
// ----------------------------
void MmedPipelineManager::attachPhotoBranch(const std::string &path) {
    if (!m_pipeline || m_photo_capturing)
        return;

    time_t tm_t = time(NULL);

    struct tm *local_time;
    local_time = localtime(&tm_t);
    m_photo_capturing = true;
    std::string pic_path = makePhotoPath(m_pic_path);
    printf("======= pic_path = %s\n", pic_path);

    // 创建元素
    m_q_photo = gst_element_factory_make("queue", "q_photo");
    m_convert_photo = gst_element_factory_make("videoconvert", "convert_photo");
    m_jpegenc = gst_element_factory_make("jpegenc", "jpegenc");
    m_photo_sink = gst_element_factory_make("filesink", "photo_sink");

    g_object_set(m_photo_sink, "location", pic_path.c_str(), "sync", FALSE,
                 NULL);

    // 加入 pipeline
    gst_bin_add_many(GST_BIN(m_pipeline), m_q_photo, m_convert_photo, m_jpegenc,
                     m_photo_sink, NULL);
    if (!gst_element_link_many(m_q_photo, m_convert_photo, m_jpegenc,
                               m_photo_sink, NULL)) {
        g_printerr("Failed to link photo branch\n");
        return;
    }

    // tee → photo queue
    m_tee_photo_pad = gst_element_get_request_pad(m_tee, "src_%u");
    GstPad *sink_pad = gst_element_get_static_pad(m_q_photo, "sink");
    if (gst_pad_link(m_tee_photo_pad, sink_pad) != GST_PAD_LINK_OK) {
        g_printerr("Failed to link tee to photo branch\n");
        gst_object_unref(sink_pad);
        return;
    }
    gst_object_unref(sink_pad);

    // 设置 photo branch 为 PLAYING
    gst_element_sync_state_with_parent(m_q_photo);
    gst_element_sync_state_with_parent(m_convert_photo);
    gst_element_sync_state_with_parent(m_jpegenc);
    gst_element_sync_state_with_parent(m_photo_sink);

    // ⭐ 抓第一帧
    m_photo_probe_pad = gst_element_get_static_pad(m_q_photo, "src");
    m_photo_probe_id =
        gst_pad_add_probe(m_photo_probe_pad, GST_PAD_PROBE_TYPE_BUFFER,
                          onPhotoBuffer, this, nullptr);

    g_print("Photo branch attached, waiting first frame...\n");
}

// -------------------------
// detach photo branch
// -------------------------
void MmedPipelineManager::detachPhotoBranch() {
    if (!m_tee_photo_pad)
        return;

    g_print("Detaching photo branch\n");

    // ① 先停掉 photo branch（最关键）
    gst_element_set_state(m_photo_sink, GST_STATE_NULL);
    gst_element_set_state(m_jpegenc, GST_STATE_NULL);
    gst_element_set_state(m_convert_photo, GST_STATE_NULL);
    gst_element_set_state(m_q_photo, GST_STATE_NULL);

    // （1.8 上不需要 get_state 等待，直接同步即可）

    // ② unlink tee -> queue
    GstPad *sink_pad = gst_element_get_static_pad(m_q_photo, "sink");
    gst_pad_unlink(m_tee_photo_pad, sink_pad);
    gst_object_unref(sink_pad);

    // ③ release tee request pad
    gst_element_release_request_pad(m_tee, m_tee_photo_pad);
    gst_object_unref(m_tee_photo_pad);
    m_tee_photo_pad = nullptr;

    // ④ 从 pipeline 中移除
    gst_bin_remove_many(GST_BIN(m_pipeline), m_q_photo, m_convert_photo,
                        m_jpegenc, m_photo_sink, NULL);

    // ⑤ 清空指针（防止二次 detach）
    m_q_photo = nullptr;
    m_convert_photo = nullptr;
    m_jpegenc = nullptr;
    m_photo_sink = nullptr;

    // ⑥ probe pad unref（你之前正确拿了）
    if (m_photo_probe_pad) {
        gst_object_unref(m_photo_probe_pad);
        m_photo_probe_pad = nullptr;
    }

    g_print("Photo branch detached cleanly\n");
}

// 视频分支
// ----------------------------
// attachVideoBranch
// ----------------------------
void MmedPipelineManager::attachVideoBranch() {
    if (!m_pipeline || m_tee_video_pad)
        return;

    std::string video_file =
        "/data/DCIM/video" + std::to_string(m_video_index++) + ".avi";

    // 1. 创建元素
    m_q_video = gst_element_factory_make("queue", "q_video");
    m_convert_video = gst_element_factory_make("videoconvert", "convert_video");
    m_encoder = gst_element_factory_make("jpegenc", "jpegenc_video");
    m_muxer = gst_element_factory_make("avimux", "avi_muxer");
    m_video_sink = gst_element_factory_make("filesink", "video_sink");

    if (!m_q_video || !m_convert_video || !m_encoder || !m_muxer ||
        !m_video_sink) {
        g_printerr("Failed to create MJPEG video elements\n");
        return;
    }

    g_object_set(m_video_sink, "location", video_file.c_str(), "sync", FALSE,
                 NULL);

    // 2. 加入 pipeline
    gst_bin_add_many(GST_BIN(m_pipeline), m_q_video, m_convert_video, m_encoder,
                     m_muxer, m_video_sink, NULL);

    // 3. 链接 branch 内部
    if (!gst_element_link_many(m_q_video, m_convert_video, m_encoder, m_muxer,
                               m_video_sink, NULL)) {
        g_printerr("Failed to link MJPEG video branch\n");
        return;
    }

    // 4. tee → queue
    m_tee_video_pad = gst_element_get_request_pad(m_tee, "src_%u");
    GstPad *sink_pad = gst_element_get_static_pad(m_q_video, "sink");
    if (gst_pad_link(m_tee_video_pad, sink_pad) != GST_PAD_LINK_OK) {
        g_printerr("Failed to link tee to video queue\n");
        gst_object_unref(sink_pad);
        return;
    }
    gst_object_unref(sink_pad);

    // 5. 同步状态
    gst_element_sync_state_with_parent(m_q_video);
    gst_element_sync_state_with_parent(m_convert_video);
    gst_element_sync_state_with_parent(m_encoder);
    gst_element_sync_state_with_parent(m_muxer);
    gst_element_sync_state_with_parent(m_video_sink);

    g_print("MJPEG video recording started: %s\n", video_file.c_str());
}

// ----------------------------
// detachVideoBranch
// ----------------------------
void MmedPipelineManager::detachVideoBranch() {
    if (!m_tee_video_pad)
        return;

    g_print("Requesting video branch block...\n");

    // 添加阻塞 Probe，当数据流暂停时，会触发 video_block_probe_callback
    // 使用 GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM 比 BLOCK 更适合停止下游的情况
    m_video_block_probe_id =
        gst_pad_add_probe(m_tee_video_pad, GST_PAD_PROBE_TYPE_BLOCK_DOWNSTREAM,
                          video_block_probe_callback, this, NULL);
}

// 销毁 pipeline
void MmedPipelineManager::destroyPipeline() {
    if (!m_pipeline)
        return;

    gst_element_set_state(m_pipeline, GST_STATE_NULL);

    // 释放 preview pad
    if (m_tee_preview_pad) {
        gst_element_release_request_pad(m_tee, m_tee_preview_pad);
        gst_object_unref(m_tee_preview_pad);
        m_tee_preview_pad = nullptr;
    }

    if (m_q_preview) {
        gst_bin_remove(GST_BIN(m_pipeline), m_q_preview);
        m_q_preview = nullptr;
    }
    if (m_convert_preview) {
        gst_bin_remove(GST_BIN(m_pipeline), m_convert_preview);
        m_convert_preview = nullptr;
    }
    if (m_appsink) {
        gst_bin_remove(GST_BIN(m_pipeline), m_appsink);
        m_appsink = nullptr;
    }

    // photo branch
    if (m_tee_photo_pad) {
        gst_element_release_request_pad(m_tee, m_tee_photo_pad);
        gst_object_unref(m_tee_photo_pad);
        m_tee_photo_pad = nullptr;
    }
    if (m_q_photo) {
        gst_bin_remove(GST_BIN(m_pipeline), m_q_photo);
        m_q_photo = nullptr;
    }
    if (m_jpegenc) {
        gst_bin_remove(GST_BIN(m_pipeline), m_jpegenc);
        m_jpegenc = nullptr;
    }
    if (m_photo_sink) {
        gst_bin_remove(GST_BIN(m_pipeline), m_photo_sink);
        m_photo_sink = nullptr;
    }

    // video branch
    if (m_tee_video_pad) {
        gst_element_release_request_pad(m_tee, m_tee_video_pad);
        gst_object_unref(m_tee_video_pad);
        m_tee_video_pad = nullptr;
    }
    if (m_q_video) {
        gst_bin_remove(GST_BIN(m_pipeline), m_q_video);
        m_q_video = nullptr;
    }
    if (m_convert_video) {
        gst_bin_remove(GST_BIN(m_pipeline), m_convert_video);
        m_convert_video = nullptr;
    }
    if (m_encoder) {
        gst_bin_remove(GST_BIN(m_pipeline), m_encoder);
        m_encoder = nullptr;
    }
    if (m_muxer) {
        gst_bin_remove(GST_BIN(m_pipeline), m_muxer);
        m_muxer = nullptr;
    }
    if (m_video_sink) {
        gst_bin_remove(GST_BIN(m_pipeline), m_video_sink);
        m_video_sink = nullptr;
    }

    if (m_src) {
        gst_bin_remove(GST_BIN(m_pipeline), m_src);
        m_src = nullptr;
    }
    if (m_caps) {
        gst_bin_remove(GST_BIN(m_pipeline), m_caps);
        m_caps = nullptr;
    }
    if (m_tee) {
        gst_bin_remove(GST_BIN(m_pipeline), m_tee);
        m_tee = nullptr;
    }

    gst_object_unref(m_pipeline);
    m_pipeline = nullptr;
}
void MmedPipelineManager::setPreviewOffsetX(uint32_t offset_x) {
    if (!m_crop_preview || !m_videobox_preview)
        return;

    // 对齐到 4 的倍数
    int offset = offset_x & ~3;

    // videocrop 裁掉左侧 offset 像素
    g_object_set(m_crop_preview, "left", offset, NULL);

    // videobox 在右侧补回相同的像素，实现平移
    g_object_set(m_videobox_preview, "right", -offset, NULL);

    // 可选：打印调试信息
    g_print("Preview offset set: left crop=%d, right videobox=%d\n", offset,
            -offset);
}