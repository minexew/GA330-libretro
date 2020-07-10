#include "filesystem.hpp"

#include <iostream>

void Filesystem::define_drive(char letter, std::filesystem::path&& path) {
    this->drives.emplace(letter, std::move(path));
}

void Filesystem::close_file(int handle) {
    this->open_files[handle].close();
    this->open_files.erase(handle);
}

int Filesystem::open_file(std::filesystem::path const& path, std::string const& mode) {
    std::string path_str {path};

    // Sanitize path
    std::replace(path_str.begin(), path_str.end(), '\\', '$');
    std::replace(path_str.begin(), path_str.end(), '/', '$');
    std::replace(path_str.begin(), path_str.end(), ':', '_');

    if (mode.substr(0, 2) == "rb") {
        std::fstream f(path_str, std::ios::in | std::ios::binary);

        if (f.is_open()) {
            auto h = this->next_handle;
            this->open_files.emplace(h, std::move(f));
            this->next_handle++;
            return h;
        }
    }
    else if (mode.substr(0, 2) == "wb") {
        std::fstream f(path_str, std::ios::out | std::ios::binary | std::ios::trunc);

        if (f.is_open()) {
            auto h = this->next_handle;
            this->open_files.emplace(h, std::move(f));
            this->next_handle++;
            return h;
        }
    }

    return -1;
}

gsl::span<std::byte> Filesystem::read_file(int handle, gsl::span<std::byte> output) {
    // FIXME: validate handle
    auto& f = this->open_files[handle];
    f.read((char*) output.data(), output.size());

    return output.subspan(0, f.gcount());
}

void Filesystem::seek_file(int handle, int offset, int c_mode) {
    // FIXME: validate handle
    auto& f = this->open_files[handle];

    switch (c_mode) {
        case SEEK_SET:
            f.seekg(offset, std::ios_base::beg);
            f.seekp(offset, std::ios_base::beg);
            break;
        case SEEK_CUR:
            f.seekg(offset, std::ios_base::cur);
            f.seekp(offset, std::ios_base::cur);
            break;
        case SEEK_END:
            f.seekg(offset, std::ios_base::end);
            f.seekp(offset, std::ios_base::end);
            break;
    }
}

int Filesystem::tell_file(int handle) {
    // FIXME: validate handle
    auto& f = this->open_files[handle];

    return f.tellg();
}

gsl::span<std::byte const> Filesystem::write_file(int handle, gsl::span<std::byte const> input) {
    // FIXME: validate handle
    auto& f = this->open_files[handle];
    f.write((char*) input.data(), input.size());

    return input.subspan(0, input.size());
}
