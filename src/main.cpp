#include "../include/grep.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "cxxopts.hpp"
#include <filesystem>
#include <unordered_set>
#include <glog/logging.h>

const std::string readInput()
{
    std::string inputLine;
    std::getline(std::cin, inputLine);
    return inputLine;    
}

int main(int argc, char* argv[]) 
{
    // Force output buffering
    std::cout.setf(std::ios::unitbuf);
    std::cerr.setf(std::ios::unitbuf);
    google::InitGoogleLogging(argv[0]);
    
    cxxopts::Options options("grep", "Search for a pattern in a file or directory");
    
    options.add_options()
        ("h,help", "Display help message")
        ("i,insensitive-case", "Ignore case sensivity", cxxopts::value<bool>()->default_value("false"))
        ("r,recursive", "Recursive search", cxxopts::value<bool>()->default_value("true"))
        ("n,line-number", "Prefix each line with its number", cxxopts::value<bool>()->default_value("false"))
        ("w,word-regex","Match whole words", cxxopts::value<bool>()->default_value("false"))
        ("p,pattern","Pattern to search for",cxxopts::value<std::string>())
        ("di,dirs","Directories to search in",cxxopts::value<std::vector<std::string>>()->default_value({"."}))
        ("d,debug-mode","enable dubug messages", cxxopts::value<bool>()->default_value("false"));

    try 
    {
        VLOG(1) << "Detailed debug info";

        std::cout.flush();
        
        auto result = options.parse(argc, argv);
        bool debugMode = result["debug-mode"].as<bool>();
        VLOG(1, debugMode) << "Parsing arguments...";
        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            return 0;
        }

        if (!result.count("pattern")) {
            std::cout << "Error: Pattern is required" << std::endl;
            std::cout << options.help() << std::endl;
            return 1;
        }

        VLOG(1, debugMode) << "Getting options...";

        bool caseSensitive = !result.count("i");
        bool recursive = result.count("r");
        bool showLines = result.count("n");
        bool invertMatch = result.count("v");
        bool matchWholeWord = result.count("w");
        std::string pattern = result["pattern"].as<std::string>();
        std::vector<std::string> dirs = result["dirs"].as<std::vector<std::string>>();
        
        VLOG(1, debugMode) << "Creating grep command...";
        
        grep grep_cmd(caseSensitive, recursive, showLines, matchWholeWord);
        
        VLOG(1, debugMode) << "Searching for pattern: " << pattern;
        
        VLOG(1, debugMode) << "Counting files in specified Dirs : ";
        std::unordered_set<std::filesystem::path> files;
        std::vector<std::string> results;
        for (const auto& d : dirs) 
        {
            auto foundFiles = grep_cmd.traverseFiles(d);
            for (const auto& f : foundFiles) {
                files.insert(f);
            }
        }
        for (const auto& file : files) 
        {
            auto fileResults = grep_cmd.searchInFile(file, pattern);
            for(auto i : fileResults ) results.push_back(i);
        }
        VLOG(1, debugMode) << "Searching completed.";
        if (!results.empty()) 
        {
            for (const auto& result : results) 
            {
                std::cout << result << std::endl;
            }
        } else {
            VLOG(1, debugMode) << "Pattern not found";
        }
        VLOG(1, debugMode) << "Search completed.";
        return 0;
    }
    catch(const std::exception& e)
    {
        std::cout << "Error parsing options: " << e.what() << std::endl;
        std::cout << options.help() << std::endl;
        return 1;
    }
}