#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>

struct Keyboard {
public:
    std::array<bool, 16> keys = { };

    inline void Reset() { keys.fill(false); }
    inline bool GetKey(int idx) const { return keys[idx]; }

    void SetKey(char key) {
        auto res = KeyMap.find(key);
        if (res != KeyMap.end())
            Reset(), keys[res->second] = true;
    }

    bool GetKeyFromMap(char key) const {
        auto res = KeyMap.find(key);
        return (res != KeyMap.end() ? keys[res->second] : false);
    }


private:
    const std::unordered_map<int, std::uint8_t> KeyMap {
        {'1', 0x1}, {'2', 0x2}, {'3', 0x3}, {'4', 0xc},
        {'q', 0x4}, {'w', 0x5}, {'e', 0x6}, {'r', 0xd},
        {'a', 0x7}, {'s', 0x8}, {'d', 0x9}, {'f', 0xe},
        {'z', 0xa}, {'x', 0x0}, {'c', 0xb}, {'v', 0xf}
    };
};
