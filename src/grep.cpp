#include <string>
#include <filesystem>
#include <regex>
#include <iostream>
#include "../include/grep.h"

bool grep::search(const std::string& inputLine, const std::string& pattern)
{
    if (pattern == "/d") 
    {
        for (auto& i : inputLine)
        {
            if (std::isdigit(i)) return true;
        }
        return false;
    }
    if (pattern == "/w")
    {
        for (auto& c : inputLine)
            if (!std::isalnum(c) && c != '_') return false;
        return true;
    }

    if (pattern.length() == 1) {
        return inputLine.find(pattern) != std::string::npos;
    }
    else 
    {
        throw std::runtime_error("Unhandled pattern " + pattern);
    }
    return true;
}

