#ifndef _GUI_LIVEVIEW_H
#define _GUI_LIVEVIEW_H
#include "gui_base_view.h"
#include "lvgl.h"

class GUI_Liveview : public GUI_BaseView {

  public:
    GUI_Liveview();
    ~GUI_Liveview();

    virtual void setEventCallback(EventCallback cb);

  private:
    lv_obj_t *m_root;
    lv_obj_t *m_photo;
    EventCallback m_ctrl_callback;
};

#endif
