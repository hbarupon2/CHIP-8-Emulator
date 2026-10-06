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
                    chip->draw_flag = 1;
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
                chip->pc += 2;
            break;
        case 0x4:
            if (chip->V[instruction.x] != instruction.nn)
                chip->pc += 2;
            break;
        case 0x5:
            if (instruction.n == 0x0)
                if (chip->V[instruction.x] == chip->V[instruction.y])
                    chip->pc += 2;
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
                    chip->V[instruction.x] = sum;
                    if (sum > 255)
                        chip->V[0xF] = 1;
                    else
                        chip->V[0xF] = 0;
                    break;
                case 0x5:
                    const int sub = chip->V[instruction.x] - chip->V[instruction.y];
                    chip->V[instruction.x] = sub;
                    if (sub >= 0)
                        chip->V[0xF] = 1;
                    else
                        chip->V[0xF] = 0;
                    break;
                case 0x6:
                    const uint8_t vx = chip->V[instruction.x];
                    chip->V[instruction.x] = vx >> 1;
                    chip->V[0xF] = vx & 0x1;
                    break;
                case 0x7:
                    const int sub1 = chip->V[instruction.y] - chip->V[instruction.x];
                    chip->V[instruction.x] = sub1;
                    if (sub1 >= 0)
                        chip->V[0xF] = 1;
                    else
                        chip->V[0xF] = 0;
                    break;
                case 0xE:
                    const uint8_t vx1= chip->V[instruction.x];
                    chip->V[instruction.x] = vx1 << 1;
                    chip->V[0xF] = vx1 >> 7;
                    break;
                default:
                    goto unimplemented;
            }
            break;
        case 0x9:
            if (instruction.n == 0x0)
                if (chip->V[instruction.x] != chip->V[instruction.y])
                    chip->pc += 2;
            break;
        case 0xA:
            chip->I = instruction.nnn;
            break;
        case 0xB:
            chip->pc = instruction.nnn + chip->V[0];
            break;
        case 0xC:
            chip->V[instruction.x] = (rand() & 0xFF) & instruction.nn;
            break;
        case 0xD:
            uint8_t px = chip->V[instruction.x] % DISPLAY_W;
            uint8_t py = chip->V[instruction.y] % DISPLAY_H;
            chip->V[0xF] = 0;
            for (int row = 0; row < instruction.n && py + row < DISPLAY_H; row++) {
                uint8_t sprite = chip->mem[(chip->I + row) % MEM_SIZE];
                for (int col = 0; col < 8 && px + col < DISPLAY_W; col++) {
                    if (sprite & (0x80 >> col)) {
                        uint8_t *pixel = &chip->display[(py + row) * DISPLAY_W + (px + col)];
                        if (*pixel) chip->V[0xF] = 1;
                        *pixel ^= 1;
                    }
                }
            }
            chip->draw_flag = 1;
            break;
        case 0xE:
            switch (instruction.nn) {
                case 0x9E:
                    if (chip->keys[chip->V[instruction.x] & 0xF] == 1)
                        chip->pc += 2;
                    break;
                case 0xA1:
                    if (chip->keys[chip->V[instruction.x] & 0xF] == 0)
                        chip->pc += 2;
                    break;
                default:
                    goto unimplemented;
            }
            break;
        case 0xF:
            switch (instruction.nn) {
                case 0x07:
                    chip->V[instruction.x] = chip->delay_timer;
                    break;
                case 0x0A:
                    int keydown = 0;
                    for (int i = 0; i < 16; i++) {
                        if (chip->keys[i] == 1) {
                            chip->V[instruction.x] = i;
                            keydown = 1;
                            break;
                        }
                    }
                    if (!keydown)
                        chip->pc -= 2;
                    break;
                case 0x15:
                    chip->delay_timer = chip->V[instruction.x];
                    break;
                case 0x18:
                    chip->sound_timer = chip->V[instruction.x];
                    break;
                case 0x1E:
                    chip->I += chip->V[instruction.x];
                    break;
                case 0x29:
                    chip->I = FONT_START + (chip->V[instruction.x] & 0xF) * 5;
                    break;
                case 0x33:
                    if (chip->I < MEM_SIZE)
                        chip->mem[chip->I] = chip->V[instruction.x] / 100;
                    if (chip->I + 1 < MEM_SIZE)
                        chip->mem[chip->I + 1] = (chip->V[instruction.x] / 10) % 10;
                    if (chip->I + 2 < MEM_SIZE)
                        chip->mem[chip->I + 2] = chip->V[instruction.x] % 10;
                    break;
                case 0x55:
                    if (instruction.x <= 16) {
                        for (int i = 0; i <= instruction.x && chip->I + i < MEM_SIZE; i++)
                            chip->mem[chip->I + i] = chip->V[i];
                    }
                    break;
                case 0x65:
                    if (instruction.x <= 16) {
                        for (int i = 0; i <= instruction.x && chip->I + i < MEM_SIZE; i++)
                            chip->V[i] = chip->mem[chip->I + i];
                    }
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
