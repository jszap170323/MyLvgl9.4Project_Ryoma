#include "gui_message_ctrl.h"

GuiMessageCtrl::GuiMessageCtrl() {
    // m_ui_to_mmed_msg_queue.createMessageQueue(ui_to_multimedia, 512, 1024);
    // m_mmed_to_ui_msg_queue.createMessageQueue(multimedia_to_ui, 512, 1024);
    m_ui_to_mmed_msg_queue.createMsgQueue(ui_to_multimedia_ipc_key);
    m_mmed_to_ui_msg_queue.createMsgQueue(multimedia_to_ui_ipc_key);
}

GuiMessageCtrl::~GuiMessageCtrl() {}

bool GuiMessageCtrl::start() {
    m_receive_thread_t = std::thread(&GuiMessageCtrl::receiveUiMessage, this);

    return true;
}

void GuiMessageCtrl::receiveUiMessage() {
    while (true) {
        uint8_t recv_buf[1024];
        ssize_t len =
            m_mmed_to_ui_msg_queue.readMsg(recv_buf, sizeof(recv_buf), 1);

        if (len > 0) {
            auto *msg = reinterpret_cast<IpcMsgBuffer *>(recv_buf);

            printf("GuiMessageCtrl::receiveUiMessage cmd=%u data_size=%u\n",
                   msg->cmd, msg->data_size);
        }
    }
}

bool GuiMessageCtrl::sendMsgToMmed(long mtype, uint32_t cmd_type, uint32_t cmd,
                                   const void *payload, uint32_t payload_len) {

    m_ui_to_mmed_msg_queue.sendCommand(mtype, // mtype
                                       cmd_type,
                                       cmd, // cmd
                                       payload, payload_len);

    return true;
}
