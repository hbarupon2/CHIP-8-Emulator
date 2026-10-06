# CHIP-8 Emulator

Emulator for CHIP-8 processor.

## How it works
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

## Instruction decoding

## The assember


## FPGA Implementation
Will feature Verilog codes that can be uploaded to an FPGA
to simulate this processor. Flash programs to the FPGA
to run them.