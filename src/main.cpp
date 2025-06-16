#include "../include/grep.h"
#include "../include/atomic_stack.h"
#include "../include/path_validator.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "cxxopts.hpp"
#include <filesystem>
#include <unordered_set>

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
    
    cxxopts::Options options("grep", "Search for a pattern in a file or directory");
    options.add_options()
        ("h,help", "Display help message")
        ("i,insensitive-case", "Ignore case sensivity", cxxopts::value<bool>()->default_value("false"))
        ("r,recursive", "Recursive search", cxxopts::value<bool>()->default_value("true"))
        ("n,line-number", "Prefix each line with its number", cxxopts::value<bool>()->default_value("false"))
        ("w,word-regex","Match whole words", cxxopts::value<bool>()->default_value("false"))
        ("p,pattern","Pattern to search for",cxxopts::value<std::string>())
        ("dirs","Directories to search in",cxxopts::value<std::vector<std::string>>()->default_value({"."}))
        ("d,debug-mode","enable debug messages", cxxopts::value<bool>()->default_value("false"));

    try 
    {
        auto result = options.parse(argc, argv);
        bool debugMode = result["debug-mode"].as<bool>();
        
        if (debugMode) {
            std::cout << "Detailed debug info" << std::endl;
            std::cout << "Parsing arguments..." << std::endl;
        }

        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            return 0;
        }

        if (!result.count("pattern")) {
            std::cout << "Error: Pattern is required" << std::endl;
            std::cout << options.help() << std::endl;
            return 1;
        }

        if (debugMode) {
            std::cout << "Getting options..." << std::endl;
        }

        bool caseSensitive = !result.count("i");
        bool recursive = result.count("r");
        bool showLines = result.count("n");
        bool invertMatch = result.count("v");
        bool matchWholeWord = result.count("w");
        std::string pattern = result["pattern"].as<std::string>();
        std::vector<std::string> dirs = result["dirs"].as<std::vector<std::string>>();
        
        if (debugMode) {
            std::cout << "Creating grep command..." << std::endl;
            std::cout << "Searching for pattern: " << pattern << std::endl;
        }

        grep grep_cmd(caseSensitive, recursive, showLines, matchWholeWord);
        AtomicStack<std::filesystem::path, PathValidator> fileStack;
        AtomicStack<std::string> resultStack;

        // Producer thread: traverse files and push to stack
        std::thread producer([&] {
            for (const auto& d : dirs) {
                grep_cmd.traverseFiles(d, fileStack);
            }
            // Signal consumer to stop (poison pill)
            fileStack.push(Message<std::filesystem::path>{"", true});
        });

        // Consumer thread: pop files and search
        std::thread consumer([&] {
            while (true) {
                auto msgOpt = fileStack.pop();
                if (!msgOpt.has_value() || msgOpt->self_destruct) {
                    // Signal printer to stop
                    resultStack.push(Message<std::string>{"", true});
                    break;
                }
                grep_cmd.searchInFile(msgOpt->value, pattern, resultStack);
            }
        });

        // Printer thread: pop results and print them
        std::thread printer([&] {
            constexpr size_t BUFFER_SIZE = 1000;  // Buffer 1000 results before flushing
            std::vector<std::string> outputBuffer;
            outputBuffer.reserve(BUFFER_SIZE);
            
            try {
                while (true) {
                    auto resultOpt = resultStack.pop();
                    if (!resultOpt.has_value() || resultOpt->self_destruct) {
                        // Flush any remaining results
                        if (!outputBuffer.empty()) {
                            for (const auto& result : outputBuffer) {
                                std::cout << result << std::endl;
                            }
                            outputBuffer.clear();
                        }
                        break;
                    }

                    // Add result to buffer
                    outputBuffer.push_back(resultOpt->value);

                    // Flush buffer if it's full
                    if (outputBuffer.size() >= BUFFER_SIZE) {
                        for (const auto& result : outputBuffer) {
                            std::cout << result << std::endl;
                        }
                        outputBuffer.clear();
                    }
                }
            } catch (const std::exception& e) {
                std::cerr << "Error in printer thread: " << e.what() << std::endl;
                // Signal other threads to stop
                resultStack.push(Message<std::string>{"", true});
            }
        });

        producer.join();
        consumer.join();
        printer.join();

        if (debugMode) {
            std::cout << "All threads completed." << std::endl;
            std::cout << "Search completed." << std::endl;
        }
        return 0;
    }
    catch(const std::exception& e)
    {
        std::cout << "Error parsing options: " << e.what() << std::endl;
        std::cout << options.help() << std::endl;
        return 1;
    }
}