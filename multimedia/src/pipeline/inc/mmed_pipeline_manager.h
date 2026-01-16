#ifndef _MMED_PIPELINE_MANAGER_H

#define _MMED_PIPELINE_MANAGER_H

// #include <gst/app/gstappsink.h>
// #include <gst/gst.h>
// #include <iostream>
// #include <string>
// #include <thread>

// #include "ipc_shared_memory.h"

// class MmedPipelineManager {

//   public:
//     MmedPipelineManager();
//     ~MmedPipelineManager();

//     bool preview();
//     void start();
//     void createPreviewPipeline();

//   private:
//     bool startGstThread();
//     IpcSharedMemory *m_shared_memory;
//     std::thread m_gstThread;
//     GMainLoop *m_mainLoop;
//     GMainContext *m_context;
//     GstElement *m_pipeline;
// };

#include <chrono>
#include <gst/app/gstappsink.h>
#include <gst/gst.h>
#include <iostream>
#include <string>
#include <thread>

class IpcSharedMemory;

class MmedPipelineManager {
  public:
    MmedPipelineManager();
    ~MmedPipelineManager();

    // 生命周期
    void start();
    void stop();

    // 模式控制（由 StateMachine 调用）
    void enablePreview();
    void takePhoto();
    void startRecord();
    void stopRecord();

    void detachPhotoBranch();
    void attachVideoBranch();
    void detachVideoBranch();

    GstPad *m_photo_probe_pad = nullptr;
    gulong m_photo_probe_id = 0;
    bool m_photo_capturing = false;

    GMainContext *m_context = nullptr;

    bool m_video_recording = false; // 视频录像标志
    int m_video_index = 0;          // 多次录像索引

  private:
    // === pipeline thread ===
    void gstThreadFunc();

    // === pipeline building ===
    void createPipeline();
    void destroyPipeline();

    // === branches ===
    void attachPreviewBranch();
    void detachPreviewBranch();

    void attachPhotoBranch(const std::string &path);

  private:
    // === GStreamer core ===
    GstElement *m_pipeline = nullptr;
    GstElement *m_src = nullptr;
    GstElement *m_caps = nullptr;
    GstElement *m_tee = nullptr;

    // preview
    GstElement *m_q_preview = nullptr;
    GstElement *m_appsink = nullptr;
    GstElement *m_convert_preview = nullptr;
    GstElement *m_convert = nullptr;

    GstPad *m_tee_preview_pad = nullptr;

    // photo
    GstElement *m_q_photo = nullptr;
    GstElement *m_jpegenc = nullptr;
    GstElement *m_photo_sink = nullptr;
    GstElement *m_convert_photo = nullptr;
    GstPad *m_tee_photo_pad = nullptr;
    std::string m_pic_path = "";
    u_int32_t m_photo_index = 0;

    // video
    GstElement *m_q_video = nullptr;
    GstElement *m_encoder = nullptr;
    GstElement *m_mux = nullptr;
    GstElement *m_video_sink = nullptr;
    GstPad *m_tee_video_pad = nullptr;
    GstElement *m_convert_video = nullptr;
    GstElement *m_muxer = nullptr;
    GstElement *m_caps_video = nullptr;

    // === GLib loop ===
    GMainLoop *m_loop = nullptr;
    // GMainContext *m_context = nullptr;
    std::thread m_thread;

    // shared memory
    IpcSharedMemory *m_shm = nullptr;
    gulong m_video_block_probe_id = 0;
    static GstPadProbeReturn video_block_probe_callback(GstPad *pad,
                                                        GstPadProbeInfo *info,
                                                        gpointer user_data);
};

#endif /*  _MMED_PIPELINE_MANAGER_H*/
