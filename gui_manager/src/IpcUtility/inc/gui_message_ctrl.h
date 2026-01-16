#ifndef _GUI_MESSAGE_CTRL_H
#define _GUI_MESSAGE_CTRL_H

#include "ipc_message_queue.h"
#include "ipc_ui_multimedia_command.h"
#include <iostream>
#include <thread>

const std::string ui_to_multimedia = "/ui_to_multimedia_msg_queue";
const std::string multimedia_to_ui = "/multimedia_to_ui_msg_queue";

class GuiMessageCtrl {

  public:
    GuiMessageCtrl();
    ~GuiMessageCtrl();
    bool start();

    bool sendMsgToMmed(long mtype, uint32_t cmd, const void *payload,
                       uint32_t payload_len);

  private:
    void receiveUiMessage();

    IpcMessageQueue m_ui_to_mmed_msg_queue;
    IpcMessageQueue m_mmed_to_ui_msg_queue;
    std::thread m_receive_thread_t;
};

#endif /*  _GUI_MESSAGE_CTRL_H*/
