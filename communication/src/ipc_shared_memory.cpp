#include "ipc_shared_memory.h"

IpcSharedMemory::IpcSharedMemory(const std::string &name, size_t size,
                                 bool create)
    : name_(name), size_(size), shm_fd_(-1), shm_ptr_(nullptr), owner_(create) {
    if (create) {
        shm_fd_ = shm_open(name_.c_str(), O_CREAT | O_RDWR, 0666);
        if (shm_fd_ < 0) {
            perror("shm_open create");
            return;
        }
        if (ftruncate(shm_fd_, size_) < 0) {
            perror("ftruncate");
            close(shm_fd_);
            return;
        }
    } else {
        shm_fd_ = shm_open(name_.c_str(), O_RDWR, 0666);
        if (shm_fd_ < 0) {
            perror("shm_open open");
            return;
        }
    }

    shm_ptr_ =
        mmap(nullptr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0);
    if (shm_ptr_ == MAP_FAILED) {
        perror("mmap");
        shm_ptr_ = nullptr;
        close(shm_fd_);
    }
}

IpcSharedMemory::~IpcSharedMemory() {
    if (shm_ptr_) {
        munmap(shm_ptr_, size_);
    }
    if (shm_fd_ >= 0) {
        close(shm_fd_);
    }
    if (owner_) {
        shm_unlink(name_.c_str());
    }
}

bool IpcSharedMemory::write(const void *data, size_t size, size_t offset) {
    // printf("===========write &m_write = %0x,&m_read = %0x",&m_write,&m_read);
    if (!shm_ptr_ || offset + size > size_)
        return false;
    // printf("==============write before");
    m_write.wait();
    // printf("==============write after");
    std::lock_guard<std::mutex> lock(mtx_);
    memcpy(static_cast<uint8_t *>(shm_ptr_) + offset, data, size);
    m_read.post();
    //	printf("==============read post");
    return true;
}

bool IpcSharedMemory::read(void *data, size_t size, size_t offset) {
    //  printf("===========read &m_write = %0x,&m_read = %0x", &m_write,
    //  &m_read);
    if (!shm_ptr_ || offset + size > size_)
        return false;
    //  printf("==============read before");
    m_read.wait();
    //  printf("==============read after");
    std::lock_guard<std::mutex> lock(mtx_);
    memcpy(data, static_cast<uint8_t *>(shm_ptr_) + offset, size);
    m_write.post();
    //  printf("==============write post");
    return true;
}
