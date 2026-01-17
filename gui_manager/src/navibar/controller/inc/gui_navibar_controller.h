#ifndef _GUI_NAVIBAR_CONTROLLER_H
#define _GUI_NAVIBAR_CONTROLLER_H

#include "gui_base_controller.h"
#include "gui_navibar_view.h"

class GUI_NavibarController : public GUI_BaseController {

  public:
    GUI_NavibarController(lv_obj_t *root);
    ~GUI_NavibarController();

    bool isNavibarfolded();
    void foldNavibar();

  private:
    void handleEvent(const GUI_NavibarEvent &e);
    std::unique_ptr<GUI_NaviBarView> m_navibar_view;
};

#endif