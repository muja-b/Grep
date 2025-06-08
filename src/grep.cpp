#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <regex>
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

void grep::traverseFiles(const std::filesystem::path& directoryPath, AtomicStack& fileStack)
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
            fileStack.push(message{entry.path()});
        }
    }
}

std::vector<std::string> grep::searchInFile(const std::filesystem::path& filePath, const std::string& pattern)
{
    auto result = std::vector<std::string>{};
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Could not open file: " << filePath << std::endl;
        return result;
    }
    std::string line;
    int lineNumber = 1;
    while (std::getline(file, line))
    {
        if (search(line, pattern))
        {
            std::ostringstream ss;
            ss << filePath << ": " << line << std::endl;
            if(m_showLines)
                ss << " -line ::" << lineNumber << std::endl;
            result.push_back(ss.str());
        }
        lineNumber++;
    }
    file.close();
    return result;
}

