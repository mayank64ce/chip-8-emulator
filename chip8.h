#ifndef CHIP8_H
#define CHIP8_H

class Chip8{
    public:
        Chip8(); // Constructor
        ~Chip8(); // Destructor
    
        bool loadROM(const char* filename); // Load a ROM file
        void emulateCycle(); // Execute one CPU cycle

        // Public so we can access them for drawing
        unsigned char gfx[64 * 32]; // Graphics buffer
        unsigned char key[16]; // Key states
        bool drawFlag; // Set when screen needs redraw
    
    private:
        // Memory and registers
        unsigned char memory[4096]; // 4K memory
        unsigned char V[16]; // 16 registers V0-VF
        unsigned short I; // Index register
        unsigned short pc; // Program counter
        unsigned short sp; // Stack pointer
        unsigned short stack[16]; // Stack

        // Timers
        unsigned char delay_timer;
        unsigned char sound_timer;

        // Current opcode
        unsigned short opcode;

        void initialize(); // Initialize/reset the system

};

#endif