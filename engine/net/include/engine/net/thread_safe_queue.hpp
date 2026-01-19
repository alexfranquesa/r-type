#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace engine::net {

template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    void push(const T& value) {
        {
            std::scoped_lock lock(mutex_);
            queue_.push(value);
        }
        cv_.notify_one();
    }

    void push(T&& value) {
        {
            std::scoped_lock lock(mutex_);
            queue_.push(std::move(value));
        }
        cv_.notify_one();
    }

    std::optional<T> try_pop() {
        std::scoped_lock lock(mutex_);
        if (queue_.empty()) {
            return std::nullopt;
        }
        auto value = std::move(queue_.front());
        queue_.pop();
        return value;
    }

    T wait_and_pop() {
        std::unique_lock lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty(); });
        auto value = std::move(queue_.front());
        queue_.pop();
        return value;
    }

    bool empty() const {
        std::scoped_lock lock(mutex_);
        return queue_.empty();
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<T> queue_;
};

}  // namespace engine::net
