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
        handleCommand(msg.cmd, msg.data);
    }
}

bool MmedCtroller::handleCommand(uint32_t cmd, void *args) {
    printf("MmedCtroller::handleCommand cmd = %d\n", cmd);

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