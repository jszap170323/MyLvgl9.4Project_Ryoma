#include "gui_video_frame_canvas.h"

GUI_VideoFrameCanvas::GUI_VideoFrameCanvas(lv_obj_t *root) : m_root(root) {

    video_canvas_init();
    m_video_frame_thread =
        std::thread(&GUI_VideoFrameCanvas::video_thread_func, this);

    // ============ 创建刷新定时器 ============
    lv_timer_t *refresh_timer = lv_timer_create(
        [](lv_timer_t *t) {
            GUI_VideoFrameCanvas *lv =
                static_cast<GUI_VideoFrameCanvas *>(lv_timer_get_user_data(t));
            lv->refresh_timer_cb(t);
        },
        16, this); // ~60Hz
}
GUI_VideoFrameCanvas::~GUI_VideoFrameCanvas() {}

void GUI_VideoFrameCanvas::video_canvas_init(void) {
    size_t buf_size = LV_CANVAS_BUF_SIZE(root_width, root_height, 16,
                                         LV_DRAW_BUF_STRIDE_ALIGN);
    printf("Allocating canvas buffer: %zu bytes (%.2f MB)\n", buf_size,
           buf_size / (1024.0 * 1024.0));

    // 分配视频缓冲区
    video_buf = new uint8_t[buf_size];
    memset(video_buf, 0, buf_size);

    // 分配显示缓冲区（双缓冲）
    display_buf = new uint8_t[buf_size];
    memset(display_buf, 0, buf_size);

    // 创建 Canvas
    canvas = lv_canvas_create(m_root);

    // 使用显示缓冲区
    lv_canvas_set_buffer(canvas, display_buf, root_width, root_height,
                         LV_COLOR_FORMAT_RGB565);

    lv_obj_set_size(canvas, root_width, root_height);
    lv_obj_center(canvas);

    // 设置 Canvas 背景透明
    lv_obj_set_style_bg_opa(canvas, LV_OPA_TRANSP, 0);

    printf("Canvas initialized successfully\n");
}

// ============ 视频线程函数 ============
void GUI_VideoFrameCanvas::video_thread_func() {
    printf("Video thread started\n");

    IpcSharedMemory shm("/VideoFrameMemory", root_width * root_height * 2,
                        false);

    struct timespec last_time;
    clock_gettime(CLOCK_MONOTONIC, &last_time);

    while (running) {

        // 从共享内存读取视频帧
        shm.read(video_buf, root_width * root_height * 2);

        pushFrame(video_buf, root_width * root_height * 2);

        // 控制帧率（~60 FPS）
        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);

        long elapsed_ns =
            (current_time.tv_sec - last_time.tv_sec) * 1000000000L +
            (current_time.tv_nsec - last_time.tv_nsec);

        long target_ns = 16666666L; // 60 FPS
        if (elapsed_ns < target_ns) {
            long sleep_ns = target_ns - elapsed_ns;
            struct timespec sleep_time = {.tv_sec = sleep_ns / 1000000000L,
                                          .tv_nsec = sleep_ns % 1000000000L};
            nanosleep(&sleep_time, NULL);
        }

        clock_gettime(CLOCK_MONOTONIC, &last_time);
    }

    printf("Video thread finished\n");
}

// ============ LVGL 定时器回调（用于刷新 Canvas） ============
void GUI_VideoFrameCanvas::refresh_timer_cb(lv_timer_t *timer) {

    std::vector<uint8_t> frame;

    {
        std::lock_guard<std::mutex> lk(frame_mtx);
        if (!frame_queue.empty()) {
            frame = std::move(frame_queue.front());
            frame_queue.pop_front();

            memcpy(display_buf, frame.data(), frame.size());
            lv_obj_invalidate(canvas);
        }
    }
}

void GUI_VideoFrameCanvas::pushFrame(uint8_t *buffer, uint32_t buf_size) {
    std::lock_guard<std::mutex> lk(frame_mtx);
    std::vector<uint8_t> frame(buf_size);
    memcpy(frame.data(), buffer, buf_size);

    frame_queue.push_front(std::move(frame));
}

void GUI_VideoFrameCanvas::updateOffset(int32_t offset_pixels) {
    printf("-------------------------------------aadaafa");
    // 更新Canvas位置（向左移动）
    lv_obj_set_pos(canvas, -offset_pixels, 0);

    printf("Set offset to %d pixels\n", offset_pixels);
}