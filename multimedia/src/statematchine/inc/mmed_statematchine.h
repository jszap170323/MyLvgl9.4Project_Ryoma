#ifndef _MMED_STATEMATCHINE_H

#define _MMED_STATEMATCHINE_H

#include "ipc_ui_multimedia_command.h"
#include "mmed_pipeline_manager.h"
#include "mmed_setting_manager.h"
#include "mmed_state_matchine_interface.h"

#include "block_queue.h"
#include "ipc_message_queue.h"
#include <atomic>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <thread>
#include <thread>
#include <unistd.h>

#include <cstdint> // 如果需要 uint32_t
#include <unordered_map>

class MmedStateMatchine : public MmedStateMatchineInterface {
  public:
    MmedStateMatchine(MmedPipelineManager &pipeline_mng,
                      MmedSettingManager &setting_mng);
    ~MmedStateMatchine();

    bool handleEvent(Event event, void *param);

    void initlize();

    void postMessage(const IpcMsgBuffer &msg) { m_block_queue.putTask(msg); }

  private:
    void start();
    void messageLoop();
    void initAllowedEvents();
    void initTranstionTable();
    bool decodeMessage(const IpcMsgBuffer &msg, uint32_t &e, void *&param);
    bool dispatchToExecutor(uint32_t event, void *param);

    MmedPipelineManager &m_pipeline_mng;
    MmedSettingManager &m_setting_mng;

    MmedState m_state{MmedState::PREVIEW};
    std::mutex m_mutex;
    std::thread m_thread;
    BlockingQueue<IpcMsgBuffer> m_block_queue;

    // state -> allowed events
    std::unordered_map<MmedState, std::vector<uint32_t>> m_allowedEvents; //

    // state + event -> next state
    std::unordered_map<MmedState, std::unordered_map<uint32_t, MmedState>>
        m_transtionTable;
};

#endif