#ifndef _GUI_VIDEO_FRAME_CANVAS_H
#define _GUI_VIDEO_FRAME_CANVAS_H
#include "gui_base_view.h"
#include "gui_event_frame.h"
#include "ipc_shared_memory.h"
#include "lvgl.h"
#include <deque>
#include <vector>
class GUI_VideoFrameCanvas : public GUI_BaseView<GUI_FrameEvent> {

  public:
    GUI_VideoFrameCanvas(lv_obj_t *root);
    ~GUI_VideoFrameCanvas();

    void video_canvas_init(void);
    void video_thread_func();
    void refresh_timer_cb(lv_timer_t *timer);
    void pushFrame(uint8_t *buffer, uint32_t buf_size);
    void updateOffset(int32_t offset_pixels);

    uint8_t *video_buf = nullptr;   // 视频缓冲区
    uint8_t *display_buf = nullptr; // 显示缓冲区（双缓冲）
    lv_obj_t *canvas = nullptr;     // Canvas对象
    std::thread m_video_frame_thread;
    std::atomic<bool> running{true};
    // std::shared_mutex;
    std::mutex frame_mtx;
    std::deque<std::vector<uint8_t>> frame_queue;
    lv_obj_t *m_root = nullptr;
};

#endif
