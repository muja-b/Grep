#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <future>
#include "../include/atomic_stack.h"

class grep
{
public:
    grep(bool caseSensitive, bool recursive, bool showLines, bool matchWholeWord)
        : m_caseSensitive(caseSensitive), m_recursive(recursive), m_showLines(showLines), m_matchWholeWord(matchWholeWord) {}
    
    bool search(const std::string& inputLine, const std::string& pattern);
    void traverseFiles(const std::filesystem::path& directoryPath, AtomicStack<std::filesystem::path>& fileStack);
    void searchInFile(const std::filesystem::path& filePath, const std::string& pattern, AtomicStack<std::string>& resultStack);

private:
    bool m_caseSensitive;
    bool m_recursive;
    bool m_showLines;
    bool m_invertMatch;
    bool m_matchWholeWord;

    // Helper method for parallel processing
    void processChunk(const std::filesystem::path& filePath, 
                     const struct ChunkInfo& chunk,
                     const std::string& pattern,
                     AtomicStack<std::string>& resultStack);
};