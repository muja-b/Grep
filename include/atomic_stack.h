#pragma once
#include <stack>
#include <mutex>
#include <optional>
#include <filesystem>
#include <unordered_set>
#include <system_error>

struct message {
    std::filesystem::path value;
    bool self_destruct;
    message(const std::filesystem::path& val, bool destruct = false) : value(val),self_destruct(destruct) {}
};

class AtomicStack {
public:
    void push(const message& value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::error_code ec;
        auto canonical = std::filesystem::canonical(value.value, ec);
        if (ec) return;
        size_t hash = std::hash<std::string>{}(canonical.string());
        if (m_seen_hashes.insert(hash).second) 
        {
            m_stack.push(value);
            return;
        }
        return;
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
    std::unordered_set<size_t> m_seen_hashes;
};