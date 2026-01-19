#include "engine/net/thread_safe_queue.hpp"
#include <mutex>

template<typename T>
void ThreadSafeQueue<T>::push(const T& value) {
    std::lock_guard<std::mutex> lock(mtx);
    q.push(value);
}

template<typename T>
bool ThreadSafeQueue<T>::pop(T& out) {
    std::lock_guard<std::mutex> lock(mtx);
    if (q.empty()) return false;
    out = q.front();
    q.pop();
    return true;
}