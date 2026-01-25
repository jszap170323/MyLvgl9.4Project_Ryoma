#include "liveview_controller.h"

#include "gui_navibar_controller.h"
#include <sys/reboot.h>

GUI_LiveviewController::GUI_LiveviewController() {
    m_lv_view = std::make_unique<GUI_Liveview>();
    m_lv_view->setEventCallback(
        [this](const GUI_LiveviewEvent &e) { handleEvent(e); });

    m_navibar_controller =
        std::make_unique<GUI_NavibarController>(m_lv_view->getRoot());

    m_navibar_controller->setLiveviewMoveFramePixesCallback(
        [this](int32_t offset) { this->updateFrameOffset(offset); });
}

GUI_LiveviewController::~GUI_LiveviewController() {}

void GUI_LiveviewController::updateFrameOffset(int32_t offset_pixels) {
    if (m_lv_view) {
        m_lv_view->updateOffset(offset_pixels);
    }
}

void GUI_LiveviewController::handleEvent(const GUI_LiveviewEvent &e) {
    switch (e.type) {
    case GUI_LiveviewEventType::ScreentClick: {
        printf("ScreentClick===================\n");
        if (m_navibar_controller) {
            if (m_navibar_controller->isNavibarfolded() == false) {
                m_navibar_controller->foldNavibar();
            }
        }

        break;
    }

    case GUI_LiveviewEventType::PhotoClick: {
        printf("photo===================");
        // GUI_DataMng::getInstance()->sendMsgToMmed(
        //     static_cast<uint32_t>(CmdType::CtrlType),
        //     static_cast<uint32_t>(CtrlCmd::StartTakePhoto), 0, 0);

        static int32_t value = 0;
        value = (value % 8);
        GUI_DataMng::getInstance()->sendMsgToMmed(
            static_cast<uint32_t>(CmdType::SetParamType),
            static_cast<uint32_t>(SetParamCmd::SetBackLightLevel), &value, 4);
        value++;

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
