#include "liveview_controller.h"
#include <sys/reboot.h>
GUI_LiveviewController::GUI_LiveviewController(GuiMessageCtrl *msg_ctrl)
    : GUI_BaseController(msg_ctrl) {
    m_lv_view = std::make_unique<GUI_Liveview>();
    m_lv_view->setEventCallback([this](const GUI_Event &e) { handleEvent(e); });
}

GUI_LiveviewController::~GUI_LiveviewController() {}

void GUI_LiveviewController::handleEvent(const GUI_Event &e) {
    switch (e.type) {
    case GUI_EventType::PHOTO_CLICK: {
        printf("photo===================");
        m_msg_ctrl->sendMsgToMmed(
            1, static_cast<uint32_t>(CtrlCmd::StartTakePhoto), 0, 0);
        break;
    }
    case GUI_EventType::StartRecord: {
        printf("StartRecord===================");
        m_msg_ctrl->sendMsgToMmed(
            1, static_cast<uint32_t>(CtrlCmd::StartRecVideo), 0, 0);
        break;
    }

    case GUI_EventType::StopRecord: {
        printf("StopRecord===================");
        m_msg_ctrl->sendMsgToMmed(
            1, static_cast<uint32_t>(CtrlCmd::StopRecVideo), 0, 0);
        break;
    }

    default:
        break;
    }
}
