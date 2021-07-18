
https://user-images.githubusercontent.com/20952474/125767156-247666d7-33e1-4bb8-9ee9-c5eeb5e86349.mp4

# Chip-8
A Chip-8 emulator (interpreter, to be pedantic), debugger, and disassembler, made to run in a terminal. 
The emulator passes all testing ROMS I could get my hands on.
The disassembler isn't recursive; does not have any dependencies (code wise), and all its methods are `static`.
It can be used like this: `Disassembler::Disassemble(filename)`
The debugger, is pretty much what you'd expect, a nice UI to track registers and such, and the ability to step through the assembly while the program is running.

## Usage
To download and compile:

    git clone https://github.com/Hab5/chip8.git &&
    cd chip8 &&
    make release
    
To run the emulator: 

    ./chip8 your-file
Some ROMS are included in the `roms/` directory. ([Credit](#credit-for-the-roms-included))

## Dependency
- `ncurses` for the `Text User Interface (TUI)`
   - Most likely on your system already, if not, it is packaged for every package manager out there.  

## Keybindings
|              |                       |
|--------------|-----------------------|
| **`Escape`** | Quit                  |
| **`Enter`**  | Reset ROM             |
| **`Space`**  | Pause                 |
| **`Tab`**    | Step (when paused)    |
| **`-`**      | Decrease speed (20Hz) |
| **`+`**      | Increase speed (20Hz) |

See [HexPad](#hexpad) for the Chip-8 keyboard.

## Specifications
Most of the information is sourced from [Cowgod's Chip-8 Technical Reference](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM); the de facto  Chip-8 reference.  
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
- **`SP`** 8-bit Stack Pointer (unused, as I make use of `std::vector` for the stack)  
- **`DT`** 8-bit Delay Timer  
- **`ST`** 8-bit Sound Timer  
___
### Display
- Resolution of **`64x32`**
- The TUI uses a resolution of **`64x16`** as it make uses of the lower half-block unicode character `▄` in combination with the foreground/background color of a character cell to double the terminal's vertical resolution and simulate the correct aspect ratio.
___
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

**Original Chip-8:**  

    1 2 3 C  
    4 5 6 D  
    7 8 9 E  
    A 0 B F  
___
### Instruction Set & Assembly mnemonics

- The Chip-8 has 36 different instructions  
- All instructions are two bytes long  
- The first byte of each instructions should be located at an even address in memory  
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
[Mattmikolay's Instruction Set Details](https://github.com/mattmikolay/chip-8/wiki/CHIP%E2%80%908-Instruction-Set)
[Wikipedia's Chip-8 Page](https://en.wikipedia.org/wiki/CHIP-8)

# Credit for the ROMS included  
- Testing Roms:
   - https://github.com/corax89/chip8-test-rom
   - https://github.com/Skosulor/c8int/tree/master/test
   - https://github.com/metteo/chip8-test-rom
- https://github.com/dmatlack/chip8/tree/master/roms (For everything else, details about authors in filename)


