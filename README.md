
https://user-images.githubusercontent.com/20952474/125767156-247666d7-33e1-4bb8-9ee9-c5eeb5e86349.mp4

# Chip-8

Chip-8 is a simple, interpreted, programming language which was first used on some do-it-yourself computer systems in the late 1970s and early 1980s. The COSMAC VIP, DREAM 6800, and ETI 660 computers are a few examples.
These computers typically were designed to use a television as a display, had between 1 and 4K of RAM, and used a 16-key hexadecimal keypad for input. [<sup>1</sup>][^1]

## Specifications
### Memory
**4096 bytes of RAM**  
* `0x000 - 0x1ff` Reserved (used to be for the interpreter itself)
   - `0x50 - 0xAA`  Fontset: 16 * 5 bytes sprites (`0-F`)
      - Example for the `0` sprite:  
        | Sprite                  | Binary     | Hex  |
        |-------------------------|------------|------|
        | + + + + | 0b11110000 | 0xF0 |
        | + &nbsp;  &nbsp;  &nbsp;  + | 0b10010000 | 0x90 |
        | + &nbsp;  &nbsp;  &nbsp;  + | 0b10010000 | 0x90 |
        | + &nbsp;  &nbsp;  &nbsp;  + | 0b10010000 | 0x90 |
        | + + + + | 0b11110000 | 0xF0 |
* `0x200`- Entry Point of most Chip-8 programs
___
### Registers
- **`vA-vF`** 16 * 8-bit registers  
- **`I`** 16-bit Index register  
- **`PC`** 16-bit Program Counter  
- **`STACK`** 16 * 16-bit values to store return addresses from subroutines  
- **`SP`** 8-bit Stack Pointer (unused, as I use `std::vector` for the stack)  
- **`DT`** 8-bit Delay Timer  
- **`ST`** 8-bit Sound Timer  
___
### Display
- Resolution of **`64x32`**
- The TUI uses a resolution of **`64x16`** as it make uses of the lower half-block unicode character `▄` in combination with the foreground/background color of a character cell to double the vertical resolution and simulate the correct aspect ratio.
__
### Timers & Sound
- **`Delay Timer`**: Is active whenever the **`DT`** register is non-zero. Is decremented by 1 at the rate of **60Hz**.  
- **`Sound Timer`**: Is active whenever the **`ST`** register is non-zero. Is decremented by 1 at the rate of **60hz**.  
    - When active, the Chip-8 buzzer will sound. This is emulated by printing the emulator frame in **red**.
___
### HexPad
**Used:**  

    1 2 3 4  
    Q W E R  
    A S D F  
    Z X C V

**Chip-8:**  

    1 2 3 C  
    4 5 6 D  
    7 8 9 E  
    A 0 B F  
___
### Instruction Set & Assembly mnemonics

- The Chip-8 has 36 different instructions  
- All instruction are two bytes long  
- The first byte of each instruction should be located at an even address in memory  
    - Sprites can be an uneven number of bytes, but must be padded so instructions following it stays properly situated in memory  
  
  
Variables in instructions are represented this way:  
    - **`0xfXYf`** X,Y = Nibbles (Used to designate registers)  
    - **`0xfNNN`** NNN = Address  
    - **`0xfNNf`** NN  = Byte  
    - **`0xfffN`** N   = Nibble  


| **Opcode** | **Mnemonic** | **Variables** | **Description**                                         |
|--------|----------|-----------|-----------------------------------------------------|
| 0NNN   | **sys**    | NNN       | Jump to machine code routine at NNN                 |
| 00E0   | **cls**     |           | Clear the display                                   |
| 00EE   | **ret**     |           | Return from a subroutine                            |
| 1NNN   | **jp**     | NNN       | Jump to location NNN                                |
| 2NNN   | **call**     | NNN       | Call subroutine NNN                                 |
| 3XNN   | **se**     | VX, NN    | Skip the next instruction if VX == NN               |
| 4XNN   | **sne**     | VX, NN    | Skip the next instruction if VX != NN               |
| 5XY0   | **se**     | VX, NN    | Skip the next instruction if VX = VY                |
| 6XNN   | **ld**     | VX, NN    | Set VX  = NN                                        |
| 7XNN   | **add**     | VX, NN    | Set VX += NN                                        |
| 8XY0   | **ld**     | VX, VY    | Set VX  = VY                                        |
| 8XY1   | **or**     | VX, VX    | Set VX \|= VY                                       |
| 8XY2   | **and**     | VX, VY    | Set VX &= VY                                        |
| 8XY3   | **xor**     | VX, VY    | Set VX ^= VY                                        |
| 8XY4   | **add**     | VX, VY    | Set VX += VY, VF = carry                            |
| 8XY5   | **sub**     | VX, VY    | Set VX -= VY, VF = !BORROW                          |
| 8XY6   | **shr**     | VX, {,VY} | Set VX >>= 1, VF = LSB(VX), VX /= 2                 |
| 8XY7   | **subn**     | VX, VY    | Set VX = VY - VX, VF = !BORROW                      |
| 8XYE   | **shl**     | VX, {,VY} | Set VX <<= 1, VF = LSB(VX), VX *= 2                 |
| 9XY0   | **sne**     | VX, VY    | Skip next instruction if VX != VY                   |
| ANNN   | **ld**     | I, NNN    | Set I = NNN                                         |
| BNNN   | **jp**     | V0, NNN   | Jump to location NNN + V0                           |
| CXNN   | **rnd**     | VX, NN    | Set VX = rng & NN                                   |
| DXYN   | **drw**     | VX, VY, N | Display memory I to I+N at (VX, VY), VF = collision |
| EX9E   | **skp**     | VX        | Skip next instruction if keypad[VX]                 |
| EXA1   | **skpn**     | VX        | Skip next instruction if !keypad[VX]                |
| FX07   | **ld**     | VX, DT    | Set VX = DT                                         |
| FX0A   | **ld**     | VX, KEY   | Wait for input, Set VX = KEY                        |
| FX15   | **ld**     | DT, VX    | Set DT = VX                                         |
| FX18   | **ld**     | ST, VX    | Set ST = VX                                         |
| FX1E   | **add**     | I, VX     | Set I += VX                                         |
| FX29   | **ld**     | CHAR, VX  | Set I = location of sprite for digit VX             |
| FX33   | **ld**     | BCD, VX   | Store BCD(VX) at memory I, I+1, I+2                 |
| FX55   | **ld**     | [I], VX   | Store registers V0->VX in memory from I             |
| FX65   | **ld**     | VX, [I]   | Store memory from I to V0->VX                       |
___
# Resources

[Cowgod's Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTML)

# References
[^1]: http://devernay.free.fr/hacks/chip8/C8TECH10.HTM#1.0


