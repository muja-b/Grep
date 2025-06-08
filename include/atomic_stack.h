#pragma once
#include <stack>
#include <mutex>
#include <optional>
#include <filesystem>

struct message {
    std::filesystem::path value;
    bool self_destruct;
    message(const std::filesystem::path& val, bool destruct = false) : value(val),self_destruct(destruct) {}
};

class AtomicStack {
public:
    void push(const message& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_stack.push(value);
    }

    std::optional<message> pop() {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_stack.empty()) return std::nullopt;
        message value = m_stack.top();
        m_stack.pop();
        return value;
    }

private:
    std::stack<message> m_stack;
    std::mutex m_mutex;
};