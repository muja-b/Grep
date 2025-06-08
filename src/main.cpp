#include "../include/grep.h"
#include "../include/atomic_stack.h"
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

void produce_files(const std::vector<std::string>& dirs, grep& grep_cmd, AtomicStack& fileStack) {
    for (const auto& d : dirs) {
        auto foundFiles = grep_cmd.traverseFiles(d);
        for (const auto& f : foundFiles) {
            fileStack.push({f});
        }
    }
}

void consume_files(AtomicStack& fileStack, grep& grep_cmd, const std::string& pattern, std::vector<std::string>& results) {
    while (true) {
        auto msgOpt = fileStack.pop();
        if (!msgOpt.has_value()) break; // Stack empty, producer done
        auto fileResults = grep_cmd.searchInFile(msgOpt->value, pattern);
        for (const auto& line : fileResults) {
            results.push_back(line);
        }
    }
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

        AtomicStack fileStack;

        // Producer thread: traverse files and push to stack
        std::thread producer([&] {
            for (const auto& d : dirs) {
                grep_cmd.traverseFiles(d, fileStack);
            }
            // Signal consumer to stop (poison pill)
            fileStack.push({"", true});
        });

        // Consumer thread: pop files and search
        std::thread consumer([&] {
            while (true) {
                auto msgOpt = fileStack.pop();
                if (!msgOpt.has_value() || msgOpt->self_destruct) break;
                auto fileResults = grep_cmd.searchInFile(msgOpt->value, pattern);
                for (const auto& line : fileResults) {
                    std::cout << line << std::endl;
                }
            }
        });

        producer.join();
        consumer.join();

        VLOG(1, debugMode) << "Producer and consumer threads completed.";
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