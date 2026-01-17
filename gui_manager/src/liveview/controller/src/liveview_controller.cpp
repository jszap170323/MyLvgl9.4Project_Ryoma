#include "liveview_controller.h"

#include "gui_navibar_controller.h"
#include <sys/reboot.h>

GUI_LiveviewController::GUI_LiveviewController() {
    m_lv_view = std::make_unique<GUI_Liveview>();
    m_lv_view->setEventCallback(
        [this](const GUI_LiveviewEvent &e) { handleEvent(e); });

    m_navibar_controller =
        std::make_unique<GUI_NavibarController>(m_lv_view->getRoot());
}

GUI_LiveviewController::~GUI_LiveviewController() {}

void GUI_LiveviewController::handleEvent(const GUI_LiveviewEvent &e) {
    switch (e.type) {
    case GUI_LiveviewEventType::ScreentClick: {
        printf("ScreentClick===================");
        if (m_navibar_controller) {
            if (m_navibar_controller->isNavibarfolded() == false) {
                m_navibar_controller->foldNavibar();
            }
        }

        break;
    }

    case GUI_LiveviewEventType::PhotoClick: {
        printf("photo===================");
        GUI_DataMng::getInstance()->sendMsgToMmed(
            static_cast<uint32_t>(CmdType::CtrlType),
            static_cast<uint32_t>(CtrlCmd::StartTakePhoto), 0, 0);
        break;
    }
    case GUI_LiveviewEventType::StartRecord: {
        printf("StartRecord===================");
        GUI_DataMng::getInstance()->sendMsgToMmed(
            static_cast<uint32_t>(CmdType::CtrlType),
            static_cast<uint32_t>(CtrlCmd::StartRecVideo), 0, 0);
        break;
    }

    case GUI_LiveviewEventType::StopRecord: {
        printf("StopRecord===================");
        GUI_DataMng::getInstance()->sendMsgToMmed(
            static_cast<uint32_t>(CmdType::CtrlType),
            static_cast<uint32_t>(CtrlCmd::StopRecVideo), 0, 0);
        break;
    }

    default:
        break;
    }
}
