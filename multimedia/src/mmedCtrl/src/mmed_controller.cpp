#include "mmed_controller.h"

MmedCtroller::MmedCtroller(MmedPipelineManager *pipe_manager)
    : m_pipe_manager(pipe_manager) {}

MmedCtroller::~MmedCtroller() {}

void MmedCtroller::start() {
    m_receive_thread_t = std::thread(&MmedCtroller::receiveMessage, this);
}
void MmedCtroller::postCommand(IpcMsgBuffer &msg) {
    printf("medCtroller::postCommand\n");
    m_block_queue.putTask(msg);
}

void MmedCtroller::receiveMessage() {
    while (1) {
        IpcMsgBuffer msg = m_block_queue.takeTask();
        printf("MmedCtroller::receiveMessage\n");
        CmdType type = static_cast<CmdType>(msg.cmd_type);
        printf("type = %d,msg.cmd = %d, msg.data_size = %d\n", type, msg.cmd,
               msg.data_size);

        switch (type) {
        case CmdType::CtrlType: {
            handleCtrlCommand(msg.cmd, msg.data);
            break;
        }
        case CmdType::SetParamType: {
            handleSetParamsCommand(msg.cmd, &msg);
            break;
        }

        default:
            break;
        }
    }
}

bool MmedCtroller::handleCtrlCommand(uint32_t cmd, void *args) {
    printf("MmedCtroller::handleCtrlCommand cmd = %d\n", cmd);

    CtrlCmd command = static_cast<CtrlCmd>(cmd);
    switch (command) {
    case CtrlCmd::StartTakePhoto: {
        m_pipe_manager->takePhoto();
        break;
    }
    case CtrlCmd::StartRecVideo: {
        m_pipe_manager->startRecord();
        break;
    }

    case CtrlCmd::StopRecVideo: {
        m_pipe_manager->stopRecord();
        break;
    }

    default: { break; }
    }

    return true;
}

// bool MmedCtroller::handleSetParamsCommand(uint32_t cmd, void *args) {

//     printf("MmedCtroller::handleSetParamsCommand cmd = %d\n", cmd);

//     SetParamCmd command = static_cast<SetParamCmd>(cmd);
//     switch (command) {
//     case SetParamCmd::SetPreviewPixesMove: {
//         // m_pipe_manager->stopRecord();
//         uint32_t *param = reinterpret_cast<uint32_t *>(args);
//         uint32_t offset_x = 0;
//         memcpy(&offset_x, param, sizeof(uint32_t));
//         printf("mmed SetPreviewPixesMove offset_x = %d\n", offset_x);
//         break;
//     }

//     default: { break; }
//     }
//     return true;
// }

// MmedCtroller::handleSetParamsCommand
bool MmedCtroller::handleSetParamsCommand(uint32_t cmd, void *args) {
    // args 指向 IpcMsgBuffer (或者偏移后的位置，取决于你的实现)
    // 假设你之前的调用是 handleSetParamsCommand(msg.cmd, msg.data);
    // 现在修改后的 readMsg 会填充整个结构体。

    // 如果 receiveMessage 传的是 &msg (IpcMsgBuffer 对象)
    IpcMsgBuffer *pMsg = static_cast<IpcMsgBuffer *>(args);

    printf("cmd=%d, data_size=%d\n", pMsg->cmd, pMsg->data_size);

    SetParamCmd command = static_cast<SetParamCmd>(cmd);
    switch (command) {
    case SetParamCmd::SetPreviewPixesMove: {
        if (pMsg->data_size >= sizeof(uint32_t)) {
            uint32_t offset_x = 0;
            // ⭐ 直接从 data[0] 读取，不需要 +16 偏移了！
            memcpy(&offset_x, pMsg->data, sizeof(uint32_t));
            printf("mmed SetPreviewPixesMove offset_x = %d\n", offset_x);
        }
        break;
    }
        // ...
    }
    return true;
}
