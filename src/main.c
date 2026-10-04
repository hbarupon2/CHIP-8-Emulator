/**
 * Main file for CHIP-8 emulator
 */


#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define MEM_SIZE 4096
#define ROM_START 0x200

typedef struct {
    uint8_t mem[MEM_SIZE];
    uint8_t V[16];
    uint16_t I;
    uint16_t pc;
} Chip8;

typedef struct {
    uint8_t first_nibble;
    uint8_t x;
    uint8_t y;
    uint8_t n;
    uint8_t nn;
    uint16_t nnn;
} Instruction;

uint16_t fetch_instruction(Chip8 *chip);
Instruction decode_instruction(uint16_t opcode);
int execute_instruction(Chip8 *chip, Instruction instruction);

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

    int should_continue = 0;

    while (should_continue != -1) {
        uint16_t prev_pc = chip.pc;
        uint16_t opcode = fetch_instruction(&chip);
        Instruction instruction = decode_instruction(opcode);
        printf(
            "Program counter: 0x%03X, Instruction: 0x%04X, First Nibble: 0x%01X, X: 0x%01X, Y: 0x%01X, N: 0x%01X, NN: 0x%02X, NNN: 0x%03X \n",
            prev_pc, opcode, instruction.first_nibble, instruction.x, instruction.y, instruction.n, instruction.nn,
            instruction.nnn);

        should_continue = execute_instruction(&chip, instruction);

        for (int i = 0; i < 16; i++)
            printf("V[%d]: 0x%02X\n", i, chip.V[i]);
        printf("I: 0x%04X\n", chip.I);
        printf("PC: 0x%04X\n", chip.pc);

        struct timespec ts = { .tv_sec = 0, .tv_nsec = 2.5e8 };
        nanosleep(&ts, NULL);
        fflush(stdout);
    }

    return 0;
}

uint16_t fetch_instruction(Chip8 *chip) {
    const uint16_t opcode = chip->mem[chip->pc] << 8 | chip->mem[chip->pc + 1];
    chip->pc += 2;

    return opcode;
}

Instruction decode_instruction(uint16_t opcode) {
    Instruction instruction = {0};
    instruction.first_nibble = (opcode >> 12) & 0xF;
    instruction.x = (opcode >> 8) & 0xF;
    instruction.y = (opcode >> 4) & 0xF;
    instruction.n = opcode & 0xF;
    instruction.nn = opcode & 0xFF;
    instruction.nnn = opcode & 0xFFF;

    return instruction;
}

int execute_instruction(Chip8 *chip, Instruction instruction) {
    switch (instruction.first_nibble) {
        case 0x6:
            chip->V[instruction.x] = instruction.nn;
            break;
        case 0x7:
            chip->V[instruction.x] += instruction.nn;
            break;
        case 0xA:
            chip->I = instruction.nnn;
            break;
        case 0x1:
            chip->pc = instruction.nnn;
            break;
        default:
            printf("Unimplemented opcode: 0x%01X\n", instruction.first_nibble);
            return -1;
    }

    return 0;
}
