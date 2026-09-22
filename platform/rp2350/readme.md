# RP2350 - Raspberry Pi Pico 2 and Pico 2 W

This platform supports the RP2350A-based Raspberry Pi Pico 2 and Pico 2 W
boards with USB stdio/VISA/SCPI, GPIO, ADC and temperature measurement, PWM,
UART, I2C, SPI, timers, unique ID, and the board status LED.

## Prerequisites

Use the Pico SDK and ARM toolchain versions configured near the top of the
repository `CMakeLists.txt`. Ensure CMake and Ninja are available on `PATH`.

## Raspberry Pi Pico 2

From the repository root:

```powershell
cmake -S . -B build\pico2 -G Ninja -DPICO_BOARD=pico2
cmake --build build\pico2
```

The versioned UF2 is generated in the build directory and copied to:

```text
uf2\cti_visa_rp2350_pico2_0.9.4.uf2
```

## Raspberry Pi Pico 2 W

From the repository root:

```powershell
cmake -S . -B build\pico2_w -G Ninja -DPICO_BOARD=pico2_w
cmake --build build\pico2_w
```

The versioned UF2 is generated in the build directory and copied to:

```text
uf2\cti_visa_rp2350_pico2_w_0.9.4.uf2
```

Hold **BOOTSEL** while connecting the board, then copy the matching UF2 to the
mounted `RP2350` mass-storage volume. The Pico 2 W status LED is controlled
through the CYW43 wireless chip; all other CTI functionality uses the
Pico-compatible header pinout.
