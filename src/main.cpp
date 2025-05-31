#include "grep.h"
#include <iostream>
#include <string>
#include <filesystem>

void printHelp() 
{
    std::cout << "Usage: grep [options] pattern {file/directory}\n"
              << "Options:\n"
              << "  -i, --insensitive-case    Ignore case sensivity\n"
              << "  -r, --recursive           Recursively search\n"
              << "  -n, --line-number         Prefix each line with its number\n"
              << "  -h, --help                Display help message\n";
}

bool match_pattern(const std::string& inputLine, const std::string& pattern) {
    if (pattern == "/d") 
    {
        for(auto& i : inputLine)
        {
            if(std::isdigit(i)) return 1;
        }
        return 0;
    }

    if (pattern.length() == 1) {
        return inputLine.find(pattern) != std::string::npos;
    }
    else {
        throw std::runtime_error("Unhandled pattern " + pattern);
    }
}

int main(int argc, char* argv[]) 
{
    // Flush after every op
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;
    std::string inputLine;
    std::getline(std::cin, inputLine);

    if (argc != 3) {
        std::cerr << "Expected two arguments" << std::endl;
        return 1;
    }

    std::string flag = argv[1];
    std::string pattern = argv[2];

    if (flag != "-i") {
        std::cerr << "Expected first argument to be '-i'" << std::endl;
        return 1;
    }

    try {
        if (match_pattern(inputLine, pattern)) {
            std::cerr << "Pattern Found" << std::endl;
            return 0;
        } else {
            std::cerr << "Pattern not found" << std::endl;
            return 1;
        }
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
} 