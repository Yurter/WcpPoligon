#pragma once

#include <queue>
#include <mutex>

#define SEMICOLON_REQUIREMENT void(0)
#define sleep_for_ms(x) std::this_thread::sleep_for(std::chrono::milliseconds(x))
#define guaranteed_pop(queue,data) while (!queue.pop(data))  { sleep_for_ms(10); } SEMICOLON_REQUIREMENT

template <class Type>
class AsyncQueue
{

public:

    AsyncQueue() : _queue_capacity(100) {}
    ~AsyncQueue() { clear(); }

    [[nodiscard]] bool push(Type data)
    {
        std::lock_guard<std::mutex> lock(_queue_mutex);
        if (_queue.size() == _queue_capacity) {
            return false;
        }
        _queue.push(data);
        return true;
    }
    [[nodiscard]] bool pop(Type& data)
    {
        std::lock_guard<std::mutex> lock(_queue_mutex);
        if (_queue.empty()) { return false; }
        data = _queue.front();
        _queue.pop();
        return true;
    }
    bool empty()
    {
        std::lock_guard<std::mutex> lock(_queue_mutex);
        return _queue.empty();
    }
    bool full()
    {
        std::lock_guard<std::mutex> lock(_queue_mutex);
        return _queue.size() == _queue_capacity;
    }
    size_t size()
    {
        std::lock_guard<std::mutex> lock(_queue_mutex);
        return _queue.size();
    }
    void clear()
    {
        std::lock_guard<std::mutex> lock(_queue_mutex);
        std::queue<Type> empty;
        std::swap(_queue, empty);
    }

private:

    std::queue<Type>    _queue;
    std::mutex          _queue_mutex;
    uint64_t            _queue_capacity;

};

