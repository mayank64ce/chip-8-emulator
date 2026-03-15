#include <iostream>
#include <SDL2/SDL.h>
#include "chip8.h"

const int SCREEN_WIDTH = 64;
const int SCREEN_HEIGHT = 32;
const int SCALE = 10; // Scale up the display

class Display{
    public:
        Display() : window(nullptr), renderer(nullptr), texture(nullptr) {}

        bool initialize(){
            if(SDL_Init(SDL_INIT_VIDEO) < 0){
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

            if(!window){
                std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
                return false;
            }

            renderer = SDL_CreateRenderer(
                window,
                -1,
                SDL_RENDERER_ACCELERATED
            );

            if(!renderer){
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

            if(!texture){
                std::cerr << "Texture creation failed: " << SDL_GetError() << std::endl;
                return false;
            }

            return true;
        }

        void update(const unsigned char* gfx){
            // Create a buffer of pixels in RGBA format
            uint32_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];

            for(int i=0; i<SCREEN_WIDTH * SCREEN_HEIGHT;i++){
                // If pixel is on, make it white, otherwise make it black
                pixels[i] = gfx[i] ? 0xFFFFFFFF: 0x00000000;
            }

            SDL_UpdateTexture(texture, nullptr, pixels, SCREEN_WIDTH * sizeof(uint32_t));
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);
        }

        void cleanup() {
            if(texture) SDL_DestroyTexture(texture);
            if(renderer) SDL_DestroyRenderer(renderer);
            if(window) SDL_DestroyWindow(window);
            SDL_Quit();
        }

    private:
        SDL_Window* window;
        SDL_Renderer* renderer;
        SDL_Texture* texture;
};

int main(int argc, char** argv){
    if(argc != 2){
        std::cout << "Usage: " << argv[0] << " <ROM file>" << std::endl;
        return 1;
    }

    Chip8 chip8;
    Display display;

    if(!display.initialize()){
        return 1;
    }

    if(!chip8.loadROM(argv[1])){
        return 1;
    }

    bool running = true;
    SDL_Event event;

    // Main emulation loop
    while(running){
        // Handle events
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_QUIT){
                running = false;
            }

            // Key Down
            if(event.type == SDL_KEYDOWN){
                switch(event.key.keysym.sym){
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

            // Key Up
            if(event.type == SDL_KEYUP){
                switch(event.key.keysym.sym){
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

        // Emulate one CPU cycle
        chip8.emulateCycle();

        // Update display if needed
        if(chip8.drawFlag){
            display.update(chip8.gfx);
            chip8.drawFlag = false;
        }

        // Slow down emulation to ~60 FPS
        SDL_Delay(16); // ~60 Hz (1000 ms / 60 = 16.67 ms)

    }

    display.cleanup();

    return 0;
}