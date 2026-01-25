#include "gui_navibar_controller.h"

GUI_NavibarController::GUI_NavibarController(lv_obj_t *root) {
    m_navibar_view = std::make_unique<GUI_NaviBarView>(root);

    m_navibar_view->setEventCallback(
        [this](const GUI_NavibarEvent &e) { handleEvent(e); });
}
GUI_NavibarController::~GUI_NavibarController() {}

bool GUI_NavibarController::isNavibarfolded() {
    return m_navibar_view->isNavibarfolded();
}

void GUI_NavibarController::foldNavibar() { m_navibar_view->foldNavibar(); }

void GUI_NavibarController::handleEvent(const GUI_NavibarEvent &e) {
    switch (e.type) {
    case GUI_NavibarEventType::PixesMove: {

        // GUI_DataMng::getInstance()->setFrameMovePixes(e.value);
        if (m_frme_move_callback) {
            printf("PixesMove=================== offsetx = %d\n", e.value);
            m_frme_move_callback(e.value);
        }

        break;
    }

    default:
        break;
    }
}
