/**
 * Main file for CHIP-8 emulator
 */


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
    if (loaded_bytes != (size_t)size) {
        fprintf(stderr, "Issue reading ROM file.\n");
        fclose(rom_file);
        return -1;
    }

    fclose(rom_file);

    for (size_t i = FONT_START; i < FONT_START + sizeof(chip8_fontset) / sizeof(char); i++)
        chip.mem[i] = chip8_fontset[i - FONT_START];

    int should_continue = 0;

    while (should_continue != -1) {
        uint16_t prev_pc = chip.pc;
        uint16_t opcode = fetch_instruction(&chip);
        Instruction instruction = decode_instruction(opcode);
        printf("Program counter: 0x%03X, Instruction: 0x%04X, First Nibble: 0x%01X, X: 0x%01X, Y: 0x%01X, N: 0x%01X, "
               "NN: 0x%02X, NNN: 0x%03X \n",
               prev_pc, opcode, instruction.first_nibble, instruction.x, instruction.y, instruction.n, instruction.nn,
               instruction.nnn);

        should_continue = execute_instruction(&chip, instruction);

        for (int i = 0; i < 16; i++)
            printf("V[%d]: 0x%02X\n", i, chip.V[i]);
        printf("I: 0x%04X\n", chip.I);
        printf("PC: 0x%04X\n", chip.pc);

        struct timespec ts = {.tv_sec = 0, .tv_nsec = 2.5e8};
        nanosleep(&ts, NULL);
        clear_screen();
    }

    return 0;
}