#pragma once

#include <string>
#include <vector>
#include <filesystem>

class grep
{
public:
    grep(bool caseSensitive, bool recursive, bool showLines, bool matchWholeWord)
    : m_caseSensitive(caseSensitive),m_recursive(recursive),m_showLines(showLines)
    ,m_matchWholeWord(matchWholeWord){};
    bool search(const std::string& inputLine, const std::string& pattern);
    std::vector<std::filesystem::path> traverseFiles(const std::string& directoryPath, const std::string& pattern);
    std::pair<std::string, bool> searchInFile(const std::filesystem::path &filePath, const std::string &pattern);

private: 
    bool m_caseSensitive;
    bool m_recursive;
    bool m_showLines;
    bool m_invertMatch;
    bool m_matchWholeWord; 

};