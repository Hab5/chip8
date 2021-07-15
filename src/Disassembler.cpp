#include "Disassembler.hpp"
#include <cstring>


std::vector<std::string> Disassembler::Disassemble(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (file.is_open()) {
        std::array<std::uint8_t, 4096> program = { };
        auto bytes_size = static_cast<std::size_t>(file.read(
            reinterpret_cast<char*>(program.data()), program.size()).gcount());
        if (bytes_size > 4096 - 0x200)
            throw std::runtime_error("Disassembler: Failed to fit ROM in memory");
        std::uint16_t OP;
        std::vector<std::string> lines;
        for (std::uint16_t PC = 0; PC < bytes_size; PC+=2) {
            OP = program[PC] << 8 | program[PC+1];
            auto op_it = std::end(OpMap);
            for (auto mask: {0xf0ff, 0xf00f, 0xf000}) {
                if ((op_it = OpMap.find(OP & mask)) != std::end(OpMap)) {
                    lines.push_back(fmt_op(op_it->second, OP));
                    break;
                }
            }
        }
        return lines;
    } else throw std::runtime_error("Disassembler: Failed to open file");
};

std::string Disassembler::fmt_op(std::string fmt, std::uint16_t OP) {
    for (auto sub: {"<reg_x>", "<reg_y>", "<addr>", "<byte>", "<nibble>"}) {
        auto pos = fmt.find(sub);
        if (pos != std::string::npos)
            fmt.replace(pos, std::strlen(sub), ([OP](auto sub) {
                char buffer[32];
                if      (std::strcmp(sub, "<reg_x>" ) == 0) sprintf(buffer, "v%x"  , X(OP)  );
                else if (std::strcmp(sub, "<reg_y>" ) == 0) sprintf(buffer, "v%x"  , Y(OP)  );
                else if (std::strcmp(sub, "<addr>"  ) == 0) sprintf(buffer, "$%04x", NNN(OP));
                else if (std::strcmp(sub, "<byte>"  ) == 0) sprintf(buffer, "$%02x", NN(OP) );
                else if (std::strcmp(sub, "<nibble>") == 0) sprintf(buffer, "$%x"  , N(OP)  );
                return std::string(buffer);
            })(sub));
    }
    return fmt;
};

const std::unordered_map<std::uint16_t, const char*>
Disassembler::OpMap = {
    {0x0000, "sys  <addr>"},
    {0x00e0, "cls  "},
    {0x00ee, "ret  "},
    {0x1000, "jp   <addr>"},
    {0x2000, "call <addr>"},
    {0x3000, "se   <reg_x>,<byte>"},
    {0x4000, "sne  <reg_x>,<byte>"},
    {0x5000, "se   <reg_x>,<reg_y>"},
    {0x6000, "ld   <reg_x>,<byte>"},
    {0x7000, "add  <reg_x>,<byte>"},
    {0x8000, "ld   <reg_x>,<reg_y>"},
    {0x8001, "or   <reg_x>,<reg_y>"},
    {0x8002, "and  <reg_x>,<reg_y>"},
    {0x8003, "xor  <reg_x>,<reg_y>"},
    {0x8004, "add  <reg_x>,<reg_y>"},
    {0x8005, "sub  <reg_x>,<reg_y>"},
    {0x8006, "shr  <reg_x>,{,<reg_y>}"},
    {0x8007, "subn <reg_x>,<reg_y>"},
    {0x800e, "shl  <reg_x>,{,<reg_y>}"},
    {0x9000, "sne  <reg_x>,<reg_y>"},
    {0xa000, "ld   I,<addr>"},
    {0xb000, "jp   v0,<addr>"},
    {0xc000, "rnd  <reg_x>,<byte>"},
    {0xd000, "drw  <reg_x>,<reg_y>,<nibble>"},
    {0xe09e, "skp  <reg_x>"},
    {0xe0a1, "sknp <reg_x>"},
    {0xf007, "ld   <reg_x>,DT"},
    {0xf00a, "ld   <reg_x>,K"},
    {0xf015, "ld   DT,<reg_x>"},
    {0xf018, "ld   ST,<reg_x>"},
    {0xf01e, "add  I,<reg_x>"},
    {0xf029, "ld   F,<reg_y>"},
    {0xf033, "ld   B,<reg_x>"},
    {0xf055, "ld   [I],<reg_x>"},
    {0xf065, "ld   <reg_x>,[I]"}
};
