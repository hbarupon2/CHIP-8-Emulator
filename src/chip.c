#include "chip.h"

#include <stdio.h>

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
        case 0x0:
            switch (instruction.nn) {
                case 0xE0:
                    for (size_t i = 0; i < sizeof(chip->display) / sizeof(uint8_t); i++)
                        chip->display[i] = 0;
                    break;

                case 0xEE:
                    chip->pc = chip->stack[--chip->sp];
                    break;
                default:
                    goto unimplemented;
            }
            break;
        case 0x1:
            chip->pc = instruction.nnn;
            break;
        case 0x2:
            chip->stack[chip->sp++] = chip->pc;
            chip->pc = instruction.nnn;
            break;
        case 0x3:
            if (chip->V[instruction.x] == instruction.nn)
                chip->pc++;
            break;
        case 0x4:
            if (chip->V[instruction.x] != instruction.nn)
                chip->pc++;
            break;
        case 0x6:
            chip->V[instruction.x] = instruction.nn;
            break;
        case 0x7:
            chip->V[instruction.x] += instruction.nn;
            break;
        case 0x8:
            switch (instruction.n) {
                case 0x0:
                    chip->V[instruction.x] = chip->V[instruction.y];
                    break;
                case 0x1:
                    chip->V[instruction.x] = chip->V[instruction.x] | chip->V[instruction.y];
                    break;
                case 0x2:
                    chip->V[instruction.x] = chip->V[instruction.x] & chip->V[instruction.y];
                    break;
                case 0x3:
                    chip->V[instruction.x] = chip->V[instruction.x] ^ chip->V[instruction.y];
                    break;
                case 0x4:
                    const int sum = chip->V[instruction.x] + chip->V[instruction.y];
                    if (sum > 255)
                        chip->V[0xF] = 1;
                    chip->V[instruction.x] = sum;
                    break;
                case 0x5:
                    const int sub = chip->V[instruction.x] - chip->V[instruction.y];
                    if (sub >= 0)
                        chip->V[0xF] = 1;
                    chip->V[instruction.x] = sub;
                    break;
                case 0x6:
                    chip->V[0xF] = chip->V[instruction.x] & 0x1;
                    chip->V[instruction.x] = chip->V[instruction.x] << 1;
                    break;
                case 0x7:
                    const int sub1 = chip->V[instruction.y] - chip->V[instruction.x];
                    if (sub1 >= 0)
                        chip->V[0xF] = 1;
                    chip->V[instruction.x] = sub1;
                    break;
                case 0xE:
                    chip->V[0xF] = chip->V[instruction.x] >> 7;
                    chip->V[instruction.x] = chip->V[instruction.x] << 1;
                    break;
                default:
                    goto unimplemented;
            }
        case 0xA:
            chip->I = instruction.nnn;
            break;
        case 0xB:
            chip->pc = instruction.nnn + chip->V[0];
            break;
        case 0xC:
            chip->V[instruction.x] = (rand() & 0xFF) & instruction.nn;
            break;
        case 0xF:
            switch (instruction.nn) {
                case 0x29:
                    chip->I = FONT_START + (chip->V[instruction.x] & 0xF) * 5;
                    break;
                default:
                    goto unimplemented;
            }
            break;
        default:
            goto unimplemented;
    }

    return 0;

    unimplemented:
    printf("Unimplemented opcode: 0x%04X\n", (instruction.first_nibble << 12) | instruction.nnn);
    return -1;
}
