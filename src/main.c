/**
 * Main file for CHIP-8 emulator
 */


#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <time.h>

#include "chip.h"
#include "fontset.h"
#include "utilities.h"

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
        uint64_t frame_start = now_ns();
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                should_quit = true;
            }
        }

        /*
        uint16_t prev_pc = chip.pc;
        uint16_t opcode = fetch_instruction(&chip);
        Instruction instruction = decode_instruction(opcode);
        printf("Program counter: 0x%03X, Instruction: 0x%04X, First Nibble: 0x%01X, X: 0x%01X, Y: 0x%01X, N: 0x%01X, "
               "NN: 0x%02X, NNN: 0x%03X \n",
               prev_pc, opcode, instruction.first_nibble, instruction.x, instruction.y, instruction.n, instruction.nn,
               instruction.nnn);
        */

        for (int i = 0; i < CYCLES_PER_FRAME; i++) {
            should_continue = execute_instruction(&chip, decode_instruction(fetch_instruction(&chip)));
        }

        if (chip.delay_timer > 0)
            chip.delay_timer--;
        if (chip.sound_timer > 0)
            chip.sound_timer--;

        if (chip.draw_flag) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

            for (int y = 0; y < DISPLAY_H; y++) {
                for (int x = 0; x < DISPLAY_W; x++) {
                    if (chip.display[y * DISPLAY_W + x] == 0)
                        continue;
                    SDL_FRect pixel = {x, y, 1, 1};
                    SDL_RenderFillRect(renderer, &pixel);
                }
            }
            SDL_RenderPresent(renderer);
            chip.draw_flag = 0;
        }

        for (int i = 0; i < 16; i++)
            printf("V[%d]: 0x%02X\n", i, chip.V[i]);
        printf("I: 0x%04X\n", chip.I);
        printf("PC: 0x%04X\n", chip.pc);

        sleep_until(frame_start + FRAME_NS);
        clear_screen();
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
