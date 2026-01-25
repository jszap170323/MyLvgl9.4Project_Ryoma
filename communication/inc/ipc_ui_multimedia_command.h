#ifndef _IPC_UI_MULTIMEDIA_COMMAND_H
#define _IPC_UI_MULTIMEDIA_COMMAND_H
#include <cstdint>
#include <errno.h>
#include <errno.h>
#include <fcntl.h> /* For O_* constants */
#include <iostream>
#include <memory>
#include <mqueue.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <string>
#include <sys/ipc.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/msg.h>
#include <sys/stat.h> /* For mode constants */
#include <sys/types.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <vector>

// typedef struct {
//     uint32_t cmd;
//     uint32_t data_size;
//     uint8_t data[];
// } __attribute__((packed)) IpcCommandData;

enum class CmdType : uint32_t {
    CtrlType,
    SetParamType,
    GetParamType

};

enum class CtrlCmd : uint32_t {
    StartTakePhoto = 0x0001,
    StartRecVideo,
    StopRecVideo
};

enum class SetParamCmd : uint32_t {
    SetPreviewPixesMove = (0x1001),
    SetBackLightLevel
};

enum class GetParamCmd : uint32_t {
    GetShuterSpeed = (0x2001),
};

// enum class Event : uint32_t {
//     START_RECORD,
//     STOP_RECORD,
//     TAKE_PHOTO,
//     PREVIEW_OFFSET,
//     SET_PARAM,
//     GET_PARAM,
//     RESET,
// };

// struct EventHash {
//     std::size_t operator()(Event e) const noexcept {
//         return static_cast<std::size_t>(e);
//     }
// };
using Event = uint32_t;

#endif // !_IPC_UI_MULTIMEDIA_COMMAND_H