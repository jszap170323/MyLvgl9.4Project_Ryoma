#include "gui_datamng.h"

GUI_DataMng::GUI_DataMng() {}
GUI_DataMng::~GUI_DataMng() {}

bool GUI_DataMng::sendMsgToMmed(uint32_t cmd_type, uint32_t cmd,
                                const void *payload, uint32_t payload_len) {

    m_msg_ctrl->sendMsgToMmed(1, // mtype
                              cmd_type,
                              cmd, // cmd
                              payload, payload_len);

    return true;
}

bool GUI_DataMng::setFrameMovePixes(uint32_t offset_x) {
    printf("GUI_DataMng::setFrameMovePixes\n");
    sendMsgToMmed(static_cast<uint32_t>(CmdType::SetParamType),
                  static_cast<uint32_t>(SetParamCmd::SetPreviewPixesMove),
                  &offset_x, sizeof(uint32_t));
    return true;
}