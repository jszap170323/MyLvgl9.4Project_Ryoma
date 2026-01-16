#ifndef _MMED_CONTROLLER_H
#define _MMED_CONTROLLER_H

#include <thread>

#include "block_queue.h"
#include "ipc_message_queue.h"
#include "mmed_pipeline_manager.h"

class MmedCtroller {

  public:
    MmedCtroller(MmedPipelineManager *pipe_manager);
    ~MmedCtroller();

    void start();

    void postCommand(IpcMsgBuffer &msg);
    void receiveMessage();

    bool handleCommand(uint32_t cmd, void *args);

  private:
    MmedPipelineManager *m_pipe_manager;
    BlockingQueue<IpcMsgBuffer> m_block_queue;
    std::thread m_receive_thread_t;
};

#endif /*  _MMED_CONTROLLER_H*/
