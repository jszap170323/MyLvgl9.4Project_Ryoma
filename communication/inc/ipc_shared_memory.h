#ifndef _IPC_SHARED_MEMORY_H
#define _IPC_SHARED_MEMORY_H

#include <string>
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <mutex>

class IpcSharedMemory {
public:
    IpcSharedMemory(const std::string& name, size_t size, bool create = true);
    ~IpcSharedMemory();

    bool write(const void* data, size_t size, size_t offset = 0);
    bool read(void* data, size_t size, size_t offset = 0);

    void* getBuffer() const { return shm_ptr_; }
    size_t getSize() const { return size_; }

private:
    std::string name_;
    size_t size_;
    int shm_fd_;
    void* shm_ptr_;
    bool owner_; // 是否创建者
    std::mutex mtx_; // 线程内安全
};

#endif /* _IPC_SHARED_MEMORY_H */
