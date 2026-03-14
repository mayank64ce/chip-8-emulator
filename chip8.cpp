#include "chip8.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <cstdlib>

// CHIP-8 fontset: each character is  bytes
// These are sprites for hexadecimal digits 0-F
unsigned char chip8_fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80   // F
};

Chip8::Chip8(){
    initialize();
}

Chip8::~Chip8(){
    // Nothing to clean up
}

void Chip8::initialize(){
    // Initialize registers and memory
    pc = 0x200;
    opcode = 0;
    I = 0;
    sp = 0;

    // Clear display
    memset(gfx, 0, sizeof(gfx));

    // Clear stack
    memset(stack, 0, sizeof(stack));

    // Clear registers
    memset(V, 0, sizeof(V));

    // Clear memory
    memset(memory, 0, sizeof(memory));

    // Load fontset into memory (at address 0x00 to 0x50)
    for(int i=0;i<80;i++){
        memory[i] = chip8_fontset[i];
    }

    // Reset timers;
    delay_timer = 0;
    sound_timer = 0;

    // Reset keys
    memset(key, 0, sizeof(key));

    // Seed RNG
    srand(time(NULL));

    drawFlag = true;

    std::cout << "CHIP-8 initialized!" << std::endl;
}

bool Chip8::loadROM(const char* filename){
    std::cout << "Loading ROM: " << filename << std::endl;

    // Open file
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if(!file.is_open()){
        std::cerr << "Failed to open ROM file: " << filename << std::endl;
        return false;
    }

    // Get file size
    std::streampos size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::cout << "ROM size: " << size << " bytes" << std::endl;

    // CHIP-8 memory is 4k, program space is 0x200 to 0xFFF (3584 bytes)
    if(size > (4096-512)){
        std::cerr << "ROM too large! Maximum is " << (4096 - 512) << " bytes" << std::endl;
        return false;
    }

    char* buffer = new char[size];
    file.read(buffer, size);
    file.close();

    // Load ROM into memory starting at 0x200

    for(int i=0;i<size;i++){
        memory[0x200 + i] = (unsigned char)buffer[i];
    }

    delete[] buffer;

    std::cout << "ROM loaded successfully!" << std::endl;
    return true;
}