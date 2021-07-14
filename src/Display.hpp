#pragma once

#include "Chip8.hpp"
#include "Disassembler.hpp"

#include <vector>
#include <string>
#include <array>
#include "ncurses.h"

class Display final {

public:
     Display(Chip8& c8);
    ~Display();

    void Refresh() const;

private:
    Chip8& c8;

    WINDOW*      left;
    WINDOW*      main;
    WINDOW*      right;

    std::vector<std::string> assembly;

    void UserInput() const;

    void LeftPannel() const;
    void MainPannel() const;
    void RightPannel() const;
};
