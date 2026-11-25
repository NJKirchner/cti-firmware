#pragma once
#include <cstdint>
#include <cstdio>
#include <string>
#include <functional>

namespace CTI {
using ChanIndex = int8_t;
enum StatusSource { Comms, User };
struct PlatformIO {
    std::string output;
    std::function<int(char*, int, uint32_t)> input;
    int ReadStdinTimeout(char* buffer, int maxLen, uint32_t timeout) {
        return input(buffer, maxLen, timeout);
    }
    int FGetCtimeout(uint32_t timeout) {
        char byte;
        return ReadStdinTimeout(&byte, 1, timeout) == 1 ? static_cast<uint8_t>(byte) : -1;
    }
    void StatusLED(bool, StatusSource) {}
    void Print(char c) { output += c; }
    void Print(const char* text) { output += text; }
    void Print(int len, const char* text) { output.append(text, len); }
    template <typename... Args>
    void Printf(const char* format, Args... args) {
        char text[512];
        std::snprintf(text, sizeof(text), format, args...);
        output += text;
    }
};
struct Platform { PlatformIO IO; };
extern Platform gPlatform;
}
