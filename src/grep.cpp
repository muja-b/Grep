#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <future>
#include "../include/grep.h"
#include "grep.h"
#include "../include/atomic_stack.h"

bool containsWholeWord(const std::string& inputLine, const std::string& pattern)
{
    std::string word = " " + pattern + " ";
    std::string line = " " + inputLine + " ";
    return line.find(word) != std::string::npos;
}

bool grep::search(const std::string& inputLine, const std::string& pattern)
{
    if (m_matchWholeWord && containsWholeWord(inputLine, pattern)) 
    {
        return true;
    }
    else if (m_caseSensitive && (inputLine.find(pattern) != std::string::npos))
    {
        return true;
    }
    else {
        std::string lowerInput = inputLine, lowerPattern = pattern;
        std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);
        std::transform(lowerPattern.begin(), lowerPattern.end(), lowerPattern.begin(), ::tolower);
        if(lowerInput.find(lowerPattern) != std::string::npos)
        {
            return true;
        }
    }
    try {
        std::regex_constants::syntax_option_type flags = std::regex_constants::ECMAScript;
        if (!m_caseSensitive) flags |= std::regex_constants::icase;
        std::regex re(pattern, flags);
        return std::regex_search(inputLine, re);
    } catch (const std::regex_error& e) {
        std::cerr << "Invalid regex pattern: " << e.what() << std::endl;
        return false;
    }
    return false;
}

void grep::traverseFiles(const std::filesystem::path& directoryPath, AtomicStack<std::filesystem::path>& fileStack)
{
    auto it = std::filesystem::recursive_directory_iterator(directoryPath);
    if (!m_recursive)
    {
        it.disable_recursion_pending();
    }
    for (const auto& entry : it)
    {
        if (entry.is_regular_file())
        {
            fileStack.push(Message<std::filesystem::path>{entry.path()});
        }
    }
}

struct ChunkInfo {
    size_t start;
    size_t end;
    int startLineNumber;
};

std::vector<ChunkInfo> splitFileIntoChunks(const std::filesystem::path& filePath, size_t numChunks) {
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        return {};
    }

    size_t fileSize = file.tellg();
    size_t chunkSize = fileSize / numChunks;
    std::vector<ChunkInfo> chunks;
    chunks.reserve(numChunks);

    file.seekg(0);
    std::string line;
    size_t currentPos = 0;
    int currentLine = 1;

    for (size_t i = 0; i < numChunks; i++) {
        ChunkInfo chunk;
        chunk.start = currentPos;
        chunk.startLineNumber = currentLine;

        size_t targetPos = (i + 1) * chunkSize;
        if (i == numChunks - 1) {
            targetPos = fileSize;
        }

        file.seekg(targetPos);
        std::getline(file, line);
        currentPos = file.tellg();
        currentLine += std::count(line.begin(), line.end(), '\n') + 1;

        chunk.end = currentPos;
        chunks.push_back(chunk);
    }

    return chunks;
}

void grep::processChunk(const std::filesystem::path& filePath, 
                       const ChunkInfo& chunk,
                       const std::string& pattern,
                       AtomicStack<std::string>& resultStack) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return;
    }

    constexpr size_t BUFFER_SIZE = 1024 * 1024;  // 1MB buffer
    std::vector<char> buffer(BUFFER_SIZE);
    file.rdbuf()->pubsetbuf(buffer.data(), BUFFER_SIZE);

    file.seekg(chunk.start);
    
    std::string line;
    line.reserve(4096);
    int lineNumber = chunk.startLineNumber;

    constexpr size_t LOCAL_BUFFER_SIZE = 100;
    std::vector<std::string> localResults;
    localResults.reserve(LOCAL_BUFFER_SIZE);

    while (file.tellg() < chunk.end && std::getline(file, line)) {
        if (search(line, pattern)) {
            std::ostringstream ss;
            ss << filePath << ": ";
            if(m_showLines)
                ss << "line " << lineNumber << ": ";
            ss << line;
            
            localResults.push_back(ss.str());

            if (localResults.size() >= LOCAL_BUFFER_SIZE) {
                for (const auto& result : localResults) {
                    resultStack.push(Message<std::string>{result});
                }
                localResults.clear();
            }
        }
        lineNumber++;
        line.clear();
    }

    for (const auto& result : localResults) {
        resultStack.push(Message<std::string>{result});
    }
}

void grep::searchInFile(const std::filesystem::path& filePath, const std::string& pattern, AtomicStack<std::string>& resultStack)
{
    std::error_code ec;
    auto fileSize = std::filesystem::file_size(filePath, ec);
    if (ec) {
        std::cerr << "Error getting file size: " << filePath << " - " << ec.message() << std::endl;
        return;
    }

    constexpr size_t CHUNK_SIZE = 100 * 1024 * 1024;
    size_t numChunks = std::max(1ULL, fileSize / CHUNK_SIZE);
    numChunks = std::min(numChunks, static_cast<size_t>(std::thread::hardware_concurrency()));

    auto chunks = splitFileIntoChunks(filePath, numChunks);
    if (chunks.empty()) {
        std::cerr << "Error splitting file into chunks: " << filePath << std::endl;
        return;
    }

    std::vector<std::future<void>> futures;
    futures.reserve(chunks.size());
    for (const auto& chunk : chunks) {
        futures.push_back(std::async(std::launch::async, [this, &filePath, &chunk, &pattern, &resultStack]() {
            processChunk(filePath, chunk, pattern, resultStack);
        }));
    }

    for (auto& future : futures) {
        future.wait();
    }
}

