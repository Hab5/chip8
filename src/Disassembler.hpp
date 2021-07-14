#pragma once

#include <vector>
#include <string>
#include <array>
#include <exception>
#include <fstream>
#include <unordered_map>
#include <cstring>

#define N(bytes)    ( bytes & 0x000f)        // ...N
#define NN(bytes)   ( bytes & 0x00ff)        // ..NN
#define NNN(bytes)  ( bytes & 0x0fff)        // .NNN
#define X(bytes)    ((bytes & 0x0f00) >> 8)  // .X..
#define Y(bytes)    ((bytes & 0x00f0) >> 4)  // ..Y.

class Disassembler final {
public:
    static std::vector<std::string> Disassemble(const std::string& filename);

private:
    Disassembler() = default;

    static std::string fmt_op(std::string fmt, std::uint16_t OP);
    static const std::unordered_map<std::uint16_t, const char*> OpMap;

};

