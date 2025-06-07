#include "../include/grep.h"
#include <iostream>
#include <string>
#include "cxxopts.hpp"
#include <filesystem> // Uncomment this line to use <filesystem> in the future

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
        ("r,recursive", "Recursive search", cxxopts::value<bool>()->default_value("false"))
        ("n,line-number", "Prefix each line with its number", cxxopts::value<bool>()->default_value("false"))
        ("w,word-regex","Match whole words", cxxopts::value<bool>()->default_value("false"))
        ("p,pattern","Pattern to search for",cxxopts::value<std::string>())
        ("f,files","Files to search in",cxxopts::value<std::vector<std::string>>());

    try 
    {
        std::cout << "Parsing arguments..." << std::endl;
        std::cout.flush();
        
        auto result = options.parse(argc, argv);
        
        if (result.count("help")) {
            std::cout << options.help() << std::endl;
            return 0;
        }

        if (!result.count("pattern")) {
            std::cout << "Error: Pattern is required" << std::endl;
            std::cout << options.help() << std::endl;
            return 1;
        }

        std::cout << "Getting options..." << std::endl;
        std::cout.flush();

        bool caseSensitive = !result.count("i");
        bool recursive = result.count("r");
        bool showLines = result.count("n");
        bool invertMatch = result.count("v");
        bool matchWholeWord = result.count("w");
        std::string pattern = result["pattern"].as<std::string>();
        std::vector<std::string> files = result["files"].as<std::vector<std::string>>();
        
        std::cout << "Creating grep command..." << std::endl;
        std::cout.flush();
        
        grep grep_cmd(caseSensitive, recursive, showLines, matchWholeWord);
        std::cout << "Enter text to search (press Enter when done): " << std::endl;
        std::cout.flush();
        
        const auto inputLine = readInput();
        std::cout << "Searching for pattern: " << pattern << std::endl;
        std::cout.flush();
        
        if (grep_cmd.search(inputLine, pattern)) {
            std::cout << "Pattern Found" << std::endl;
        } else {
            std::cout << "Pattern not found" << std::endl;
        }
        std::cout << "Search completed." << std::endl;
        return 0;
    }
    catch(const std::exception& e)
    {
        std::cout << "Error parsing options: " << e.what() << std::endl;
        std::cout << options.help() << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cout.flush();
        std::cin.get();
        return 1;
    }
}