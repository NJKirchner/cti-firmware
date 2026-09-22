#include "cti/platform.h"

#include <esp_adc/adc_oneshot.h>
#include <driver/gpio.h>
#include <esp_rom_sys.h>

#include <cstdio>
#include <cstring>
#include <stdarg.h>

namespace CTI {

static adc_oneshot_unit_handle_t adc_unit = nullptr;
static bool gpio_output[GPIO_NUM_MAX] = {};

static LVBlock availableGPIOs = { 44, {
    0x00, 0x00, 0x00, 0x28,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D,
    0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27
}};

LVBlock* PlatformDigital::Available() { return &availableGPIOs; }

void PlatformDigital::SetOutput(ChanIndex channel, bool value) {
    gpio_set_level(static_cast<gpio_num_t>(channel), value);
}

void PlatformDigital::SetDirection(ChanIndex channel, bool output) {
    gpio_set_direction(static_cast<gpio_num_t>(channel),
        output ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT);
    if (channel >= 0 && channel < GPIO_NUM_MAX) gpio_output[channel] = output;
}

void PlatformDigital::SetPull(ChanIndex channel, PullDirection dir) {
    gpio_set_pull_mode(static_cast<gpio_num_t>(channel),
        (dir == Up) ? GPIO_PULLUP_ONLY :
        (dir == Down) ? GPIO_PULLDOWN_ONLY : GPIO_FLOATING);
}

void PlatformDigital::GetValue(ChanIndex channel, bool* value) {
    *value = gpio_get_level(static_cast<gpio_num_t>(channel)) != 0;
}

void PlatformDigital::GetDirection(ChanIndex channel, bool* output) {
    *output = channel >= 0 && channel < GPIO_NUM_MAX && gpio_output[channel];
}

void PlatformDigital::GetPull(ChanIndex, PullDirection* dir) {
    *dir = None;
}

static LVBlock availableAIs = { 10, { 0x00, 0x00, 0x00, 0x03, 0x00, 0x32, 0x01, 0x33, 0x02, 0x34 } };

LVBlock* PlatformAnalog::Available() { return &availableAIs; }

void PlatformAnalog::EnableInput(ChanIndex channel) {
    if (!adc_unit) {
        adc_oneshot_unit_init_cfg_t unit_config = {};
        unit_config.unit_id = ADC_UNIT_1;
        adc_oneshot_new_unit(&unit_config, &adc_unit);
    }
    adc_oneshot_chan_cfg_t channel_config = {};
    channel_config.atten = ADC_ATTEN_DB_12;
    channel_config.bitwidth = ADC_BITWIDTH_12;
    adc_oneshot_config_channel(adc_unit, static_cast<adc_channel_t>(channel), &channel_config);
}

void PlatformAnalog::GetInput(ChanIndex channel, uint16_t* value) {
    if (!adc_unit) EnableInput(channel);
    int raw = 0;
    adc_oneshot_read(adc_unit, static_cast<adc_channel_t>(channel), &raw);
    *value = static_cast<uint16_t>(raw);
}

int PlatformIO::_getchar_timeout_us(uint32_t timeout_us) {
    int c = getchar();
    return c == EOF ? -1 : c;
}

void PlatformIO::InitStatusLED() {}

void PlatformIO::_statusLED(bool val) {
    gpio_set_level(GPIO_NUM_25, val ? 1 : 0);
}

} // namespace CTI
