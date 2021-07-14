#include "Chip8.hpp"

void Chip8::Cycle() {
    
    if (!paused || step) {
        OP = memory[PC] << 8 | memory[PC+1]; PC += 2;
        auto op_ptr = std::function<void()>();
        for (auto mask: {0xf0ff, 0xf00f, 0xf000}) {
            if ((op_ptr = Dispatch[(OP & mask)]) != nullptr) {
                std::invoke(op_ptr);
                break;
            }
        }
        step = false;
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
    memory.fill(0x00);
    stack.clear();
    V.fill(0x0000);

    PC     = ENTRY_POINT;
    I      = 0x0000;
    OP     = 0x0000;
    DT     = 0x0000;
    ST     = 0x0000;

    pixels.fill(false);

    LoadROM(filename);
    LoadFont();
}
