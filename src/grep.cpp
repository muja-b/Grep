#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include "../include/grep.h"
#include "grep.h"

bool containsWholeWord(const std::string& inputLine, const std::string& pattern)
{
    std::string word = " " + pattern + " ";
    std::string line = " " + inputLine + " ";
    return line.find(word) != std::string::npos;
}

bool grep::search(const std::string& inputLine, const std::string& pattern)
{
    if (m_matchWholeWord) 
    {
        return containsWholeWord(inputLine, pattern);
    }
    else if (pattern == "/d") 
    {
        for (auto& i : inputLine)
        {
            if (std::isdigit(i)) return true;
        }
        return false;
    }
    else if (pattern == "/w")
    {
        for (auto& c : inputLine)
            if (!std::isalnum(c) && c != '_') return false;
        return true;
    }
    else if (m_caseSensitive)
    {
        return inputLine.find(pattern) != std::string::npos;
    }
    else if (pattern.length() == 1) {
        std::string lowerInput = inputLine, lowerPattern = pattern;
        std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);
        std::transform(lowerPattern.begin(), lowerPattern.end(), lowerPattern.begin(), ::tolower);
        return lowerInput.find(lowerPattern) != std::string::npos;
    }
    else 
    {
        throw std::runtime_error("Unhandled pattern " + pattern);
    }
    return true;
}

std::vector<std::filesystem::path> grep::traverseFiles(const std::string& directoryPath, const std::string& pattern)
{
    std::vector<std::filesystem::path> files;
    auto it = std::filesystem::recursive_directory_iterator(directoryPath);
    if (!m_recursive)
    {
        it.disable_recursion_pending();
    }
    for (const auto& entry : it)
    {
        if (entry.is_regular_file())
        {
            files.push_back(entry.path());
        }
    }
    return files;
}

std::pair<std::string, bool> grep::searchInFile(const std::filesystem::path& filePath, const std::string& pattern)
{
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        std::cerr << "Could not open file: " << filePath << std::endl;
        return std::pair("", false);
    }
    std::string line;
    int lineNumber = 1;
    if (m_matchWholeWord && pattern.length() < 2)
    {
        std::cerr << "Pattern must be at least 2 characters long for whole word search." << std::endl;
        return std::pair("", false);
    }
    while (std::getline(file, line))
    {
        if (search(line, pattern))
        {
            std::ostringstream ss;
            ss << filePath << ": " << line << std::endl;
            if(m_showLines)
                ss << " -line ::" << lineNumber++ << std::endl;
            return std::pair(ss.str(), true);
        }
        lineNumber++;
    }
    file.close();
    std::cerr << "Pattern not found in file: " << filePath << std::endl;
    return std::pair("", false);
}

