#include "Chip8.hpp"
#include "Display.hpp"
#include "Disassembler.hpp"

#include <iostream>
#include <chrono>
#include <stdexcept>
#include <unistd.h>

using ms =  std::chrono::duration<float, std::chrono::milliseconds::period>;
using hr_clock = std::chrono::high_resolution_clock;

int main(int argc, char* argv[]) {

    if (argc < 2) throw std::runtime_error("chip8: no input file");

    Chip8   chip8(argv[1]);
    Display display(chip8);

    auto last_tick = hr_clock::now();
    auto last_tick_timer = hr_clock::now();

    while (!chip8.quit) {
        auto tick = hr_clock::now();

        float delta       = ms(tick - last_tick).count();
        float delta_timer = ms(tick - last_tick_timer).count();


        if (delta > 1/chip8.cycle_speed*1000) // 150hz default
            chip8.Cycle(), display.Refresh(), last_tick = tick;

        if (delta_timer > 1/60.f*1000) // 60hz
            chip8.UpdateTimers(), last_tick_timer = tick;
    }

    return 0;
}
