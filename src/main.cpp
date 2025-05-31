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

int main(int argc, char* argv[]) 
{
    std::cout << "dummy output" << std::endl;
    return 0;
} 