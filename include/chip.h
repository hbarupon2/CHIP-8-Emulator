#pragma once

#include <stdlib.h>

#define MEM_SIZE 4096
#define ROM_START 0x200
#define FONT_START 0x050

#define DISPLAY_W 64
#define DISPLAY_H 32

typedef struct {
    uint8_t mem[MEM_SIZE];
    uint8_t V[16];
    uint16_t I;
    uint16_t pc;
    uint8_t display[DISPLAY_W * DISPLAY_H];
    uint16_t stack[16];
    uint8_t sp;
    uint8_t delay_timer;
    uint8_t sound_timer;
    uint8_t keys[16];
    uint8_t draw_flag;
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
