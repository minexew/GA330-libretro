#include "ccdl.hpp"

#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <rom>\n";
        return -1;
    }

    load_rom(argv[1]);
    return 0;
}

int get_keypad_state() {
    return 0;
}
