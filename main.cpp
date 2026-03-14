#include <iostream>
#include "chip8.h"

int main(int argc, char** argv){
    if(argc != 2){
        std::cout << "Usage: " << argv[0] << " <ROM file>" << std::endl;
        return 1;
    }

    Chip8 chip8;

    if(!chip8.loadROM(argv[1])){
        return 1;
    }

    std::cout << "ROM loaded! Press Enter to exit..." << std::endl;
    std::cin.get();

    return 0;
}