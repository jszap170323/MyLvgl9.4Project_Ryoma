#ifndef _MMED_MESSAGE_CTRL_H
#define _MMED_MESSAGE_CTRL_H

#include "ipc_message_queue.h"
#include "mmed_controller.h"
#include <iostream>
#include <thread>

const std::string ui_to_multimedia = "/ui_to_multimedia_msg_queue";
const std::string multimedia_to_ui = "/multimedia_to_ui_msg_queue";

class MmedMessageCtrl {

  public:
    MmedMessageCtrl(MmedCtroller *controller);
    ~MmedMessageCtrl();
    bool start();

    bool sendMsgToUI(long mtype, uint32_t cmd, const void *payload,
                     uint32_t payload_len);

  private:
    void receiveUiMessage();

    IpcMessageQueue m_ui_to_mmed_msg_queue;
    IpcMessageQueue m_mmed_to_ui_msg_queue;
    std::thread m_receive_thread_t;
    MmedCtroller *m_mmed_controller;
};

#endif /*  _MMED_MESSAGE_CTRL_H*/
