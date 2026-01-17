#include "mmed_message_ctrl.h"

MmedMessageCtrl::MmedMessageCtrl(MmedCtroller *controller)
    : m_mmed_controller(controller) {
    // m_ui_to_mmed_msg_queue.createMessageQueue(ui_to_multimedia, 512, 1024);
    // m_mmed_to_ui_msg_queue.createMessageQueue(multimedia_to_ui, 512, 1024);
    m_ui_to_mmed_msg_queue.createMsgQueue(ui_to_multimedia_ipc_key);
    m_mmed_to_ui_msg_queue.createMsgQueue(multimedia_to_ui_ipc_key);
}

MmedMessageCtrl::~MmedMessageCtrl() {}

bool MmedMessageCtrl::start() {
    m_receive_thread_t = std::thread(&MmedMessageCtrl::receiveUiMessage, this);

    return true;
}

void MmedMessageCtrl::receiveUiMessage() {
    while (true) {
        // std::cout <<"readMsg" <<std::endl;
        // uint8_t recv_buf[1024];
        // ssize_t len =
        //     m_ui_to_mmed_msg_queue.readMsg(recv_buf, sizeof(recv_buf), 1);
        // if (len > 0) {
        //     IpcCommandData *recv_msg =
        //         reinterpret_cast<IpcCommandData *>(recv_buf);
        //     printf("cmd=%u, data_size=%u\n", recv_msg->cmd,
        //            recv_msg->data_size);
        //     uint32_t value;
        //     memcpy(&value, recv_msg->data, sizeof(value));
        //     printf("payload=%u\n", value);
        // }

        uint8_t recv_buf[1024];
        ssize_t len =
            m_ui_to_mmed_msg_queue.readMsg(recv_buf, sizeof(recv_buf), 1);

        printf("==111111 len = %d\n", len);
        auto *msg = reinterpret_cast<IpcMsgBuffer *>(recv_buf);

        m_mmed_controller->postCommand(*msg);
    }
}

bool MmedMessageCtrl::sendMsgToUI(long mtype, uint32_t cmd, const void *payload,
                                  uint32_t payload_len) {

    // m_mmed_to_ui_msg_queue.sendCommand(mtype, // mtype
    //                                    cmd,   // cmd
    //                                    payload, payload_len);

    return true;
}