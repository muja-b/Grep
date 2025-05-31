#pragma once

#include <string>

class grep
{
    grep(bool caseSensitive, bool recursive, bool showLines)
    : m_caseSensitive(caseSensitive),m_recursive(recursive),m_showLines(showLines){};
    bool search(const std::string& filename, std::string& pattern);
    void setCaseSensitive(bool value);
    void setRecursive(bool value);
    void setShowLineNumbers(bool value);
private: 
    bool m_caseSensitive;
    bool m_recursive;
    bool m_showLines;

};