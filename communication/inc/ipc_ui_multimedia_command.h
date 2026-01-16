#ifndef _IPC_UI_MULTIMEDIA_COMMAND_H
#define _IPC_UI_MULTIMEDIA_COMMAND_H
#include <cstdint>

// typedef struct {
//     uint32_t cmd;
//     uint32_t data_size;
//     uint8_t data[];
// } __attribute__((packed)) IpcCommandData;

enum class CtrlCmd : uint32_t {
    StartTakePhoto = 0,
    StartRecVideo,
    StopRecVideo
};

enum class SetParamCmd : uint32_t { SetShuterSpeed = (1 << 10) };

enum class GetParamCmd : uint32_t {
    GetShuterSpeed = (2 << 10),
};

#endif // !_IPC_UI_MULTIMEDIA_COMMAND_H