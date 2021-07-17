#include "Chip8.hpp"

// The Dispatch hashtable, is in the header file.

Chip8::Chip8(const std::string& filename): filename(filename) {
    LoadROM(filename);
    LoadFont();
}

void Chip8::Cycle() {
    if (!paused || step) {
        OP = memory[PC] << 8 | memory[PC+1]; // Next two bytes
        PC += 2; step = false; // Increment PC for next cycle

        auto op_ptr = std::function<void()>();
        // Masks ordered that way to avoid collision
        for (auto mask: {0xf0ff, 0xf00f, 0xf000})
            if ((op_ptr = Dispatch[(OP & mask)]) != nullptr)
                return std::invoke(op_ptr);
    }
}

void Chip8::LoadROM(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (file.is_open()) {
        auto bytes_size = static_cast<std::size_t>(file.read(
            reinterpret_cast<char*>(memory.data()) + ENTRY_POINT,
            memory.size() - ENTRY_POINT).gcount());
        if (bytes_size > 4096 - ENTRY_POINT)
            throw std::runtime_error("chip8: Failed to fit ROM in memory");
    } else throw std::runtime_error("chip8: Failed to open file");
}

void Chip8::LoadFont() {
    std::copy(std::begin(fontset), std::end(fontset),
              std::begin(memory) + FONT_ADDRESS);
}

void Chip8::Reset() {
    PC = ENTRY_POINT;
    I = OP = DT = ST = 0x0000;
    memory.fill(0x00);
    pixels.fill(0x00);
    V.fill(0x0000);
    stack.clear();

    LoadROM(filename);
    LoadFont();
}
