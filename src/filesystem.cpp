#include "filesystem.hpp"

#include <iostream>

void Filesystem::define_drive(char letter, std::filesystem::path&& path) {
    this->drives.emplace(letter, std::move(path));
}

int Filesystem::open_file(std::filesystem::path const& path) {
    for (auto p : path) {
        std::cout << p << "\n";
    }
}
