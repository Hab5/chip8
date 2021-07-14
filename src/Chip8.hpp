#pragma once

#include "Keyboard.hpp"

#include <vector>
#include <array>
#include <string>
#include <cstdint>
#include <functional>
#include <fstream>
#include <exception>
#include <iostream>
#include <unistd.h>
#include <algorithm>
#include <chrono>
#include <exception>

#define SCREEN_WIDTH  64
#define SCREEN_HEIGHT 32
#define ENTRY_POINT   0x0200
#define FONT_ADDRESS  0x0500

#define N(bytes)    ( bytes & 0x000f)        // ...N
#define NN(bytes)   ( bytes & 0x00ff)        // ..NN
#define NNN(bytes)  ( bytes & 0x0fff)        // .NNN
#define X(bytes)    ((bytes & 0x0f00) >> 8)  // .X..
#define Y(bytes)    ((bytes & 0x00f0) >> 4)  // ..Y.

#define VX V[X(OP)]
#define VY V[Y(OP)]
#define VF V[0xf]

const std::array<std::uint8_t, 80> fontset {
    0xf0, 0x90, 0x90, 0x90, 0xf0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xf0, 0x10, 0xf0, 0x80, 0xf0, // 2
    0xf0, 0x10, 0xf0, 0x10, 0xf0, // 3
    0x90, 0x90, 0xf0, 0x10, 0x10, // 4
    0xf0, 0x80, 0xf0, 0x10, 0xf0, // 5
    0xf0, 0x80, 0xf0, 0x90, 0xf0, // 6
    0xf0, 0x10, 0x20, 0x40, 0x40, // 7
    0xf0, 0x90, 0xf0, 0x90, 0xf0, // 8
    0xf0, 0x90, 0xf0, 0x10, 0xf0, // 9
    0xf0, 0x90, 0xf0, 0x90, 0x90, // a
    0xe0, 0x90, 0xe0, 0x90, 0xe0, // b
    0xf0, 0x80, 0x80, 0x80, 0xf0, // c
    0xe0, 0x90, 0x90, 0x90, 0xe0, // d
    0xf0, 0x80, 0xf0, 0x80, 0xf0, // e
    0xf0, 0x80, 0xf0, 0x80, 0x80  // f
};

class Display;

class Chip8 final {

friend Display;

public:
    Chip8(const std::string& filename): filename(filename) {
        LoadROM(filename);
        LoadFont();
    }


    void Cycle();  // One instruction
    void Reset();  // Reset ROM execution

    inline void UpdateTimers() { DT -= (DT>0), ST -= (ST>0); }

    inline bool GetPixel(int x, int y) const { return pixels[x + ( y * SCREEN_WIDTH)]; }

    float cycle_speed  = 150.f; // in Hertz
    bool  quit         = false;
    bool  paused       = false;
    bool  step         = false;

private:
    std::string filename;

    std::array<std::uint8_t, 4096> memory  = { };
    std::vector<std::uint16_t>     stack   = { };
    std::array<std::uint8_t,   16> V       = { };         // Registers
    std::uint16_t                  PC      = ENTRY_POINT; // Program Counter
    std::uint16_t                  I       = 0x0000;      // Index Register
    std::uint16_t                  OP      = 0x0000;      // Current Instruction
    std::uint8_t                   DT      = 0x00;        // Delay Timer
    std::uint8_t                   ST      = 0x00;        // Sound Timer


    std::array<bool, SCREEN_WIDTH*SCREEN_HEIGHT> pixels = { };

    Keyboard hexpad;

    void LoadROM(const std::string& filename);
    void LoadFont();

    using OpHashMap =
        std::unordered_map<std::uint16_t, std::function<void()>>;

    OpHashMap Dispatch = {
        {0x0000, [this]() { void(this); }},
        {0x00e0, [this]() { pixels.fill(0x00); }},
        {0x00ee, [this]() { PC = stack.back(); stack.pop_back(); }},
        {0x1000, [this]() { PC = NNN(OP); }},
        {0x2000, [this]() { stack.push_back(PC); PC = NNN(OP); }},
        {0x3000, [this]() { if (VX == NN(OP)) PC += 0x02; }},
        {0x4000, [this]() { if (VX != NN(OP)) PC += 0x02; }},
        {0x5000, [this]() { if (VX == VY) PC += 0x02; }},
        {0x6000, [this]() { VX  = NN(OP); }},
        {0x7000, [this]() { VX += NN(OP); }},
        {0x8000, [this]() { VX  = VY; }},
        {0x8001, [this]() { VX |= VY; }},
        {0x8002, [this]() { VX &= VY; }},
        {0x8003, [this]() { VX ^= VY; }},
        {0x8004, [this]() { VF = (VX + VY > 0xff); VX = (VX + VY) & 0xff; }},
        {0x8005, [this]() { VF = (VX > VY); VX -= VY; }},
        {0x8006, [this]() { VF = (VX & 0x1); VX >>= 1; }},
        {0x8007, [this]() { VF = (VY > VX); VX = VY - VX; }},
        {0x800e, [this]() { VF = (VX & 0x80) >> 7; VX <<= 1; }},
        {0x9000, [this]() { if (VX != VY) PC += 0x02; }},
        {0xa000, [this]() { I = NNN(OP); }},
        {0xb000, [this]() { PC = V[0x00] + NNN(OP); }},
        {0xc000, [this]() { VX = ((rand() % 0xff + 1) & NN(OP)); }},
        {0xe09e, [this]() { if ( hexpad.GetKey(VX)) { PC += 0x02; hexpad.Reset(); } }},
        {0xe0a1, [this]() { if (!hexpad.GetKey(VX)) PC += 0x02; else hexpad.Reset(); }},
        {0xf007, [this]() { VX = DT; }},
        {0xf015, [this]() { DT = VX; }},
        {0xf018, [this]() { ST = VX; }},
        {0xf01e, [this]() { I += VX; }},
        {0xf029, [this]() { I = FONT_ADDRESS + (VX*5); }},
        {0xf033, [this]() { memory[I]=VX/100; memory[I+1]=(VX%100)/10; memory[I+2]=VX%10;}},
        {0xf055, [this]() { std::copy_n(V.begin(), X(OP)+1, memory.begin()+I); }},
        {0xf065, [this]() { std::copy_n(memory.begin()+I, X(OP)+1, V.begin()); }},
        {0xf00a, [this]() {
            auto i = std::distance(hexpad.keys.begin(),
                std::find(hexpad.keys.begin(), hexpad.keys.end(), 1));
            if (i != 16) VX = i; else PC -= 2; hexpad.Reset(); }},
        {0xd000, [this]() { VF = 0x00;
            for (int row = 0; row < N(OP); row++) {
                auto byte = memory[I + row];
                for (int col = 0; col < 8; col++)
                    if (byte & (0x80 >> col)) {
                        auto x = (VX+col) % SCREEN_WIDTH;
                        auto y = (VY+row) % SCREEN_HEIGHT;
                        auto idx = x + (y * SCREEN_WIDTH);
                        auto tmp = pixels[idx];
                        VF = (!(pixels[idx] ^= 0x01) && (tmp != pixels[idx]));
                    }
            }}}
    };
};
