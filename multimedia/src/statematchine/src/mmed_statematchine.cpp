#include "mmed_statematchine.h"

MmedStateMatchine::MmedStateMatchine(MmedPipelineManager &pipeline_mng,
                                     MmedSettingManager &setting_mng)
    : m_pipeline_mng(pipeline_mng), m_setting_mng(setting_mng),
      m_state(MmedState::PREVIEW) {}
MmedStateMatchine::~MmedStateMatchine() {}
void MmedStateMatchine::initlize() {
    initAllowedEvents();
    initTranstionTable();
    start();
}

void MmedStateMatchine::start() {
    m_thread = std::thread(&MmedStateMatchine::messageLoop, this);
}

void MmedStateMatchine::messageLoop() {
    while (true) {
        IpcMsgBuffer msg = m_block_queue.takeTask();

        Event e;
        void *param = nullptr;

        // if (!decodeMessage(msg, e, param))
        //     continue;

        handleEvent(msg.cmd, msg.data);
    }
}

void MmedStateMatchine::initAllowedEvents() {
    // m_allowedEvents[MmedState::PREVIEW] = {};

    // m_allowedEvents[MmedState::RECORDING] = {};

    // m_allowedEvents[MmedState::CAPTURING] = {
    //     // UI 不允许打断
    // };

    // m_allowedEvents[MmedState::ERROR] = {};
}
void MmedStateMatchine::initTranstionTable() {

    // ---------- PREVIEW ----------
    m_transtionTable[MmedState::PREVIEW] = {
        {static_cast<uint32_t>(CtrlCmd::StartTakePhoto), MmedState::PREVIEW}};

    m_transtionTable[MmedState::PREVIEW] = {
        {static_cast<uint32_t>(SetParamCmd::SetBackLightLevel),
         MmedState::PREVIEW}};

    // // ---------- RECORDING ----------
    // m_transtionTable[MmedState::RECORDING] = {
    //     {Event::STOP_RECORD, MmedState::PREVIEW},
    // };

    // // ---------- ERROR ----------
    // m_transtionTable[MmedState::ERROR] = {
    //     {Event::RESET, MmedState::PREVIEW},
    // };
}

bool MmedStateMatchine::handleEvent(uint32_t event, void *param) {
    std::cout << "MmedStateMatchine::handleEvent event = " << event
              << std::endl;
    std::lock_guard<std::mutex> lock(m_mutex);

    // 1️⃣ 查状态迁移是否合法
    auto stateIt = m_transtionTable.find(m_state);
    if (stateIt == m_transtionTable.end()) {
        std::cout << "stateIt == m_transtionTable.end()" << std::endl;
        return false;
    }

    auto eventIt = stateIt->second.find(event);
    if (eventIt == stateIt->second.end()) {
        std::cout << "eventIt == stateIt->second.end()" << std::endl;
        return false;
    }

    // 2️⃣ 执行动作（FSM 不关心细节）
    if (!dispatchToExecutor(event, param)) {
        std::cout << "dispatchToExecutor false" << std::endl;
        return false;
    }

    // 3️⃣ 状态迁移
    m_state = eventIt->second;
    return true;
}

bool MmedStateMatchine::decodeMessage(const IpcMsgBuffer &msg, uint32_t &e,
                                      void *&param) {
    // switch (static_cast<CmdType>(msg.cmd_type)) {
    // case CmdType::CtrlType:
    //     switch (static_cast<CtrlCmd>(msg.cmd)) {
    //     case CtrlCmd::StartRecVideo:
    //         e = Event::START_RECORD;
    //         break;
    //     case CtrlCmd::StopRecVideo:
    //         e = Event::STOP_RECORD;
    //         break;
    //     case CtrlCmd::StartTakePhoto:
    //         e = Event::TAKE_PHOTO;
    //         break;
    //     }
    //     param = nullptr;
    //     return true;

    // case CmdType::SetParamType:
    //     e = Event::PREVIEW_OFFSET;
    //     // param = msg.data; //
    //     return true;

    // case CmdType::GetParamType:
    //     e = Event::GET_PARAM;
    //     // param = msg.data; //
    //     return true;
    // }
    return false;
}

bool MmedStateMatchine::dispatchToExecutor(uint32_t event, void *param) {

    // u_int32_t event_t = static_cast<u_int32_t>(event);
    if (m_pipeline_mng.canHandle(event)) {
        printf("====m_pipeline_mng.canHandle(event)\n");
        return m_pipeline_mng.handle(event, param);
    }

    if (m_setting_mng.canHandle(event)) {
        printf("====m_setting_mng.canHandle(event)\n");
        return m_setting_mng.handle(event, param);
    }

    printf("====not found\n");

    return false;
}
