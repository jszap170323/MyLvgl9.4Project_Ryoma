#ifndef _MMED_CONTROLLER_H
#define _MMED_CONTROLLER_H

#include "block_queue.h"
#include "ipc_message_queue.h"
#include "mmed_pipeline_manager.h"
#include <atomic>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <thread>
#include <unistd.h>

class MmedCtroller {

  public:
    MmedCtroller(MmedPipelineManager *pipe_manager);
    ~MmedCtroller();

    void start();

    void postCommand(IpcMsgBuffer &msg);
    void receiveMessage();

    bool handleCtrlCommand(uint32_t cmd, void *args);
    bool handleSetParamsCommand(uint32_t cmd, void *args);

  private:
    MmedPipelineManager *m_pipe_manager;
    BlockingQueue<IpcMsgBuffer> m_block_queue;
    std::thread m_receive_thread_t;
};

#endif /*  _MMED_CONTROLLER_H*/
