# CLAUDE.md — melodybox

Offline ESP32-classic MP3 player firmware (originally scaffolded from gobuild's
`iot` preset).

**Not a Go platform service** — C++/Arduino/PlatformIO firmware. The platform
clean-arch template, gobuild Go presets, DI, domains, the kuery shared-pkg rule,
and the code-review-graph MCP DO NOT apply here. Reusable firmware code belongs
in the shared **kuino** library (imported via `lib_deps`), not copied into this
repo. IDE clang shows false `Arduino.h not found` / undeclared errors (no ESP32
toolchain include paths) — trust only `pio run`.

## What it is
A standalone, offline MP3 player: no WiFi. Tracks live on the SD card of a
YX6300 serial MP3 module; 5 buttons drive play/pause, next, prev, and volume;
an SSD1306 OLED shows the now-playing screen.

## Stack / hardware
- ESP32 classic (WROOM-32, board `esp32dev`, `huge_app.csv` partitions for the
  VN unifont), Arduino framework via PlatformIO
- SSD1306 128x64 OLED on HW I2C (SDA 21 / SCL 22)
- YX6300 MP3 module on UART2 (Serial2, RX 16 / TX 17, 9600 baud)
- 5 push buttons (active-low, `INPUT_PULLUP`)
- Shared lib: `github.com/vukyn/kuino` — mp3 + display + button modules only.
  NO wifi / httpjson (this device is offline).

## Build / flash
```bash
cp include/config.h.example include/config.h   # then adjust
pio run
pio run -t upload
pio device monitor
```

## config.h
Device wiring (pins, `START_VOLUME`, `TRACK_COUNT`) lives in `include/config.h`
— gitignored. Copy from `include/config.h.example`. No secrets (offline device),
but the file stays gitignored so per-device wiring never lands in the repo.
