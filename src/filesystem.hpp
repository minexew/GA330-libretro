#ifndef GA330_FILESYSTEM_HPP
#define GA330_FILESYSTEM_HPP

#include <filesystem>
#include <fstream>
#include <unordered_map>

// our C++20 clutch
#include <gsl/span>

class Filesystem {
public:
    void define_drive(char letter, std::filesystem::path&& path);

    void close_file(int handle);
    int open_file(std::filesystem::path const& path, std::string const& mode);
    gsl::span<std::byte> read_file(int handle, gsl::span<std::byte> output);
    void seek_file(int handle, int offset, int c_mode);
    int tell_file(int handle);
    gsl::span<std::byte const> write_file(int handle, gsl::span<std::byte const> input);

private:
    int next_handle = 1;
    std::unordered_map<char, std::filesystem::path> drives;
    std::unordered_map<int, std::fstream> open_files;
};

#endif
