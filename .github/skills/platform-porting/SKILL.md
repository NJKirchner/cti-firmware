---
name: platform-porting
description: Add a new MCU family or board while preserving existing CTI firmware targets, build selection, and SCPI behavior.
---

# CTI platform-porting skill

Use this guidance when adding an MCU family or board (for example ESP32) to CTI firmware. The goal is a separate, validated target—not a broad refactor of the working RP2040, RP2350, or AVR paths.

## Non-negotiable invariants

- Add a sibling platform implementation under `platform/<family>/`; do not rewrite an existing family to make it generic.
- Keep shared SCPI, VISA, command parsing, and application logic in `source/` and `include/`. Platform code owns hardware and toolchain boundaries.
- Keep board-specific pin maps, LEDs, boot behavior, and networking in `platform/<family>/boards/<board>/`.
- Make top-level build changes only for platform dispatch, target registration, or shared integration that is required by the new target.
- Preserve existing target names, artifact names, protocol bytes, and behavior. A new target must not change what an existing target builds.
- Never assume a familiar peripheral has the same channel numbering, clock, interrupt, reset, or electrical behavior on another MCU.

These rules reflect the RP2350 port: it was added beside RP2040 because PWM clocks, ADC temperature selection, board identity, and peripheral details were not interchangeable. They also reflect the Uno port: constrained memory and the Optiboot/DTR reset window required target-local buffering and host timing guidance rather than changes to the shared protocol.

## Before editing

1. Identify the exact MCU, SDK/toolchain, board/module revisions, bootloader, and intended artifact (`UF2`, `HEX`, `BIN`, etc.).
2. Read `include/cti/platform.h` and related platform headers to inventory the contract: communication, GPIO, ADC, PWM, UART, I2C, SPI, timers, unique ID, status LED, and board identity.
3. Trace how the top-level `CMakeLists.txt` selects a platform and how an existing sibling registers sources, compile definitions, SDK libraries, and board files.
4. Choose the nearest working reference by behavior, not by brand. RP2040/RP2350 are useful for Pico SDK structure; AVR is useful for small-memory and bootloader constraints.
5. Record the baseline build commands and artifact names for at least one existing target before changing files.

## Recommended layout

```text
platform/<family>/
  CMakeLists.txt                 # only if the family needs one
  <family>.inc.cmake             # family/toolchain integration when applicable
  platform/                      # family-level drivers and platform contract
  boards/<board>/                # board pin map, init, LED, optional network files
```

Keep one source file per hardware concern when that matches the existing family. Prefer a narrow family implementation over preprocessor branches scattered through shared code. Add a new board directory when pinout, LED wiring, boot mode, or module support differs materially.

## Port in layers

Implement and test in this order:

1. Build selection and minimal startup.
2. Host transport and serial initialization.
3. Board identity (`*IDN?`), reset behavior, and status LED.
4. GPIO and digital I/O.
5. ADC and temperature reporting.
6. PWM, deriving frequency from the actual system/peripheral clock.
7. UART, I2C, and SPI with the MCU's real peripheral and pin-mux rules.
8. Timers, unique ID, optional network support, and packaging/install output.

Do not claim parity because a firmware image links. Every implemented command must either work on the target or be explicitly unsupported using the repository's normal error behavior.

## Protocol and host behavior

Treat the Pico response format and SCPI framing as the compatibility contract:

- Preserve line endings, binary block layout, byte order, and response sequencing.
- Test exact command bytes, including `*IDN?\n`, not only interactive terminal input.
- Exercise unknown headers, blank frames, repeated commands, and binary payloads containing `0x0A` so the parser cannot desynchronize.
- Check startup output. A bootloader must not emit bytes that look like a response or error.
- Document reset-on-open behavior. Uno/Optiboot resets when DTR changes, so hosts should keep the port open and wait at least two seconds after opening, or avoid toggling DTR. Treat this as a transport/boot constraint, not a reason to alter SCPI parsing.
- For USB-native or Wi-Fi boards, define when the transport is ready and make the host-visible behavior explicit.

## Build isolation

- Add only the new platform/board to the build graph; do not change existing platform source lists unless shared registration requires it.
- Keep separate build directories per target (`build\\pico`, `build\\pico2`, `build\\<new-board>`). Never use a stale cache to infer support.
- Capture exact configure and build commands in the family README, including required SDK/toolchain versions and expected output paths.
- Keep generated firmware in the repository's established artifact location; do not commit large intermediate build trees unless the project already does so.

## Validation gate

Before merging a new family or board:

- Clean-configure and build every new board variant.
- Regression-build at least one RP2040 and one RP2350 target; include AVR when the change touches shared protocol or build dispatch.
- Verify identity, startup/reset timing, GPIO, ADC/temperature, PWM, serial buses, timers, status LED, and unique ID on hardware where available.
- Compare representative SCPI responses byte-for-byte with the canonical implementation, including binary responses.
- Inspect the diff for accidental edits to existing platform directories and confirm old artifact names are unchanged.
- Record hardware-only limitations (bootloader delay, DTR reset, pin conflicts, ADC restrictions, wireless LED control) beside the build instructions.

## ESP32 starting point

For ESP32, first choose the exact family and framework (ESP-IDF or the repository's supported Arduino integration), then add `platform/esp32/` and one board directory per module. Resolve GPIO matrix/pin-strap restrictions, ADC attenuation and calibration, LEDC PWM clock behavior, UART/I2C/SPI controller assignment, flash/partition layout, and reset/log output before attempting feature parity. Keep Wi-Fi/BLE and FreeRTOS integration board-local until the core CTI transport and command behavior pass the validation gate.

## Completion checklist

- [ ] New family is a sibling platform; existing targets are not generalized or rewritten.
- [ ] Board files contain pin, LED, boot, identity, and module-specific behavior.
- [ ] All platform contract surfaces are implemented or explicitly unsupported.
- [ ] Exact SCPI framing and response bytes match the compatibility contract.
- [ ] New and regression builds are clean from separate build directories.
- [ ] Hardware caveats and exact artifact names are documented.
