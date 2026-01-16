#ifndef _BLOCK_QUEUE_H
#define _BLOCK_QUEUE_H

#include <condition_variable>
#include <deque>
#include <mutex>

template <class T> class BlockingQueue {
  public:
    BlockingQueue() = default;
    ~BlockingQueue() = default;

    // 禁止拷贝（很重要）
    BlockingQueue(const BlockingQueue &) = delete;
    BlockingQueue &operator=(const BlockingQueue &) = delete;

    // 放入任务
    void putTask(const T &task) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push_back(task);
        }
        m_cond.notify_one();
    }

    // 支持 move
    void putTask(T &&task) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_queue.push_back(std::move(task));
        }
        m_cond.notify_one();
    }

    // 取任务（阻塞）
    T takeTask() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond.wait(lock, [&] { return !m_queue.empty(); });

        T task = std::move(m_queue.front());
        m_queue.pop_front();
        return task;
    }

    // 非阻塞尝试
    bool tryTake(T &out) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty())
            return false;

        out = std::move(m_queue.front());
        m_queue.pop_front();
        return true;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

  private:
    std::deque<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond;
};

#endif // _BLOCK_QUEUE_H
