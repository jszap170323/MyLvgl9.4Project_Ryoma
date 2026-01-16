#ifndef _LIVEVIEW_CONTROLLER_H
#define _LIVEVIEW_CONTROLLER_H
#include "gui_base_controller.h"
#include "gui_liveview.h"

class GUI_LiveviewController : public GUI_BaseController {

  public:
    GUI_LiveviewController(GuiMessageCtrl *msg_ctrl);
    ~GUI_LiveviewController();

    void handleEvent(const GUI_Event &e);

  private:
    std::unique_ptr<GUI_Liveview> m_lv_view;
};

#endif
