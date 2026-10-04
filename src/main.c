/**
 * Main file for CHIP-8 emulator
 */


#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MEM_SIZE 4096
#define ROM_START 0x200

typedef struct {
    uint8_t mem[MEM_SIZE];
    uint8_t V[16];
    uint16_t I;
    uint16_t pc;
} Chip8;

uint16_t fetch_instruction(Chip8 *chip);

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

    size_t loaded_bytes;
    if ((loaded_bytes = fread(chip.mem + ROM_START, sizeof(chip.mem[0]), size, rom_file)) != (size_t) size) {
        fprintf(stderr, "Issue reading ROM file.\n");
        fclose(rom_file);
        return -1;
    }

    fclose(rom_file);

    for (int i = 0; i < 6; i++) {
        uint16_t prev_pc = chip.pc;
        uint16_t opcode = fetch_instruction(&chip);
        printf("Program counter: 0x%03X, Instruction: 0x%04X \n", prev_pc, opcode);
    }

    return 0;
}

uint16_t fetch_instruction(Chip8 *chip) {
    uint16_t opcode = chip->mem[chip->pc] << 8 | chip->mem[chip->pc + 1];
    chip->pc += 2;

    return opcode;
}