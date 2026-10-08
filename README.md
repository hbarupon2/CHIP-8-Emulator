# CHIP-8 Emulator

Emulator for CHIP-8 processor.

## Running It

### macOS
Requires CMake and SDL3 (Homebrew: `brew install sdl3 cmake`).

    ./run-emulator.sh path/to/rom.ch8

If CMake cannot find SDL3, modify the `run-emulator.sh` build command with
`-DCMAKE_PREFIX_PATH=/opt/homebrew`.

The window that opens is a 64x32 display upscaled by 10.

Key map:

| CHIP-8 | QWERTY Keyboard |
|---|---|
| 1 2 3 C | 1 2 3 4 |
| 4 5 6 D | Q W E R |
| 7 8 9 E | A S D F |
| A 0 B F | Z X C V |

The processor runs at 700 instructions per second by default. If you
would like to slow step it to debug your code, change `SLOW_STEP` in `src/main.c` to `1`.
Setting it to `0` will run it at full speed.

* Note: no audio output is configured yet

## Assembler

To assemble code into a ROM, use the assembler. The programs
are assembled using the Cowgod mnemonics into a `.ch8` file.

    ./run-assembler.sh /path/to/code.s

The assembler writes the .ch8 file next to the .s file.

Comments are supported at the end of the line with a `;`.

Labels are currently not supported, so jump instructions require
raw addresses (for now). The ROM is loaded at `0x200`. Each instruction is 2 bytes.

## Some More Details
* 4 KB of RAM
* 16 8-bit registers (V0-VF)
  * VF doubles as a flag register
* Index register (I) for storing memory addresses
* Program counter - starts at 0x0200
* 16 return address stack
* 8 bit delay and sound timer
* 64 x 32 px display
* 16 keys, labeled 0 to F in a 4x4 grid
* 2-byte opcodes
  * Big endian

    
## FPGA Implementation
Will feature Verilog code that can be uploaded to an FPGA and tested.