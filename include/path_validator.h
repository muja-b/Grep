#pragma once
#include <filesystem>
#include <unordered_set>
#include <system_error>
#include "validator.h"

class PathValidator : public IValidator<std::filesystem::path> {
public:
    bool validate(const std::filesystem::path& value) const override {
        std::error_code ec;
        auto canonical = std::filesystem::canonical(value, ec);
        if (ec) return false;
        size_t hash = std::hash<std::string>{}(canonical.string());
        return m_seen_hashes.insert(hash).second;
    }

private:
    mutable std::unordered_set<size_t> m_seen_hashes;
}; 