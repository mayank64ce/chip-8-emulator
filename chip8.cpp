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

void Chip8::emulateCycle() {
    // Fetch opcode
    opcode = memory[pc] << 8 | memory[pc + 1];

    // Debug: print current opcode
    // std::cout << "PC: 0x" << std::hex << pc << " Opcode: 0x" << opcode << std::dec << std::endl;

    // Decode and execute
    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode & 0x00FF) {
                case 0x00E0: { // 0x00E0: Clear screen

                    // std::cout << "Clear screen" << std::endl;
                    memset(gfx, 0, sizeof(gfx));
                    drawFlag = true;
                    pc += 2;
                    break;

                }

                case 0x00EE: { // 0x00EE: resturns from subroutine

                    sp--; // Decrement stack pointer
                    pc = stack[sp]; // Get return address from stack

                    pc += 2;
                    break;
                }

                default:
                    // std::cout << "Unknown 0x000 opcode: 0x" << std::hex << opcode << std::dec << std::endl;
                    pc += 2;
                    break;
            }
            break;
        
        case 0x1000: { // 0x1NNN : 	Jumps to address NNN.

            unsigned short nnn = (opcode & 0x0FFF);

            // std::cout << "goto 0x" << std::hex << nnn << std::dec << std::endl;

            pc = nnn;

            break;

        }

        case 0x2000: {
            // 0x2NNN: call subroutine at nnn
            unsigned short nnn = (opcode & 0x0FFF);
            stack[sp] = pc; // Store current address on stack
            sp++;           // Increment stack pointer
            pc = nnn;       // Jump to subroutine
            break;
        }

        case 0x3000: {
            // 0x3XNN: skip next instruction if VX == NN
            unsigned char x = (opcode & 0x0F00) >> 8;
            unsigned short nn = (opcode & 0x0FF);

            if(V[x] == nn) {
                pc += 4;
            } else {
                pc += 2;
            }

            break;
        }

        case 0x4000: {
            // 0x4XNN: skip next instruction if VX != NN
            unsigned char x = (opcode & 0x0F00) >> 8;
            unsigned short nn = (opcode & 0x0FF);

            if(V[x] != nn) {
                pc += 4;
            } else {
                pc += 2;
            }

            break;
        }

        case 0x5000: {
            // 0x5XY0: Skip next instruction if VX == VY
            unsigned char x = (opcode & 0x0F00) >> 8;
            unsigned char y = (opcode & 0x00F0) >> 4;

            if(V[x] == V[y]){
                pc += 4;
            }else{
                pc += 2;
            }

            break;
        }
        
        case 0x6000: {// 0x6XNN: Set VX to NN
            // We'll implement this next
            unsigned char x = (opcode & 0x0F00) >> 12;
            unsigned char nn = (opcode & 0x0FF);

            // std::cout << "Set V[" << (int)x << "] to " << (int)nn << std::endl;
            V[x] = nn;
            pc += 2; // Move to next instruction (opcodes are 2 bytes)
            break;
        }
        
        case 0x7000: {// 0x7XNN: increment VX by NN
            unsigned char x = (opcode & 0x0F00) >> 8;
            unsigned char nn = (opcode & 0x0FF);
            // std::cout << "Increment V[" << (int)x << "] by " << (int)nn << std::endl;

            V[x] += nn;
            pc += 2;
            break;
        }

        case 0x8000: {
        unsigned char x = (opcode & 0x0F00) >> 8;
        unsigned char y = (opcode & 0x00F0) >> 4;

        switch (opcode & 0x000F) {
            case 0x0000:  // 0x8XY0: VX = VY
                V[x] = V[y];
                pc += 2;
                break;

            case 0x0001:  // 0x8XY1: VX |= VY
                V[x] |= V[y];
                pc += 2;
                break;

            case 0x0002:  // 0x8XY2: VX &= VY
                V[x] &= V[y];
                pc += 2;
                break;

            case 0x0003:  // 0x8XY3: VX ^= VY
                V[x] ^= V[y];
                pc += 2;
                break;

            case 0x0004:  // 0x8XY4: VX += VY, VF = carry
                if (V[y] > (0xFF - V[x])) {
                    V[0xF] = 1;  // Carry
                } else {
                    V[0xF] = 0;
                }
                V[x] += V[y];
                pc += 2;
                break;

            case 0x0005:  // 0x8XY5: VX -= VY, VF = NOT borrow
                if (V[y] > V[x]) {
                    V[0xF] = 0;  // Borrow
                } else {
                    V[0xF] = 1;  // No borrow
                }
                V[x] -= V[y];
                pc += 2;
                break;

            case 0x0006:  // 0x8XY6: VX >>= 1, VF = shifted out bit
                V[0xF] = V[x] & 0x1;  // Save LSB
                V[x] >>= 1;
                pc += 2;
                break;

            case 0x0007:  // 0x8XY7: VX = VY - VX, VF = NOT borrow
                if (V[x] > V[y]) {
                    V[0xF] = 0;  // Borrow
                } else {
                    V[0xF] = 1;
                }
                V[x] = V[y] - V[x];
                pc += 2;
                break;

            case 0x000E:  // 0x8XYE: VX <<= 1, VF = shifted out bit
                V[0xF] = V[x] >> 7;  // Save MSB
                V[x] <<= 1;
                pc += 2;
                break;

            default:
                // std::cout << "Unknown 0x8000 opcode: 0x" << std::hex << opcode << std::dec << std::endl;
                pc += 2;
                break;
        }
        break;
    }

        case 0x9000: {
            // 0x9XY0: Skip next instruction if VX != VY
            unsigned char x = (opcode & 0x0F00) >> 8;
            unsigned char y = (opcode & 0x00F0) >> 4;

            if(V[x] != V[y]){
                pc += 4;
            }else{
                pc += 2;
            }

            break;
        }



        case 0xA000: { // 0xANNN: Sets I to the address NNN

            unsigned short nnn = (opcode & 0x0FFF);

            // std::cout << "Set I to 0x" << std::hex << nnn << std::dec << std::endl;

            I = nnn;
            pc += 2;

            break;
        }

        case 0xB000: {  // 0xBNNN: Jump to address NNN + V0
            unsigned short nnn = (opcode & 0x0FFF);
            pc = nnn + V[0];
            break;
        }

        case 0xC000: {  // 0xCXNN: VX = random byte AND NN
            unsigned char x = (opcode & 0x0F00) >> 8;
            unsigned char nn = (opcode & 0x00FF);
            V[x] = (rand() % 256) & nn;
            pc += 2;
            break;
        }

        case 0xD000: { // 0xDXYN: Draw sprite
            unsigned char x = (opcode & 0x0F00) >> 8;
            unsigned char y = (opcode & 0x00F0) >> 4;
            unsigned char height = (opcode & 0x00F);

            unsigned char xPos = V[x];
            unsigned char yPos = V[y];

            // std::cout << "Draw a sprite at (" << (int)xPos << ", " << (int)yPos << ") height " << (int)height << std::endl;
            
            V[0xF] = 0; // Reset the collision flag

            // Loop through each row of the sprite

            for(int row = 0;row < height; row++){
                unsigned char spriteData = memory[I + row];

                // Loop through each bit (pixel) in this row
                for(int col=0;col<8;col++){
                    // Check if this pixel is set (reading from left to right)
                    if((spriteData & (0x80 >> col)) != 0){
                        // Calculate screen pixel position
                        int pixelX = (xPos + col) % 64; // Wrap around screen
                        int pixelY = (yPos + row) % 32;

                        int pixelIndex = pixelX + (pixelY * 64);

                        // Check for collision
                        if(gfx[pixelIndex] == 1){
                            V[0xF] = 1; // Collision detected
                        }

                        // XOR the pixel
                        gfx[pixelIndex] ^= 1;

                    }
                }
            }

            drawFlag = true;
            pc += 2;
            break;
        }

        case 0xE000: {
            switch(opcode & 0x00FF){
                case 0x009E: {
                    // 0xEX9E: Skip next instruction if key VX is pressed
                    unsigned char x = (opcode & 0x0F00) >> 8;
                    if(key[V[x]] != 0){
                        pc += 4; // skip next instruction
                    }else {
                        pc += 2;
                    }

                    break;
                }

                case 0x00A1: {
                    // 0xEXA1: skip next instruction if key VX is NOT pressed
                    unsigned char x = (opcode & 0x0F00) >> 8;
                    if(key[V[x]] == 0){
                        pc += 4; // skip next instruction
                    }else {
                        pc += 2;
                    }

                    break;
                }

                default:
                    // std::cout << "Unknown 0xE000 opcode: 0x" << std::hex << opcode << std::dec << std::endl;
                    pc += 2;
                    break;
            }
        }

        case 0xF000: {
            unsigned char x = (opcode & 0x0F00) >> 8;

            switch (opcode & 0x00FF) {
                case 0x0007:  // 0xFX07: VX = delay_timer
                    V[x] = delay_timer;
                    pc += 2;
                    break;

                case 0x000A: {  // 0xFX0A: Wait for key press, store in VX
                    bool keyPressed = false;
                    for (int i = 0; i < 16; i++) {
                        if (key[i] != 0) {
                            V[x] = i;
                            keyPressed = true;
                            break;
                        }
                    }
                    if (!keyPressed) {
                        return;  // Don't increment PC, repeat this instruction
                    }
                    pc += 2;
                    break;
                }

                case 0x0015:  // 0xFX15: delay_timer = VX
                    delay_timer = V[x];
                    pc += 2;
                    break;

                case 0x0018:  // 0xFX18: sound_timer = VX
                    sound_timer = V[x];
                    pc += 2;
                    break;

                case 0x001E:  // 0xFX1E: I += VX
                    // Note: VF is set to 1 if range overflow (I+VX>0xFFF), 0 otherwise
                    // This is undocumented behavior, but some games use it
                    if (I + V[x] > 0xFFF) {
                        V[0xF] = 1;
                    } else {
                        V[0xF] = 0;
                    }
                    I += V[x];
                    pc += 2;
                    break;

                case 0x0029:  // 0xFX29: I = location of sprite for digit VX
                    // Font sprites are 5 bytes each, starting at memory[0]
                    I = V[x] * 5;
                    pc += 2;
                    break;

                case 0x0033: {  // 0xFX33: Store BCD representation of VX at I, I+1, I+2
                    // BCD = Binary-Coded Decimal
                    // If VX = 156, then:
                    // memory[I]   = 1 (hundreds)
                    // memory[I+1] = 5 (tens)
                    // memory[I+2] = 6 (ones)
                    memory[I]     = V[x] / 100;
                    memory[I + 1] = (V[x] / 10) % 10;
                    memory[I + 2] = V[x] % 10;
                    pc += 2;
                    break;
                }

                case 0x0055:  // 0xFX55: Store V0 through VX in memory starting at I
                    for (int i = 0; i <= x; i++) {
                        memory[I + i] = V[i];
                    }
                    // Original CHIP-8: I is incremented
                    I += x + 1;
                    pc += 2;
                    break;

                case 0x0065:  // 0xFX65: Fill V0 through VX from memory starting at I
                    for (int i = 0; i <= x; i++) {
                        V[i] = memory[I + i];
                    }
                    // Original CHIP-8: I is incremented
                    I += x + 1;
                    pc += 2;
                    break;

                default:
                    // std::cout << "Unknown 0xF000 opcode: 0x" << std::hex << opcode << std::dec << std::endl;
                    pc += 2;
                    break;
            }
            break;
        }

        default:
            // std::cout << "Unknown opcode: 0x" << std::hex << opcode << std::dec << std::endl;
            break;
 
    }

    // Update timers
    if(delay_timer > 0) {
        delay_timer--;
    }

    if(sound_timer > 0){
        if(sound_timer == 1) {
            std::cout << "BEEP!" << std::endl;
        }
        sound_timer--;
    }
}