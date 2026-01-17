#ifndef _IPC_MESSAGE_QUEUE_H
#define _IPC_MESSAGE_QUEUE_H

#include "ipc_ui_multimedia_command.h"
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
//     long mtype;
//     uint32_t cmd;
//     uint32_t data_size;
//     uint8_t data[];
// } __attribute__((packed)) IpcMsgBuffer;

// typedef struct {
//     long mtype;
//     uint8_t payload[1024]; // msg payload
// } __attribute__((packed)) IpcMsgBuffer;

// typedef struct {
//     uint32_t cmd;
//     uint32_t data_size;
//     uint8_t data[]; // flexible array member
// } __attribute__((packed)) IpcCommandData;

// Flexible payload
typedef struct __attribute__((packed)) {
    uint32_t cmd;
    uint32_t data_size;
    uint8_t data[];
} IpcCommandData;

// // System V 消息
// typedef struct __attribute__((packed)) {
//     long mtype; // 必须 > 0
//     uint32_t cmd_type;
//     uint32_t cmd;
//     uint32_t data_size;
//     uint8_t data[512]; // flexible array
// } IpcMsgBuffer;

constexpr size_t MAX_PAYLOAD = 512;

struct IpcMsgBuffer {
    long mtype;
    uint32_t cmd_type;
    uint32_t cmd;
    uint32_t data_size;
    uint8_t data[MAX_PAYLOAD];
};

const key_t IPC_KEY = 0x4D4D0001;
const std::string ui_to_multimedia_ipc_key = "/IPC_KEY_UI_TO_MULTIMEDIA";
const std::string multimedia_to_ui_ipc_key = "/IPC_KEY_MULTIMEDIA_TO_UI";

class IpcMessageQueue {
  public:
    IpcMessageQueue();

    ~IpcMessageQueue();

    int createMessageQueue(const std::string &msg_queue_name, int max_msg_count,
                           int max_msg_size);
    int createMsgQueue(const std::string &ipc_key);

    ssize_t readMsg(void *buf, size_t buf_len, long mtype);
    // ssize_t sendMsg(const void *buf, size_t buf_len, long mtype);
    ssize_t sendCommand(long mtype, uint32_t cmd_type, uint32_t cmd,
                        const void *payload, uint32_t payload_len);

  private:
    ssize_t sendRaw(const IpcMsgBuffer *msg);

    mqd_t m_mqd;
    int m_max_msg_count;
    int m_max_msg_size;
    int m_msg_id;
};

#endif