#include "visa/visa_core.h"
#include "display/hub75.h"
#include "display/hub75_frame.h"
#include "display/ws2811.h"
#include "fake_sdk.h"

#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace {
struct EndInput {};
int pop(CTI::SCPI::ScpiParser& parser) {
    int16_t code;
    char* text;
    parser.dequeueError(&code, &text);
    return code;
}
}

void visaReceiveTests() {
    using namespace CTI;
    Visa::Visa visa;
    Hub75::registerCommands(&visa);
    WS2811::registerCommands(&visa);
    visa.parser.finalize();
    auto send = [&](const std::string& command) {
        assert(visa.parser.bufferInput(command.data(), static_cast<int>(command.size())) == static_cast<int>(command.size()));
        assert(pop(visa.parser) == 0);
    };
    send("DISP:HUB75:DIM 64,64\nDISP:HUB75:FORM RGB332\nDISP:HUB75:START\n");
    const void* before = Fake::source;
    std::vector<uint8_t> image(4096);
    for (size_t i = 0; i < image.size(); ++i) image[i] = static_cast<uint8_t>(i);
    std::string stream;
    for (size_t offset = 0; offset < image.size(); offset += 2000) {
        size_t count = std::min<size_t>(2000, image.size() - offset);
        std::string length = std::to_string(count);
        stream += "DISP:HUB75:DATA " + std::to_string(offset) + ",#" +
            std::to_string(length.size()) + length +
            std::string(reinterpret_cast<const char*>(image.data() + offset), count) + "\n";
    }
    stream += "DISP:HUB75:SHOW\nDISP:HUB75:STAT?\n";
    size_t position = 0, reads = 0;
    gPlatform.IO.output.clear();
    gPlatform.IO.input = [&](char* buffer, int capacity, uint32_t timeout) {
        assert(capacity == 64 && timeout == 20000);
        if (position == stream.size()) throw EndInput{};
        Fake::completeFrame();
        size_t count = std::min<size_t>(capacity, stream.size() - position);
        std::memcpy(buffer, stream.data() + position, count);
        position += count;
        ++reads;
        return static_cast<int>(count);
    };
    try { visa.MainLoop(); } catch (const EndInput&) {}
    assert(reads == (stream.size() + 63) / 64);
    assert(pop(visa.parser) == 0 && !visa.parser.hasPendingInput());
    Fake::completeFrame();
    Hub75::Configuration config;
    config.width = config.height = 64;
    config.format = Hub75::PixelFormat::RGB332;
    std::vector<uint32_t> expected(config.scanWords());
    Hub75::pack(image.data(), 25, expected.data(), config);
    assert(Fake::source != before);
    assert(std::equal(expected.begin(), expected.end(), static_cast<const uint32_t*>(Fake::source)));
    assert(Hub75::stop());

    // Short queries must run immediately; a batch can contain several commands.
    unsigned step = 0;
    gPlatform.IO.output.clear();
    gPlatform.IO.input = [&](char* buffer, int capacity, uint32_t timeout) {
        assert(capacity == 64 && timeout == 20000);
        if (step++ == 0) {
            std::string text = "DISP:HUB75:FORM?\nDISP:HUB75:DIM?\n";
            std::memcpy(buffer, text.data(), text.size());
            return static_cast<int>(text.size());
        }
        assert(gPlatform.IO.output == "RGB332\n64,64\n");
        throw EndInput{};
    };
    try { visa.MainLoop(); } catch (const EndInput&) {}

    // Interrupted binary messages time out only after 100 empty reads, then recover.
    step = 0;
    gPlatform.IO.output.clear();
    gPlatform.IO.input = [&](char* buffer, int, uint32_t) {
        unsigned current = step++;
        if (current == 0) {
            std::string text = "DISP:HUB75:DATA 0,#296abc";
            std::memcpy(buffer, text.data(), text.size());
            return static_cast<int>(text.size());
        }
        if (current < 100) {
            assert(visa.parser.hasPendingInput());
            return 0;
        }
        if (current == 100) return 0;
        if (current == 101) {
            assert(!visa.parser.hasPendingInput() && pop(visa.parser) == -161);
            std::string text = "DISP:HUB75:FORM?\n";
            std::memcpy(buffer, text.data(), text.size());
            return static_cast<int>(text.size());
        }
        assert(gPlatform.IO.output == "RGB332\n" && pop(visa.parser) == 0);
        throw EndInput{};
    };
    try { visa.MainLoop(); } catch (const EndInput&) {}

    // Each arriving fragment restarts the idle timeout, even across a slow command.
    step = 0;
    gPlatform.IO.output.clear();
    const std::string slowQuery = "DISP:HUB75:FORM?\n";
    gPlatform.IO.input = [&](char* buffer, int, uint32_t) {
        unsigned current = step++;
        if (current >= slowQuery.size() * 100) throw EndInput{};
        if (current % 100 != 0) return 0;
        buffer[0] = slowQuery[current / 100];
        return 1;
    };
    try { visa.MainLoop(); } catch (const EndInput&) {}
    assert(pop(visa.parser) == 0 && gPlatform.IO.output == "RGB332\n");

    step = 0;
    gPlatform.IO.input = [&](char*, int, uint32_t) {
        if (step++ == 0) return -1;
        assert(pop(visa.parser) == -200);
        throw EndInput{};
    };
    try { visa.MainLoop(); } catch (const EndInput&) {}
    gPlatform.IO.input = {};
    std::cout << "Receive: 64-byte batches, binary frame packing, immediate short queries and timeout recovery passed\n";
}
