#include "ipc_message_queue.h"

IpcMessageQueue::IpcMessageQueue() {}
IpcMessageQueue::~IpcMessageQueue() {}

// Posix msgqueue
int IpcMessageQueue::createMessageQueue(const std::string &msg_queue_name,
                                        int max_msg_count, int max_msg_size) {
    struct mq_attr mqAttr = {};
    mqAttr.mq_maxmsg = max_msg_count;
    mqAttr.mq_msgsize = max_msg_size;
    m_max_msg_size = max_msg_size;
    m_max_msg_count = max_msg_count;

    m_mqd = mq_open(msg_queue_name.c_str(), O_CREAT | O_RDWR, 0666, &mqAttr);
    if (m_mqd == -1) {
        perror("mq_open");
        return -1;
    } else {
        return 0;
    }
}

// SystemV msgqueue
int IpcMessageQueue::createMsgQueue(const std::string &ipc_key) {
    key_t msg_key = ftok(ipc_key.c_str(), 'M');
    if (msg_key == -1) {
        perror("ftok");
        return -1;
    }

    m_msg_id = msgget(msg_key, IPC_CREAT | 0666);
    if (m_msg_id < 0) {
        ;
        perror("msgget");
        return -1;
    }
    printf("m_msg_id = %d\n", m_msg_id);
    return m_msg_id;
}

//  ssize_t IpcMessageQueue::readMsg(char* buf, ssize_t buf_len)
//  {
//     if (buf == nullptr || buf_len == 0) {
//         return -EINVAL;
//     }
//      ssize_t read_bytes = mq_receive(m_mqd, buf, buf_len, 0);
//      if (read_bytes < 0) {
//         if (errno == EINTR) {
//             return 0;           // 可重试
//         }
//         return -errno;
//     }

//     return read_bytes;
//  }

//  int IpcMessageQueue::sendMsg(const char* buf, ssize_t buf_len)
//  {
//     int send_bytes =  mq_send(m_mqd, buf, buf_len, 0);
//     if(send_bytes <0){
//          if (errno == EINTR) {
//             return 0;           // 可重试
//         }
//         return -errno;
//     }
//     return send_bytes;
//  }

// ssize_t IpcMessageQueue::readMsg(void *buf, size_t buf_len, long mtype) {
//     if (!buf || buf_len < sizeof(IpcMsgBuffer)) {
//         return -EINVAL;
//     }

//     // 临时 buffer：容量 = 最大允许消息
//     uint8_t tmp_buf[1024];
//     IpcMsgBuffer *msg = reinterpret_cast<IpcMsgBuffer *>(tmp_buf);

//     // ret = payload 长度（cmd + data_size + data）
//     ssize_t ret = msgrcv(m_msg_id, msg, sizeof(tmp_buf) - sizeof(long),
//     mtype,
//                          0 // 阻塞
//                          );

//     printf("mmed msgrcv size = %d\n", ret);
//     if (ret < 0) {
//         return -errno;
//     }

//     // 防御：最小 payload 校验
//     if (ret < static_cast<ssize_t>(sizeof(msg->cmd) +
//     sizeof(msg->data_size))) {
//         return -EBADMSG;
//     }

//     // 防御：payload 长度校验
//     if (msg->data_size > ret - sizeof(msg->cmd) - sizeof(msg->data_size)) {
//         return -EBADMSG;
//     }

//     size_t total_size = sizeof(long) + ret;

//     if (total_size > buf_len) {
//         return -EMSGSIZE;
//     }

//     memcpy(buf, msg, total_size);

//     printf("mmed msgrcv total_size = %d\n", total_size);

//     return total_size; // ⭐ 返回完整消息大小
// }

ssize_t IpcMessageQueue::readMsg(void *buf, size_t buf_len, long mtype) {
    if (!buf || buf_len < sizeof(IpcMsgBuffer)) {
        return -EINVAL;
    }

    // ⭐ 修正3：临时 buffer 大小必须足够容纳完整的 IpcMsgBuffer
    // System V 消息队列要求接收 buffer 足够大
    uint8_t tmp_buf[sizeof(IpcMsgBuffer)];
    IpcMsgBuffer *msg = reinterpret_cast<IpcMsgBuffer *>(tmp_buf);

    // ⭐ 修正4：接收时，期望接收最大的消息长度（即整个结构体 - mtype）
    // 这样可以保证收到完整的头部和数据
    size_t max_msgsz = sizeof(IpcMsgBuffer) - sizeof(long);

    // ret = 实际接收到的 payload 长度
    ssize_t ret = msgrcv(m_msg_id, msg, max_msgsz, mtype, 0);

    printf("mmed msgrcv size = %d\n", ret);
    if (ret < 0) {
        return -errno;
    }

    // 校验接收到的长度是否合法（至少包含 cmd_type, cmd, data_size）
    if (ret < static_cast<ssize_t>(sizeof(msg->cmd_type) + sizeof(msg->cmd) +
                                   sizeof(msg->data_size))) {
        return -EBADMSG;
    }

    // 校验 data_size 是否越界
    // 接收到的总长 ret - 头部长度 = 实际 data 长度
    size_t header_size =
        sizeof(msg->cmd_type) + sizeof(msg->cmd) + sizeof(msg->data_size);
    if (msg->data_size > ret - header_size) {
        // 防止恶意或错误的 data_size 导致读取越界
        return -EBADMSG;
    }

    // ⭐ 修正5：将接收到的完整结构体拷贝给用户
    // 我们拷贝 sizeof(IpcMsgBuffer) 是为了把 mtype 也带给用户（虽然这是可选的，
    // 但如果用户 buffer 是 IpcMsgBuffer，通常期望整个结构被填满）
    // 如果只填 payload 部分，用户可能读到未初始化的 data[] 尾部
    memset(buf, 0, sizeof(IpcMsgBuffer));   // 先清空用户 buffer
    memcpy(buf, msg, sizeof(IpcMsgBuffer)); // 拷贝整个结构

    printf("mmed msgrcv total_size copied = %d\n", sizeof(IpcMsgBuffer));

    return ret; // 返回实际 payload 长度
}

// ssize_t IpcMessageQueue::sendMsg(const void *buf, size_t buf_len, long mtype)
// ssize_t IpcMessageQueue::sendCommand(long mtype, uint32_t cmd_type,
//                                      uint32_t cmd, const void *payload,
//                                      uint32_t payload_len) {
//     if (mtype <= 0) {
//         return -EINVAL;
//     }

//     // payload_len 可以为 0（纯命令）
//     size_t total = sizeof(IpcMsgBuffer) + payload_len;
//     std::vector<uint8_t> buf(total);

//     auto *msg = reinterpret_cast<IpcMsgBuffer *>(buf.data());
//     msg->mtype = mtype;
//     msg->cmd_type = cmd_type;
//     msg->cmd = cmd;
//     msg->data_size = payload_len;

//     if (payload_len > 0) {
//         memcpy(msg->data, payload, payload_len);
//     }

//     // 返回：实际发送的 payload 字节数
//     return sendRaw(msg);
// }

ssize_t IpcMessageQueue::sendCommand(long mtype, uint32_t cmd_type,
                                     uint32_t cmd, const void *payload,
                                     uint32_t payload_len) {
    if (mtype <= 0) {
        return -EINVAL;
    }

    if (payload_len > MAX_PAYLOAD) {
        return -EMSGSIZE;
    }

    // ⭐ 修正1：直接在栈上或使用成员变量构建消息，不需要额外的 vector
    IpcMsgBuffer msg;
    memset(&msg, 0,
           sizeof(IpcMsgBuffer)); // ⭐ 非常重要：清空整个结构，防止脏数据

    msg.mtype = mtype;
    msg.cmd_type = cmd_type;
    msg.cmd = cmd;
    msg.data_size = payload_len;

    if (payload_len > 0 && payload != nullptr) {
        memcpy(msg.data, payload, payload_len); // 拷贝到结构体内部
    }

    // ⭐ 修正2：发送时，只发送 payload 部分（不包含 mtype）
    // 大小 = 头部(cmd_type + cmd + data_size) + 实际数据
    size_t msgsz = sizeof(msg.cmd_type) + sizeof(msg.cmd) +
                   sizeof(msg.data_size) + msg.data_size;

    // 发送整个结构体（除了 mtype）
    if (msgsnd(m_msg_id, &msg, msgsz, 0) == -1) {
        perror("msgsnd sendCommand");
        return -errno;
    }

    // printf("sendCommand done: size=%d, cmd=%d\n", msgsz, cmd);
    return msgsz; // 返回发送的字节数
}

ssize_t IpcMessageQueue::sendRaw(const IpcMsgBuffer *msg) {
    if (!msg || msg->mtype <= 0) {
        return -EINVAL;
    }

    // 防御：data_size 合理性（可按项目定义最大值）
    if (msg->data_size > 1024) {
        return -EMSGSIZE;
    }

    size_t msgsz = sizeof(msg->cmd) + sizeof(msg->data_size) + msg->data_size;

    if (msgsnd(m_msg_id, msg, msgsz, 0) == -1) {
        return -errno;
    }

    printf("sednRaw size = %d\n", msg->data_size);
    return msg->data_size; // ⭐ 统一语义：返回 payload 大小
}
