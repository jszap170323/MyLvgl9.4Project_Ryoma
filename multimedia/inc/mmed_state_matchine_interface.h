

#ifndef _MMED_STATEMATCHINE_INTERFACE_H
#define _MMED_STATEMATCHINE_INTERFACE_H
#include "ipc_message_queue.h"
enum class MmedState {
    PREVIEW,   // 预览中
    RECORDING, // 录像中
    CAPTURING, // 拍照中
    ERROR      // 错误态
};

namespace std {
template <> struct hash<MmedState> {
    size_t operator()(const MmedState &s) const noexcept {
        return static_cast<size_t>(s);
    }
};
}

class MmedStateMatchineInterface {

  public:
    virtual ~MmedStateMatchineInterface() = default;
    // virtual MmedState currentState() const = 0;

    // MmedState transferState(MmedState state) = 0;
    virtual void postMessage(const IpcMsgBuffer &msg) = 0;
};

#endif