#ifndef GA330_FILESYSTEM_HPP
#define GA330_FILESYSTEM_HPP

#include <filesystem>
#include <fstream>
#include <unordered_map>

class Filesystem {
public:
    void define_drive(char letter, std::filesystem::path&& path);

    int open_file(std::filesystem::path const& path);

private:
    int next_handle = 1;
    std::unordered_map<char, std::filesystem::path> drives;
    std::unordered_map<int, std::fstream> open_files;
};

#endif
