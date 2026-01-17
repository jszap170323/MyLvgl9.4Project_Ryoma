#ifndef _GUI_LIVEVIEW_H
#define _GUI_LIVEVIEW_H
#include "gui_base_view.h"
#include "gui_event_liveview.h"
#include "gui_video_frame_canvas.h"

#include "lvgl.h"
class GUI_Liveview : public GUI_BaseView<GUI_LiveviewEvent> {

  public:
    GUI_Liveview();
    ~GUI_Liveview();

    // virtual void setEventCallback(EventCallback cb);

  private:
    void handleUIEvent(lv_event_t *event);

    // lv_obj_t *m_root;
    lv_obj_t *m_photo;
    // EventCallback m_ctrl_callback;

    std::unique_ptr<GUI_VideoFrameCanvas> m_video_frame_canvas;
};

#endif
