//
// Created by dmerkushov on 09.01.23.
//

#ifndef SOAPSTAB_SYNCHRONIZED_QUEUE_H
#define SOAPSTAB_SYNCHRONIZED_QUEUE_H

#include <memory>
#include <mutex>
#include <queue>

namespace soapstab {

template<typename T>
class synchronized_queue {
public:
    synchronized_queue();
    ~synchronized_queue();

    std::shared_ptr<T> pop();
    void push(std::shared_ptr<T> item);

    size_t size();
    bool empty();

private:
    std::deque<std::shared_ptr<T>> _queue;
    std::mutex _mutex;
};

template<typename T>
synchronized_queue<T>::synchronized_queue() {
}

template<typename T>
synchronized_queue<T>::~synchronized_queue() {
}

template<typename T>
std::shared_ptr<T> synchronized_queue<T>::pop() {
    std::unique_lock<std::mutex> l(_mutex);

    std::shared_ptr<T> result;
    if(!_queue.empty()) {
        result = _queue.front();
        _queue.pop_front();
    }

    return result;
}

template<typename T>
void synchronized_queue<T>::push(std::shared_ptr<T> item) {
    std::unique_lock<std::mutex> l(_mutex);

    _queue.push_back(item);
}

template<typename T>
size_t synchronized_queue<T>::size() {
    std::unique_lock<std::mutex> l(_mutex);
    size_t size = _queue.size();
    return size;
}

template<typename T>
bool synchronized_queue<T>::empty() {
    std::unique_lock<std::mutex> l(_mutex);
    bool empty = _queue.empty();
    return empty;
}

} // namespace soapstab

#endif // SOAPSTAB_SYNCHRONIZED_QUEUE_H
