# Building a CHIP-8 Emulator from Scratch: A Complete Guide

> **Based on the tutorial by Laurence Muller**
> Original guide: [How to write an emulator (CHIP-8 interpreter)](http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/)
> This is an expanded educational companion guide with detailed explanations of foundational concepts.

## About This Guide

This guide is based on Laurence Muller's excellent CHIP-8 emulator tutorial. While the original guide is comprehensive, it assumes familiarity with concepts like binary/hexadecimal, memory addressing, and bitwise operations.

**This expanded version:**
- ✅ Explains foundational concepts from first principles (binary, hexadecimal, memory, file I/O)
- ✅ Uses SDL2 instead of GLUT/OpenGL (more modern, easier to set up on macOS)
- ✅ Takes an incremental approach with testable checkpoints at each step
- ✅ Provides very detailed explanations of *why* things work, not just *how*
- ✅ Includes Mac M2-specific setup instructions

**If you're comfortable with low-level programming concepts**, you may prefer the [original guide](http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/). If you want to understand everything from the ground up, continue here!

## Table of Contents
1. [Introduction](#introduction)
2. [What You'll Learn](#what-youll-learn)
3. [Prerequisites & Setup](#prerequisites--setup)
4. [Foundation Concepts](#foundation-concepts)
5. [CHIP-8 Architecture](#chip-8-architecture)
6. [Step-by-Step Implementation](#step-by-step-implementation)
7. [Testing & Debugging](#testing--debugging)
8. [Resources](#resources)

---

## Introduction

### What is Emulation?

**Emulation** is the process of making one computer system (the "host") behave like another computer system (the "guest"). When you build an emulator, you're essentially creating software that mimics the hardware of another computer.

Think of it like this:
- A piano can make piano sounds (that's what it's built to do)
- A synthesizer can *emulate* a piano by making the same sounds digitally
- Your computer is the synthesizer, and CHIP-8 is the piano you're emulating

### Emulator vs. Simulator

An important distinction (from the original Muller guide):

**Emulator**: Attempts to recreate the behavior of the hardware exactly. You implement each CPU instruction, each hardware register, and the emulator behaves as if it *is* that hardware.

**Simulator**: Models the behavior at a higher level. For example, a Pong simulator might just recreate the game logic (paddles, ball physics, scoring) without caring about how the original hardware worked.

We're building an **emulator** - we'll implement CHIP-8's CPU, memory, registers, and instruction set exactly as specified. This teaches you how actual hardware works!

### Why CHIP-8?

CHIP-8 is perfect for learning emulation because:

1. **It's simple**: Only 35 instructions to implement (compare to thousands in modern CPUs)
2. **It's well-documented**: Created in the 1970s, everything about it is known
3. **Fast results**: You can have games running in a weekend
4. **It teaches real concepts**: CPU cycles, memory, opcodes, registers - all real emulation concepts
5. **It's not actually hardware**: CHIP-8 was originally an interpreted language, so it's more forgiving

### What is CHIP-8?

CHIP-8 is an interpreted programming language developed in the mid-1970s by Joseph Weisbecker. It was originally used on the COSMAC VIP and Telmac 1800 8-bit microcomputers to make game programming easier.

Instead of programming directly in machine code, developers could write programs in CHIP-8, which was simpler. Your emulator will run these CHIP-8 programs.

---

## What You'll Learn

By building this emulator, you'll learn:

- **How CPUs work**: Fetch-decode-execute cycle, program counter, opcodes
- **Memory management**: How programs are stored and accessed
- **Binary and hexadecimal**: Reading and working with different number systems
- **Bitwise operations**: Practical use of AND, OR, XOR, shifts
- **Graphics rendering**: Drawing pixels to screen
- **Input handling**: Keyboard input in real-time
- **Timing and synchronization**: Making your emulator run at the right speed
- **File I/O**: Loading binary files into memory

---

## Prerequisites & Setup

### What You Need to Know

- Basic C++ (variables, functions, loops, if statements)
- Arrays and basic data structures
- Bitwise operations (you mentioned you know these - we'll use them heavily!)

### Development Environment Setup (M2 MacBook Pro)

#### 1. Install Xcode Command Line Tools

Open Terminal and run:
```bash
xcode-select --install
```

This gives you the C++ compiler (clang).

#### 2. Install Homebrew (if you don't have it)

```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

#### 3. Install SDL2

SDL2 (Simple DirectMedia Layer 2) is a library that makes graphics, sound, and input much easier than raw OpenGL.

```bash
brew install sdl2
```

#### 4. Create Your Project Directory

```bash
mkdir ~/chip8-emulator
cd ~/chip8-emulator
```

#### 5. Test Your Setup

Create a file called `test.cpp`:

```cpp
#include <SDL2/SDL.h>
#include <iostream>

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    std::cout << "SDL initialized successfully!" << std::endl;
    SDL_Quit();
    return 0;
}
```

Compile and run:
```bash
g++ -std=c++17 test.cpp -o test -I/opt/homebrew/include -L/opt/homebrew/lib -lSDL2
./test
```

You should see: `SDL initialized successfully!`

If it works, you're ready! If not, make sure SDL2 installed correctly with `brew list sdl2`.

---

## Foundation Concepts

Before we build the emulator, let's understand the fundamental concepts you'll need.

### Binary and Hexadecimal

Computers work in **binary** (base 2) - only 0s and 1s. But binary is hard for humans to read:
- `11111111` in binary = 255 in decimal

**Hexadecimal** (base 16) is a shorthand that's easier to read:
- Uses digits 0-9 and letters A-F
- `0xFF` in hexadecimal = `11111111` in binary = 255 in decimal

#### Why Hexadecimal?

Each hex digit represents exactly 4 binary bits:

```
Hex    Binary    Decimal
0x0    0000      0
0x1    0001      1
0x2    0010      2
...
0x9    1001      9
0xA    1010      10
0xB    1011      11
0xC    1100      12
0xD    1101      13
0xE    1110      14
0xF    1111      15
```

Two hex digits = one byte (8 bits):
- `0xFF` = `11111111` = 255
- `0x00` = `00000000` = 0
- `0xA5` = `10100101` = 165

In C++, you write hex numbers with the `0x` prefix: `0xFF`, `0x200`, `0xA000`

#### Quick Practice

```
0x0F   = 00001111 = 15
0xF0   = 11110000 = 240
0xFF   = 11111111 = 255
0x100  = 0001 0000 0000 = 256
0xFFF  = 1111 1111 1111 = 4095
```

**Why this matters**: CHIP-8 opcodes are 2 bytes in hex, like `0x6A05` or `0xD125`. You'll be working with hex constantly.

### Memory - How Computers Store Data

Think of memory as a huge array of bytes, where each byte has an **address**.

```
Address    Value (what's stored there)
0x000      0x12
0x001      0x34
0x002      0x56
0x003      0xA0
...
```

CHIP-8 has **4096 bytes** of memory (addresses `0x000` to `0xFFF`).

#### Why 4096?

`0xFFF` in hex = `4095` in decimal, so addresses go from 0 to 4095 = 4096 total addresses.

In C++, we'll represent this as:
```cpp
unsigned char memory[4096];  // Array of 4096 bytes
```

- `unsigned char` = one byte (0 to 255)
- `memory[0x200]` accesses the byte at address 0x200
- Each array index is a memory address

#### Pointers (Brief Refresher)

A **pointer** is just a memory address. When you read a file into memory, you're putting bytes at specific addresses. When the CPU runs, it reads instructions from memory addresses.

### File I/O - Loading ROMs

CHIP-8 programs are stored as binary files (`.c8` extension). You need to:
1. Open the file
2. Read all bytes
3. Store them in memory starting at address `0x200`

**Why 0x200?** The first 512 bytes (`0x000` to `0x1FF`) are reserved for the CHIP-8 interpreter itself (that's your emulator). Programs start at `0x200`.

Here's how to read a binary file in C++:

```cpp
#include <fstream>
#include <iostream>

// Open file in binary mode
std::ifstream file("game.c8", std::ios::binary);

if (!file.is_open()) {
    std::cout << "Failed to open file!" << std::endl;
    return false;
}

// Get file size
file.seekg(0, std::ios::end);
long fileSize = file.tellg();
file.seekg(0, std::ios::beg);

// Read into buffer
char* buffer = new char[fileSize];
file.read(buffer, fileSize);
file.close();

// Copy to memory starting at 0x200
for (int i = 0; i < fileSize; i++) {
    memory[0x200 + i] = buffer[i];
}

delete[] buffer;
```

**What's happening:**
- `std::ios::binary`: Read the file as raw bytes, not text
- `seekg()` and `tellg()`: Move through the file and get its size
- `read()`: Read bytes into a buffer
- Copy each byte to memory starting at address `0x200`

---

## CHIP-8 Architecture

Now let's understand what we're emulating. CHIP-8 is like a tiny computer with:

### 1. Memory: 4096 Bytes (4 KB)

```
+------------------+  0x000
|                  |
|  Interpreter     |  Font data is here
|  (reserved)      |
|                  |
+------------------+  0x200
|                  |
|  Program ROM     |  Your game code loads here
|                  |
|                  |
+------------------+  0xFFF
```

In code:
```cpp
unsigned char memory[4096];
```

### 2. Registers: Small Storage Locations

**Registers** are like variables built into the CPU - they're super fast to access.

CHIP-8 has:

- **16 general-purpose registers**: `V0` to `VF` (each 1 byte)
  - Used for calculations, storing values
  - `VF` is special: used as a flag for some operations

```cpp
unsigned char V[16];  // V[0] through V[15]
```

- **Index register (I)**: 2 bytes, stores memory addresses

```cpp
unsigned short I;  // 16-bit = 2 bytes
```

- **Program Counter (PC)**: 2 bytes, points to current instruction

```cpp
unsigned short pc;  // Starts at 0x200
```

The PC is like a bookmark saying "we're executing this instruction right now."

- **Stack Pointer (SP)**: 1 byte, points to top of stack

```cpp
unsigned char sp;
```

### 3. Stack: 16 Levels

The **stack** stores return addresses when you call functions/subroutines.

Think of it like a stack of plates:
- When you **call** a subroutine, you push the current address onto the stack
- When you **return**, you pop the address off the stack and jump back

```cpp
unsigned short stack[16];  // Can nest 16 subroutine calls deep
```

### 4. Timers: 2 Special Registers

- **Delay Timer (DT)**: Counts down at 60 Hz, used for timing events
- **Sound Timer (ST)**: Counts down at 60 Hz, beeps when non-zero

```cpp
unsigned char delay_timer;
unsigned char sound_timer;
```

Both decrement by 1 every 1/60th of a second (if they're above 0).

### 5. Display: 64x32 Pixels

The screen is monochrome (black and white):
- 64 pixels wide
- 32 pixels tall
- Each pixel is either ON or OFF

```cpp
unsigned char gfx[64 * 32];  // 2048 pixels total
```

`gfx[0]` = pixel at (0,0)
`gfx[1]` = pixel at (1,0)
`gfx[64]` = pixel at (0,1) (start of second row)

### 6. Input: 16-Key Keypad

CHIP-8 has a hexadecimal keypad (0-F):

```
Original layout:
1 2 3 C
4 5 6 D
7 8 9 E
A 0 B F

We'll map to keyboard:
1 2 3 4
Q W E R
A S D F
Z X C V
```

```cpp
unsigned char key[16];  // key[0x0] through key[0xF]
```

`key[0x5] = 1` means the '5' key is currently pressed.

### 7. Opcodes: The Instructions

An **opcode** is an instruction that tells the CPU what to do. CHIP-8 opcodes are 2 bytes (16 bits).

Example: `0x6A05`
- **6**: Instruction type (set register)
- **A**: Register number (V[A])
- **05**: Value to set (5)
- Meaning: "Set register VA to 5"

We'll decode these using bitwise operations:
```cpp
unsigned short opcode = 0x6A05;

unsigned char x = (opcode & 0x0F00) >> 8;  // x = 0xA
unsigned char value = (opcode & 0x00FF);    // value = 0x05
```

**What's happening:**
- `0x0F00`: Binary mask `0000 1111 0000 0000` - keeps only the 'A'
- `>> 8`: Shift right 8 bits to get the value
- `0x00FF`: Binary mask `0000 0000 1111 1111` - keeps only the '05'

### The Complete CHIP-8 Specification

Here are all 35 opcodes (don't worry, we'll implement them step by step):

| Opcode | Description |
|--------|-------------|
| `0x00E0` | Clear the screen |
| `0x00EE` | Return from subroutine |
| `0x1NNN` | Jump to address NNN |
| `0x2NNN` | Call subroutine at NNN |
| `0x3XNN` | Skip next instruction if VX == NN |
| `0x4XNN` | Skip next instruction if VX != NN |
| `0x5XY0` | Skip next instruction if VX == VY |
| `0x6XNN` | Set VX to NN |
| `0x7XNN` | Add NN to VX |
| `0x8XY0` | Set VX to VY |
| `0x8XY1` | Set VX to VX OR VY |
| `0x8XY2` | Set VX to VX AND VY |
| `0x8XY3` | Set VX to VX XOR VY |
| `0x8XY4` | Add VY to VX, VF = carry |
| `0x8XY5` | Subtract VY from VX, VF = NOT borrow |
| `0x8XY6` | Shift VX right by 1, VF = shifted bit |
| `0x8XY7` | Set VX to VY - VX, VF = NOT borrow |
| `0x8XYE` | Shift VX left by 1, VF = shifted bit |
| `0x9XY0` | Skip next instruction if VX != VY |
| `0xANNN` | Set I to address NNN |
| `0xBNNN` | Jump to address NNN + V0 |
| `0xCXNN` | Set VX to random byte AND NN |
| `0xDXYN` | Draw sprite at (VX, VY), height N |
| `0xEX9E` | Skip next instruction if key VX is pressed |
| `0xEXA1` | Skip next instruction if key VX is not pressed |
| `0xFX07` | Set VX to delay timer |
| `0xFX0A` | Wait for key press, store in VX |
| `0xFX15` | Set delay timer to VX |
| `0xFX18` | Set sound timer to VX |
| `0xFX1E` | Add VX to I |
| `0xFX29` | Set I to location of sprite for digit VX |
| `0xFX33` | Store BCD representation of VX at I, I+1, I+2 |
| `0xFX55` | Store V0 through VX in memory starting at I |
| `0xFX65` | Fill V0 through VX from memory starting at I |

**Don't panic!** Many of these are similar, and we'll build them incrementally.

---

## Step-by-Step Implementation

Now let's build this thing! We'll go step by step, testing as we go.

### Step 0: Project Structure

Create these files:

```
chip8-emulator/
├── chip8.h        # Class declaration
├── chip8.cpp      # Emulator implementation
├── main.cpp       # SDL2 main loop
└── Makefile       # Build script
```

#### chip8.h

```cpp
#ifndef CHIP8_H
#define CHIP8_H

class Chip8 {
public:
    Chip8();   // Constructor
    ~Chip8();  // Destructor

    bool loadROM(const char* filename);  // Load a ROM file
    void emulateCycle();                 // Execute one CPU cycle

    // Public so we can access them for drawing
    unsigned char gfx[64 * 32];  // Graphics buffer
    unsigned char key[16];        // Key states
    bool drawFlag;                // Set when screen needs redraw

private:
    // Memory and registers
    unsigned char memory[4096];   // 4K memory
    unsigned char V[16];          // 16 registers V0-VF
    unsigned short I;             // Index register
    unsigned short pc;            // Program counter
    unsigned short sp;            // Stack pointer
    unsigned short stack[16];     // Stack

    // Timers
    unsigned char delay_timer;
    unsigned char sound_timer;

    // Current opcode
    unsigned short opcode;

    void initialize();  // Initialize/reset the system
};

#endif
```

**What we're doing**: Creating a class to hold all CHIP-8 state. This keeps everything organized.

#### Create Makefile

```makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall -I/opt/homebrew/include
LDFLAGS = -L/opt/homebrew/lib -lSDL2

TARGET = chip8
SOURCES = main.cpp chip8.cpp
OBJECTS = $(SOURCES:.cpp=.o)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: clean
```

Now you can build with just `make` and clean with `make clean`.

---

### Step 1: Initialize the System

Let's implement the constructor and initialization.

#### chip8.cpp (Part 1)

```cpp
#include "chip8.h"
#include <iostream>
#include <fstream>
#include <cstring>   // for memset
#include <ctime>     // for rand seed
#include <cstdlib>   // for rand

// CHIP-8 fontset: each character is 5 bytes
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

Chip8::Chip8() {
    initialize();
}

Chip8::~Chip8() {
    // Nothing to clean up
}

void Chip8::initialize() {
    // Initialize registers and memory
    pc = 0x200;  // Program counter starts at 0x200
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

    // Load fontset into memory (at addresses 0x00 to 0x50)
    for (int i = 0; i < 80; i++) {
        memory[i] = chip8_fontset[i];
    }

    // Reset timers
    delay_timer = 0;
    sound_timer = 0;

    // Reset keys
    memset(key, 0, sizeof(key));

    // Seed random number generator
    srand(time(NULL));

    drawFlag = true;

    std::cout << "CHIP-8 initialized" << std::endl;
}
```

**What's happening:**

1. **PC starts at 0x200**: That's where programs load
2. **memset()**: Fast way to set all bytes in an array to 0
3. **Fontset**: Built-in sprites for digits 0-F, stored at start of memory
   - Each digit is 5 bytes (4 pixels wide, 5 pixels tall)
   - When a program wants to draw "5", it looks up the sprite in memory
4. **Random seed**: For the random number opcode later

#### Why the fontset?

Look at the sprite for "0":
```
0xF0 = 11110000 = ****
0x90 = 10010000 = *  *
0x90 = 10010000 = *  *
0x90 = 10010000 = *  *
0xF0 = 11110000 = ****
```

Each 1 bit is a pixel. This draws the digit "0" on screen!

---

### Step 2: Load a ROM

Now let's load a CHIP-8 program into memory.

#### chip8.cpp (Add to file)

```cpp
bool Chip8::loadROM(const char* filename) {
    std::cout << "Loading ROM: " << filename << std::endl;

    // Open file
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        std::cerr << "Failed to open ROM file: " << filename << std::endl;
        return false;
    }

    // Get file size
    std::streampos size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::cout << "ROM size: " << size << " bytes" << std::endl;

    // CHIP-8 memory is 4KB, program space is 0x200 to 0xFFF (3584 bytes)
    if (size > (4096 - 512)) {
        std::cerr << "ROM too large! Maximum is " << (4096 - 512) << " bytes" << std::endl;
        return false;
    }

    // Read file into buffer
    char* buffer = new char[size];
    file.read(buffer, size);
    file.close();

    // Load ROM into memory starting at 0x200
    for (int i = 0; i < size; i++) {
        memory[0x200 + i] = (unsigned char)buffer[i];
    }

    delete[] buffer;

    std::cout << "ROM loaded successfully!" << std::endl;
    return true;
}
```

**What's happening:**

1. Open file in binary mode
2. `std::ios::ate`: Start at end of file (to get size easily)
3. `tellg()`: Get current position (= file size)
4. `seekg(0, std::ios::beg)`: Go back to beginning
5. Read all bytes into buffer
6. Copy buffer to memory starting at 0x200
7. Clean up buffer

**Testing checkpoint**: Let's create a simple main.cpp to test this.

#### main.cpp (Simple test version)

```cpp
#include <iostream>
#include "chip8.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <ROM file>" << std::endl;
        return 1;
    }

    Chip8 chip8;

    if (!chip8.loadROM(argv[1])) {
        return 1;
    }

    std::cout << "ROM loaded! Press Enter to exit..." << std::endl;
    std::cin.get();

    return 0;
}
```

**Build and test:**

```bash
make
./chip8 pong2.c8
```

You should see:
```
CHIP-8 initialized
Loading ROM: pong2.c8
ROM size: 294 bytes
ROM loaded successfully!
ROM loaded! Press Enter to exit...
```

**Checkpoint passed!** You've successfully loaded a ROM into memory. ✅

---

### Step 3: The Fetch-Decode-Execute Cycle

This is the heart of the emulator. The CPU continuously:
1. **Fetch**: Read the next instruction from memory
2. **Decode**: Figure out what the instruction means
3. **Execute**: Do what the instruction says
4. **Repeat**

#### Understanding Opcodes

CHIP-8 instructions are 2 bytes. They're stored in memory as separate bytes but we need to combine them.

Example: Memory has `0x61` at address 0x200 and `0x05` at address 0x201.

The full opcode is `0x6105` which means "Set V1 to 0x05".

**How to fetch:**
```cpp
opcode = memory[pc] << 8 | memory[pc + 1];
```

**What's happening:**
- `memory[pc]`: Get first byte (0x61)
- `<< 8`: Shift left 8 bits: `0x61` becomes `0x6100`
- `memory[pc + 1]`: Get second byte (0x05)
- `|`: OR them together: `0x6100 | 0x0005 = 0x6105`

Visual:
```
memory[pc]     = 0x61 = 0110 0001
Shift left 8   =        0110 0001 0000 0000 = 0x6100
memory[pc + 1] = 0x05 = 0000 0101
OR together    =        0110 0001 0000 0101 = 0x6105
```

#### Decoding Opcodes

To figure out what an opcode means, we look at different parts of it.

For opcode `0x6A05`:

```cpp
unsigned char first_nibble = (opcode & 0xF000) >> 12;  // 0x6
unsigned char x            = (opcode & 0x0F00) >> 8;   // 0xA
unsigned char y            = (opcode & 0x00F0) >> 4;   // 0x0
unsigned char n            = (opcode & 0x000F);        // 0x5
unsigned char nn           = (opcode & 0x00FF);        // 0x05
unsigned short nnn         = (opcode & 0x0FFF);        // 0xA05
```

**Masks explained:**
- `0xF000`: Keeps only first nibble (4 bits)
- `0x0F00`: Keeps only second nibble
- `0x00F0`: Keeps only third nibble
- `0x000F`: Keeps only fourth nibble
- `0x00FF`: Keeps last byte (8 bits)
- `0x0FFF`: Keeps last 3 nibbles (12 bits)

We use a **switch statement** on the first nibble to decide which instruction family it is.

#### chip8.cpp (Add emulateCycle - skeleton first)

```cpp
void Chip8::emulateCycle() {
    // Fetch opcode
    opcode = memory[pc] << 8 | memory[pc + 1];

    // Debug: print current opcode
    std::cout << "PC: 0x" << std::hex << pc << " Opcode: 0x" << opcode << std::dec << std::endl;

    // Decode and execute
    switch (opcode & 0xF000) {
        case 0x0000:
            // Multiple opcodes start with 0x0
            // We'll implement these next
            break;

        case 0x1000:  // 0x1NNN: Jump to address NNN
            // We'll implement this next
            break;

        case 0x6000:  // 0x6XNN: Set VX to NN
            // We'll implement this next
            break;

        // ... more cases ...

        default:
            std::cout << "Unknown opcode: 0x" << std::hex << opcode << std::dec << std::endl;
            break;
    }

    // Update timers
    if (delay_timer > 0) {
        delay_timer--;
    }

    if (sound_timer > 0) {
        if (sound_timer == 1) {
            std::cout << "BEEP!" << std::endl;
        }
        sound_timer--;
    }
}
```

Now let's implement our first few opcodes!

---

### Step 4: Implementing Your First Opcodes

Let's start with simple opcodes to build confidence.

#### Opcode 0x6XNN - Set VX to NN

This sets register VX to the value NN.

Example: `0x6A05` means "Set V[A] to 0x05"

```cpp
case 0x6000: {  // 0x6XNN: Set VX to NN
    unsigned char x = (opcode & 0x0F00) >> 8;
    unsigned char nn = (opcode & 0x00FF);

    std::cout << "Set V[" << (int)x << "] to " << (int)nn << std::endl;

    V[x] = nn;
    pc += 2;  // Move to next instruction (opcodes are 2 bytes)
    break;
}
```

**Why `pc += 2`?** Each instruction is 2 bytes, so we move the program counter forward by 2 to get to the next instruction.

#### Opcode 0x7XNN - Add NN to VX

Add a value to a register.

Example: `0x7A05` means "Add 0x05 to V[A]"

```cpp
case 0x7000: {  // 0x7XNN: Add NN to VX
    unsigned char x = (opcode & 0x0F00) >> 8;
    unsigned char nn = (opcode & 0x00FF);

    std::cout << "Add " << (int)nn << " to V[" << (int)x << "]" << std::endl;

    V[x] += nn;
    pc += 2;
    break;
}
```

Note: We don't worry about overflow. If V[x] is 255 and we add 1, it wraps to 0 (because it's an unsigned char).

#### Opcode 0xANNN - Set I to address NNN

Set the index register to an address.

Example: `0xA250` means "Set I to 0x250"

```cpp
case 0xA000: {  // 0xANNN: Set I to address NNN
    unsigned short nnn = (opcode & 0x0FFF);

    std::cout << "Set I to 0x" << std::hex << nnn << std::dec << std::endl;

    I = nnn;
    pc += 2;
    break;
}
```

#### Opcode 0x1NNN - Jump to address NNN

Change the program counter to jump to a new location.

Example: `0x1250` means "Jump to address 0x250"

```cpp
case 0x1000: {  // 0x1NNN: Jump to address NNN
    unsigned short nnn = (opcode & 0x0FFF);

    std::cout << "Jump to 0x" << std::hex << nnn << std::dec << std::endl;

    pc = nnn;
    // Note: We DON'T do pc += 2 here because we're jumping!
    break;
}
```

**Important:** When jumping, we set `pc` directly. We don't add 2 because we want to jump to the exact address.

#### Opcode 0x00E0 - Clear screen

Clear the display.

```cpp
case 0x0000: {
    switch (opcode & 0x00FF) {
        case 0x00E0:  // 0x00E0: Clear screen
            std::cout << "Clear screen" << std::endl;
            memset(gfx, 0, sizeof(gfx));
            drawFlag = true;
            pc += 2;
            break;

        case 0x00EE:  // 0x00EE: Return from subroutine
            // We'll implement this when we do subroutines
            std::cout << "Return from subroutine (not implemented yet)" << std::endl;
            pc += 2;
            break;

        default:
            std::cout << "Unknown 0x0000 opcode: 0x" << std::hex << opcode << std::dec << std::endl;
            pc += 2;
            break;
    }
    break;
}
```

**Testing checkpoint:** Let's create a tiny test ROM to verify our opcodes work!

#### Create a test ROM

Create a file called `test.txt` with this hex data:
```
6105    ; Set V1 to 0x05
7102    ; Add 0x02 to V1 (V1 should now be 0x07)
A250    ; Set I to 0x250
1206    ; Jump to 0x206 (infinite loop)
```

Convert to binary (you can use an online hex-to-binary converter or this Python script):

```python
# save as make_test_rom.py
opcodes = [0x6105, 0x7102, 0xA250, 0x1206]
with open('test.c8', 'wb') as f:
    for op in opcodes:
        f.write(op.to_bytes(2, byteorder='big'))
```

Run: `python3 make_test_rom.py`

#### Update main.cpp to run a few cycles

```cpp
#include <iostream>
#include "chip8.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <ROM file>" << std::endl;
        return 1;
    }

    Chip8 chip8;

    if (!chip8.loadROM(argv[1])) {
        return 1;
    }

    std::cout << "\nRunning 5 CPU cycles...\n" << std::endl;

    for (int i = 0; i < 5; i++) {
        chip8.emulateCycle();
        std::cout << std::endl;
    }

    return 0;
}
```

**Build and run:**

```bash
make
./chip8 test.c8
```

**Expected output:**
```
CHIP-8 initialized
Loading ROM: test.c8
ROM size: 8 bytes
ROM loaded successfully!

Running 5 CPU cycles...

PC: 0x200 Opcode: 0x6105
Set V[1] to 5

PC: 0x202 Opcode: 0x7102
Add 2 to V[1]

PC: 0x204 Opcode: 0xa250
Set I to 0x250

PC: 0x206 Opcode: 0x1206
Jump to 0x206

PC: 0x206 Opcode: 0x1206
Jump to 0x206
```

**Checkpoint passed!** Your emulator is executing opcodes! ✅

Notice how after the jump (opcode 0x1206), the PC stays at 0x206, creating an infinite loop. Perfect!

---

### Step 5: Implementing the Display System

Now let's add actual graphics with SDL2!

The CHIP-8 display is 64x32 pixels. We'll scale it up so it's visible (e.g., 10x scale = 640x320 window).

#### Opcode 0xDXYN - Draw Sprite

This is the most complex opcode. It draws a sprite on screen.

**How it works:**
1. Get X coordinate from VX
2. Get Y coordinate from VY
3. Get sprite height N (in pixels)
4. Sprite data is 8 pixels wide, stored in memory starting at address I
5. Each byte is one row of the sprite
6. XOR each sprite pixel with the screen pixel
7. If any pixel gets turned off, set VF = 1 (collision detection)

Example: `0xD125` means "Draw sprite at (V[1], V[2]), height 5 pixels"

#### chip8.cpp (Add draw opcode)

```cpp
case 0xD000: {  // 0xDXYN: Draw sprite
    unsigned char x = (opcode & 0x0F00) >> 8;
    unsigned char y = (opcode & 0x00F0) >> 4;
    unsigned char height = (opcode & 0x000F);

    unsigned char xPos = V[x];
    unsigned char yPos = V[y];

    std::cout << "Draw sprite at (" << (int)xPos << ", " << (int)yPos
              << ") height " << (int)height << std::endl;

    V[0xF] = 0;  // Reset collision flag

    // Loop through each row of the sprite
    for (int row = 0; row < height; row++) {
        unsigned char spriteData = memory[I + row];

        // Loop through each bit (pixel) in this row
        for (int col = 0; col < 8; col++) {
            // Check if this pixel is set (reading from left to right)
            if ((spriteData & (0x80 >> col)) != 0) {
                // Calculate screen pixel position
                int pixelX = (xPos + col) % 64;  // Wrap around screen
                int pixelY = (yPos + row) % 32;
                int pixelIndex = pixelX + (pixelY * 64);

                // Check for collision
                if (gfx[pixelIndex] == 1) {
                    V[0xF] = 1;  // Collision detected
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
```

**Let me explain the sprite drawing in detail:**

Imagine we're drawing the sprite for "0" (which is in the fontset):
```
memory[I + 0] = 0xF0 = 11110000
memory[I + 1] = 0x90 = 10010000
memory[I + 2] = 0x90 = 10010000
memory[I + 3] = 0x90 = 10010000
memory[I + 4] = 0xF0 = 11110000
```

For row 0 (0xF0 = 11110000):
- Bit 0 (leftmost): 1 → draw pixel
- Bit 1: 1 → draw pixel
- Bit 2: 1 → draw pixel
- Bit 3: 1 → draw pixel
- Bit 4: 0 → skip
- Bit 5: 0 → skip
- Bit 6: 0 → skip
- Bit 7: 0 → skip

We check each bit with `(spriteData & (0x80 >> col))`:
- `0x80` = `10000000` (checks leftmost bit)
- `0x80 >> 1` = `01000000` (checks second bit)
- `0x80 >> 2` = `00100000` (checks third bit)
- etc.

**XOR behavior:**
- If screen pixel is 0 and sprite pixel is 1: result is 1 (turn on)
- If screen pixel is 1 and sprite pixel is 1: result is 0 (turn off) → collision!
- If screen pixel is 0 and sprite pixel is 0: result is 0 (stay off)
- If screen pixel is 1 and sprite pixel is 0: result is 1 (stay on)

This is how CHIP-8 games detect when sprites overlap!

#### Now let's add SDL2 rendering

Replace your main.cpp with this full SDL2 version:

```cpp
#include <SDL2/SDL.h>
#include <iostream>
#include "chip8.h"

const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;
const int SCALE = 10;  // Scale up the display

class Display {
public:
    Display() : window(nullptr), renderer(nullptr), texture(nullptr) {}

    bool initialize() {
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
            return false;
        }

        window = SDL_CreateWindow(
            "CHIP-8 Emulator",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            SCREEN_WIDTH * SCALE,
            SCREEN_HEIGHT * SCALE,
            SDL_WINDOW_SHOWN
        );

        if (!window) {
            std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
            return false;
        }

        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            std::cerr << "Renderer creation failed: " << SDL_GetError() << std::endl;
            return false;
        }

        texture = SDL_CreateTexture(
            renderer,
            SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_STREAMING,
            SCREEN_WIDTH,
            SCREEN_HEIGHT
        );

        if (!texture) {
            std::cerr << "Texture creation failed: " << SDL_GetError() << std::endl;
            return false;
        }

        return true;
    }

    void update(const unsigned char* gfx) {
        // Create a buffer of pixels in RGBA format
        uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];

        for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
            // If pixel is on, make it white; otherwise black
            pixels[i] = gfx[i] ? 0xFFFFFFFF : 0x00000000;
        }

        SDL_UpdateTexture(texture, nullptr, pixels, SCREEN_WIDTH * sizeof(uint32_t));
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, nullptr, nullptr);
        SDL_RenderPresent(renderer);
    }

    void cleanup() {
        if (texture) SDL_DestroyTexture(texture);
        if (renderer) SDL_DestroyRenderer(renderer);
        if (window) SDL_DestroyWindow(window);
        SDL_Quit();
    }

private:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
};

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <ROM file>" << std::endl;
        return 1;
    }

    Chip8 chip8;
    Display display;

    if (!display.initialize()) {
        return 1;
    }

    if (!chip8.loadROM(argv[1])) {
        display.cleanup();
        return 1;
    }

    bool running = true;
    SDL_Event event;

    // Main emulation loop
    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }

            // TODO: We'll add keyboard handling next
        }

        // Emulate one CPU cycle
        chip8.emulateCycle();

        // Update display if needed
        if (chip8.drawFlag) {
            display.update(chip8.gfx);
            chip8.drawFlag = false;
        }

        // Slow down emulation to ~60 FPS
        SDL_Delay(16);  // ~60 Hz (1000ms / 60 = 16.67ms)
    }

    display.cleanup();
    return 0;
}
```

**What's happening:**

1. **Display class**: Encapsulates all SDL2 stuff
   - `initialize()`: Creates window, renderer, and texture
   - `update()`: Converts CHIP-8's gfx array to SDL pixels and renders
   - `cleanup()`: Frees SDL resources

2. **Main loop**:
   - Handle SDL events (window close, keyboard, etc.)
   - Run one CPU cycle
   - If drawFlag is set, update the display
   - Delay 16ms (≈60 FPS)

3. **Pixel format**: RGBA8888 means 32 bits per pixel (8 bits each for Red, Green, Blue, Alpha)
   - `0xFFFFFFFF`: White (R=255, G=255, B=255, A=255)
   - `0x00000000`: Black (R=0, G=0, B=0, A=0)

**Rebuild and test:**

```bash
make clean
make
./chip8 test.c8
```

You should see a black window appear! It won't show anything yet because our test ROM doesn't draw. Let's fix that.

#### Create a drawing test ROM

```python
# save as draw_test.py
opcodes = [
    0x00E0,  # Clear screen
    0x6000,  # Set V0 to 0 (X coordinate)
    0x6100,  # Set V1 to 0 (Y coordinate)
    0xA000,  # Set I to 0x000 (fontset location for '0')
    0xD015,  # Draw sprite at (V0, V1), height 5
    0x1208,  # Jump to 0x208 (infinite loop to keep window open)
]

with open('draw_test.c8', 'wb') as f:
    for op in opcodes:
        f.write(op.to_bytes(2, byteorder='big'))

print("Created draw_test.c8")
```

Run: `python3 draw_test.py`

```bash
./chip8 draw_test.c8
```

**You should see a "0" character drawn in the top-left corner of the window!** 🎉

**Checkpoint passed!** You're rendering graphics! ✅

---

### Step 6: Add Input Handling

CHIP-8 has a 16-key hexadecimal keypad. We'll map it to the keyboard.

Update the event handling in main.cpp:

```cpp
// In the main loop, replace the TODO section:
while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
        running = false;
    }

    // Key down
    if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
            case SDLK_1: chip8.key[0x1] = 1; break;
            case SDLK_2: chip8.key[0x2] = 1; break;
            case SDLK_3: chip8.key[0x3] = 1; break;
            case SDLK_4: chip8.key[0xC] = 1; break;

            case SDLK_q: chip8.key[0x4] = 1; break;
            case SDLK_w: chip8.key[0x5] = 1; break;
            case SDLK_e: chip8.key[0x6] = 1; break;
            case SDLK_r: chip8.key[0xD] = 1; break;

            case SDLK_a: chip8.key[0x7] = 1; break;
            case SDLK_s: chip8.key[0x8] = 1; break;
            case SDLK_d: chip8.key[0x9] = 1; break;
            case SDLK_f: chip8.key[0xE] = 1; break;

            case SDLK_z: chip8.key[0xA] = 1; break;
            case SDLK_x: chip8.key[0x0] = 1; break;
            case SDLK_c: chip8.key[0xB] = 1; break;
            case SDLK_v: chip8.key[0xF] = 1; break;

            case SDLK_ESCAPE: running = false; break;
        }
    }

    // Key up
    if (event.type == SDL_KEYUP) {
        switch (event.key.keysym.sym) {
            case SDLK_1: chip8.key[0x1] = 0; break;
            case SDLK_2: chip8.key[0x2] = 0; break;
            case SDLK_3: chip8.key[0x3] = 0; break;
            case SDLK_4: chip8.key[0xC] = 0; break;

            case SDLK_q: chip8.key[0x4] = 0; break;
            case SDLK_w: chip8.key[0x5] = 0; break;
            case SDLK_e: chip8.key[0x6] = 0; break;
            case SDLK_r: chip8.key[0xD] = 0; break;

            case SDLK_a: chip8.key[0x7] = 0; break;
            case SDLK_s: chip8.key[0x8] = 0; break;
            case SDLK_d: chip8.key[0x9] = 0; break;
            case SDLK_f: chip8.key[0xE] = 0; break;

            case SDLK_z: chip8.key[0xA] = 0; break;
            case SDLK_x: chip8.key[0x0] = 0; break;
            case SDLK_c: chip8.key[0xB] = 0; break;
            case SDLK_v: chip8.key[0xF] = 0; break;
        }
    }
}
```

Now add the key-related opcodes to chip8.cpp:

```cpp
case 0xE000: {
    switch (opcode & 0x00FF) {
        case 0x009E: {  // 0xEX9E: Skip next instruction if key VX is pressed
            unsigned char x = (opcode & 0x0F00) >> 8;
            if (key[V[x]] != 0) {
                pc += 4;  // Skip next instruction
            } else {
                pc += 2;
            }
            break;
        }

        case 0x00A1: {  // 0xEXA1: Skip next instruction if key VX is NOT pressed
            unsigned char x = (opcode & 0x0F00) >> 8;
            if (key[V[x]] == 0) {
                pc += 4;  // Skip next instruction
            } else {
                pc += 2;
            }
            break;
        }

        default:
            std::cout << "Unknown 0xE000 opcode: 0x" << std::hex << opcode << std::dec << std::endl;
            pc += 2;
            break;
    }
    break;
}
```

---

### Step 7: Implement Remaining Opcodes

Now let's fill in all the remaining opcodes. I'll group them by category.

#### Subroutine Opcodes

```cpp
case 0x0000: {
    switch (opcode & 0x00FF) {
        case 0x00E0:  // Clear screen (already implemented)
            memset(gfx, 0, sizeof(gfx));
            drawFlag = true;
            pc += 2;
            break;

        case 0x00EE:  // 0x00EE: Return from subroutine
            sp--;           // Decrement stack pointer
            pc = stack[sp]; // Get return address from stack
            pc += 2;        // Move past the CALL instruction
            break;

        default:
            std::cout << "Unknown 0x0000 opcode: 0x" << std::hex << opcode << std::dec << std::endl;
            pc += 2;
            break;
    }
    break;
}

case 0x2000: {  // 0x2NNN: Call subroutine at NNN
    unsigned short nnn = (opcode & 0x0FFF);
    stack[sp] = pc;     // Store current address on stack
    sp++;               // Increment stack pointer
    pc = nnn;           // Jump to subroutine
    break;
}
```

**How subroutines work:**
1. **CALL (0x2NNN)**: Save current PC on stack, jump to address
2. **RETURN (0x00EE)**: Pop PC from stack, go back

It's like bookmarking where you left off!

#### Conditional Opcodes

```cpp
case 0x3000: {  // 0x3XNN: Skip next instruction if VX == NN
    unsigned char x = (opcode & 0x0F00) >> 8;
    unsigned char nn = (opcode & 0x00FF);
    if (V[x] == nn) {
        pc += 4;
    } else {
        pc += 2;
    }
    break;
}

case 0x4000: {  // 0x4XNN: Skip next instruction if VX != NN
    unsigned char x = (opcode & 0x0F00) >> 8;
    unsigned char nn = (opcode & 0x00FF);
    if (V[x] != nn) {
        pc += 4;
    } else {
        pc += 2;
    }
    break;
}

case 0x5000: {  // 0x5XY0: Skip next instruction if VX == VY
    unsigned char x = (opcode & 0x0F00) >> 8;
    unsigned char y = (opcode & 0x00F0) >> 4;
    if (V[x] == V[y]) {
        pc += 4;
    } else {
        pc += 2;
    }
    break;
}

case 0x9000: {  // 0x9XY0: Skip next instruction if VX != VY
    unsigned char x = (opcode & 0x0F00) >> 8;
    unsigned char y = (opcode & 0x00F0) >> 4;
    if (V[x] != V[y]) {
        pc += 4;
    } else {
        pc += 2;
    }
    break;
}
```

**Skip instructions** are like IF statements. They let programs make decisions!

#### Register Operations (0x8000 family)

```cpp
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
            std::cout << "Unknown 0x8000 opcode: 0x" << std::hex << opcode << std::dec << std::endl;
            pc += 2;
            break;
    }
    break;
}
```

**Register operations** are math and logic operations. VF often holds a flag (carry, borrow, etc.).

#### Jump and Random Opcodes

```cpp
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
```

#### Timer and Memory Opcodes (0xF000 family)

```cpp
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
            std::cout << "Unknown 0xF000 opcode: 0x" << std::hex << opcode << std::dec << std::endl;
            pc += 2;
            break;
    }
    break;
}
```

**BCD (Binary-Coded Decimal)** is used to split a number into its digits for displaying scores.

---

### Step 8: Polish and Optimize

Remove the debug print statements from emulateCycle() for better performance:

```cpp
void Chip8::emulateCycle() {
    // Fetch opcode
    opcode = memory[pc] << 8 | memory[pc + 1];

    // Remove this line for production:
    // std::cout << "PC: 0x" << std::hex << pc << " Opcode: 0x" << opcode << std::dec << std::endl;

    // ... rest of the code ...
}
```

You can also add a toggle to enable/disable debug output.

---

### Step 9: Test with Real Games!

Now try running the included ROMs:

```bash
make
./chip8 pong2.c8
./chip8 invaders.c8
./chip8 tetris.c8
```

**Pong controls:**
- Player 1: W (up), S (down)
- Player 2: Arrow Up, Arrow Down (you'll need to map these in SDL)

**Space Invaders controls:**
- Q/E: Move left/right
- W: Shoot

**If games don't work:**
1. Make sure all opcodes are implemented
2. Check that timers are decrementing
3. Verify the fetch-decode-execute cycle
4. Add debug output to see which opcodes are being executed

---

## Testing & Debugging

### Common Issues and Solutions

**Issue: Black screen, nothing renders**
- Check that `drawFlag` is being set in the draw opcode
- Verify SDL display is updating when drawFlag is true
- Make sure sprites are being drawn (add debug output to draw opcode)

**Issue: Program crashes or freezes**
- Check for infinite loops (PC not incrementing)
- Verify stack isn't overflowing (SP < 16)
- Make sure memory accesses are within bounds (0x000-0xFFF)

**Issue: Graphics are garbled**
- Verify sprite wrapping (modulo 64 and 32)
- Check XOR logic in draw opcode
- Make sure you're reading sprite data from the correct memory address (I)

**Issue: Input doesn't work**
- Print key states when keys are pressed
- Verify key mapping is correct
- Check that key opcodes (0xE000) are implemented

### Debug Mode

Add a debug flag to your Chip8 class:

```cpp
class Chip8 {
public:
    bool debug = false;  // Set to true for debug output

    // ...
};
```

Then in emulateCycle(), add:

```cpp
if (debug) {
    std::cout << "PC: 0x" << std::hex << pc << " Opcode: 0x" << opcode << std::dec << std::endl;
}
```

Enable it in main:
```cpp
chip8.debug = true;
```

---

## Resources

### Original Tutorial & Further Reading

1. **Original Guide by Laurence Muller**: http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/
   - The guide this README is based on and expands upon
   - More concise, assumes more background knowledge
   - Uses GLUT/OpenGL for graphics

2. **Cowgod's CHIP-8 Technical Reference**: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
   - The most detailed CHIP-8 specification
   - Essential reference for all 35 opcodes

3. **CHIP-8 Technical Reference**: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM
   - Alternative detailed specification

4. **Test ROMs**: https://github.com/corax89/chip8-test-rom
   - Test suites to verify your emulator works correctly

5. **More games**: https://www.zophar.net/pdroms/chip8.html
   - Collection of CHIP-8 ROMs to test

### Next Steps

Once you have a working emulator:

1. **Optimize performance**: Profile and speed up slow opcodes
2. **Add sound**: Use SDL_Audio to make actual beeps
3. **Add features**: Save states, pause, step-through debugger
4. **Different CHIP-8 variants**: SUPER-CHIP, XO-CHIP (extended versions)
5. **Move to a harder emulator**: Game Boy, NES, etc!

### Improving Your Emulator

**Add a disassembler**: Print human-readable opcodes
```
0x6105 → LD V1, 0x05
0xD125 → DRW V1, V2, 5
```

**Add breakpoints**: Pause when PC reaches a certain address

**Add memory viewer**: Show memory contents in real-time

**Improve timing**: Use actual 60 Hz timer instead of SDL_Delay

---

## Conclusion

**Congratulations!** 🎉

You've just built a complete CHIP-8 emulator from scratch! You now understand:

- How CPUs execute instructions
- Memory management and addressing
- Binary, hexadecimal, and bitwise operations
- Graphics rendering and input handling
- Timers and synchronization

This knowledge applies to ALL emulators and low-level programming. The concepts you learned here (fetch-decode-execute, opcodes, registers, memory) are the same in Game Boy, NES, PlayStation, and even modern CPUs!

**What you've accomplished:**
- ✅ Built a CPU emulator
- ✅ Implemented 35 different instructions
- ✅ Created a graphics system
- ✅ Handled real-time input
- ✅ Run actual games from the 1970s

You should be proud! This is a significant programming achievement.

---

## Appendix: Quick Reference

### Memory Map
```
0x000-0x1FF: Interpreter (font + reserved)
0x200-0xFFF: Program ROM space
```

### Registers
```
V0-VF: General purpose (VF is flag register)
I: Index register (memory address)
PC: Program counter
SP: Stack pointer
```

### Keypad Mapping
```
CHIP-8:  1 2 3 C        Keyboard: 1 2 3 4
         4 5 6 D                  Q W E R
         7 8 9 E                  A S D F
         A 0 B F                  Z X C V
```

### Build Commands
```bash
make            # Compile
make clean      # Remove build files
./chip8 rom.c8  # Run emulator
```

Good luck with your emulation journey! 🚀
