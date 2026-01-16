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

ssize_t IpcMessageQueue::readMsg(void *buf, size_t buf_len, long mtype) {
    if (!buf || buf_len < sizeof(IpcMsgBuffer)) {
        return -EINVAL;
    }

    // 临时 buffer：容量 = 最大允许消息
    uint8_t tmp_buf[1024];
    IpcMsgBuffer *msg = reinterpret_cast<IpcMsgBuffer *>(tmp_buf);

    // ret = payload 长度（cmd + data_size + data）
    ssize_t ret = msgrcv(m_msg_id, msg, sizeof(tmp_buf) - sizeof(long), mtype,
                         0 // 阻塞
                         );

    printf("mmed msgrcv size = %d\n", ret);
    if (ret < 0) {
        return -errno;
    }

    // 防御：最小 payload 校验
    if (ret < static_cast<ssize_t>(sizeof(msg->cmd) + sizeof(msg->data_size))) {
        return -EBADMSG;
    }

    // 防御：payload 长度校验
    if (msg->data_size > ret - sizeof(msg->cmd) - sizeof(msg->data_size)) {
        return -EBADMSG;
    }

    size_t total_size = sizeof(long) + ret;

    if (total_size > buf_len) {
        return -EMSGSIZE;
    }

    memcpy(buf, msg, total_size);

    printf("mmed msgrcv total_size = %d\n", total_size);

    return total_size; // ⭐ 返回完整消息大小
}

// ssize_t IpcMessageQueue::sendMsg(const void *buf, size_t buf_len, long mtype)
ssize_t IpcMessageQueue::sendCommand(long mtype, uint32_t cmd,
                                     const void *payload,
                                     uint32_t payload_len) {
    if (mtype <= 0) {
        return -EINVAL;
    }

    // payload_len 可以为 0（纯命令）
    size_t total = sizeof(IpcMsgBuffer) + payload_len;
    std::vector<uint8_t> buf(total);

    auto *msg = reinterpret_cast<IpcMsgBuffer *>(buf.data());
    msg->mtype = mtype;
    msg->cmd = cmd;
    msg->data_size = payload_len;

    if (payload_len > 0) {
        memcpy(msg->data, payload, payload_len);
    }

    // 返回：实际发送的 payload 字节数
    return sendRaw(msg);
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
