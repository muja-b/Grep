#include <string>
#include <filesystem>
#include <iostream>
#include "../include/grep.h"

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

