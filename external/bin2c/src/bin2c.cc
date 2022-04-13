#include <iostream>
#include <fstream>
#include <filesystem>

int main(int argc, char** argv) {
    if (argc != 3)
        throw std::runtime_error("Expected two arguments: <variable name> <file name>");

    auto args = std::vector<std::string>(argv, argv+argc);

    std::string var_name = args[1];
    std::string file_name = args[2];

    const auto file_size = fs::file_size(file_name);
    if (file_size < 1)
        throw std::runtime_error("Cannot pack empty file");
}