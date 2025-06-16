#pragma once
#include <stack>
#include <mutex>
#include <optional>
#include <string>
#include <queue>
#include <condition_variable>
#include "validator.h"

template<typename T, typename Validator = DefaultValidator<T>>
struct Message {
    T value;
    bool self_destruct;
    Message(const T& val, bool destruct = false) : value(val), self_destruct(destruct) {}
};

struct ResultMessage {
    std::string result;
    bool self_destruct;
    ResultMessage(const std::string& res, bool destruct = false) : result(res), self_destruct(destruct) {}
};

template<typename T, typename Validator = DefaultValidator<T>>
class AtomicStack {
public:
    AtomicStack(Validator validator = Validator()) : m_validator(validator) {}

    void push(const Message<T>& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_validator.validate(value.value)) {
            m_stack.push(value);
        }
    }

    std::optional<Message<T>> pop() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_stack.empty()) return std::nullopt;
        Message<T> value = m_stack.top();
        m_stack.pop();
        return value;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_stack.empty();
    }

private:
    std::stack<Message<T>> m_stack;
    mutable std::mutex m_mutex;
    Validator m_validator;
};

class ResultQueue {
public:
    void push(const ResultMessage& result) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(result);
        m_cv.notify_one();
    }

    std::optional<ResultMessage> pop() {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.wait(lock, [this] { return !m_queue.empty(); });
        
        if (m_queue.empty()) return std::nullopt;
        
        ResultMessage result = m_queue.front();
        m_queue.pop();
        return result;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }

private:
    std::queue<ResultMessage> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
};