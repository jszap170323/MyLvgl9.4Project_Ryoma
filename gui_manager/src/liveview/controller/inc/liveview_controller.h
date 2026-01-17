#ifndef _LIVEVIEW_CONTROLLER_H
#define _LIVEVIEW_CONTROLLER_H
#include "gui_base_controller.h"
#include "gui_liveview.h"
#include <thread>

class GUI_NavibarController;

class GUI_LiveviewController : public GUI_BaseController {

  public:
    GUI_LiveviewController();
    ~GUI_LiveviewController();

    void handleEvent(const GUI_LiveviewEvent &e);

  private:
    std::unique_ptr<GUI_NavibarController> m_navibar_controller;

    std::unique_ptr<GUI_Liveview> m_lv_view;
};

#endif
