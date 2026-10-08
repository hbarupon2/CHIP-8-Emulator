/**
 * Main file for CHIP-8 emulator
 */


#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <time.h>

#include "chip.h"
#include "fontset.h"
#include "utilities.h"

#define SLOW_STEP 0 // 1 for slow step, 0 for full speed

int main(int argc, char **argv);


int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./CHIP8 path/to/rom \n");
        return -1;
    }

    FILE *rom_file = fopen(argv[1], "rb");
    if (rom_file == NULL) {
        fprintf(stderr, "Could not open ROM file: %s\n", argv[1]);
        return -1;
    }

    if (fseek(rom_file, 0, SEEK_END) != 0) {
        fclose(rom_file);
        return -1;
    }

    long size;
    if ((size = ftell(rom_file)) == -1) {
        fclose(rom_file);
        return -1;
    }
    printf("%ld\n", size);

    if (size > (MEM_SIZE - ROM_START)) {
        fclose(rom_file);
        fprintf(stderr, "ROM file too large\n");
        return -1;
    }

    if (fseek(rom_file, 0, SEEK_SET) != 0) {
        fclose(rom_file);
        return -1;
    }

    Chip8 chip = {0};
    chip.pc = ROM_START;

    size_t loaded_bytes = fread(chip.mem + ROM_START, sizeof(chip.mem[0]), size, rom_file);
    if (loaded_bytes != (size_t) size) {
        fprintf(stderr, "Issue reading ROM file.\n");
        fclose(rom_file);
        return -1;
    }

    fclose(rom_file);

    for (size_t i = FONT_START; i < FONT_START + sizeof(chip8_fontset) / sizeof(char); i++)
        chip.mem[i] = chip8_fontset[i - FONT_START];

    // Open SDl window

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window *window;
    SDL_Renderer *renderer;

    if (!SDL_CreateWindowAndRenderer(
            "CHIP-8 Emulator",
            DISPLAY_W * 10,
            DISPLAY_H * 10,
            0,
            &window,
            &renderer)
    ) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not create window and renderer - %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    SDL_SetRenderScale(renderer, 10, 10);

    int should_continue = 0;
    bool should_quit = false;

    while (should_continue != -1 && should_quit == false) {
        /*
        uint16_t prev_pc = chip.pc;
        uint16_t opcode = fetch_instruction(&chip);
        Instruction instruction = decode_instruction(opcode);
        printf("Program counter: 0x%03X, Instruction: 0x%04X, First Nibble: 0x%01X, X: 0x%01X, Y: 0x%01X, N: 0x%01X, "
               "NN: 0x%02X, NNN: 0x%03X \n",
               prev_pc, opcode, instruction.first_nibble, instruction.x, instruction.y, instruction.n, instruction.nn,
               instruction.nnn);
        */

        constexpr int cycles = SLOW_STEP ? 1 : CYCLES_PER_FRAME;
        for (int i = 0; i < cycles; i++) {
            should_continue = execute_instruction(&chip, decode_instruction(fetch_instruction(&chip)));
            if (should_continue == -1)
                break;
        }

        if (chip.delay_timer > 0)
            chip.delay_timer--;
        if (chip.sound_timer > 0)
            chip.sound_timer--;

        if (chip.draw_flag) {
            render_display(&chip, renderer);
        }

        if (SLOW_STEP) {
            dump_registers(&chip);
        }

        const uint64_t time = now_ns() + (SLOW_STEP ? 1e9 + 250000000ull : FRAME_NS);
        while (!should_quit && now_ns() < time) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) {
                    should_quit = true;
                }
                if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
                    int key = chip8_key(event.key.scancode);
                    if (key != -1)
                        chip.keys[key] = event.key.down;
                }
            }
            sleep_until(now_ns() + 16000000ull);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
