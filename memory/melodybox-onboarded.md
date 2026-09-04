---
name: melodybox-onboarded
description: melodybox = offline ESP32-classic MP3 player (YX6300+OLED+buttons) consuming kuino; power/SD gotchas
metadata: 
  node_type: memory
  type: project
---

`melodybox/` — offline ESP32-classic (WROOM-32, `esp32dev`, huge_app) MP3 player firmware. Scaffolded from gobuild `iot` preset, consumes **kuino** (mp3+display+button, NO wifi/httpjson — offline). Public repo `github.com/vukyn/melodybox`. C++/PlatformIO; `include/config.h` gitignored; `demo/melodybox-player.html` feature doc. Onboarded 2026-07-06.

**Hardware**: SSD1306 128×64 OLED (HW I2C SDA21/SCL22), YX6300/YX5300 serial MP3 (UART2 Serial2, RX16/TX17, 9600), 5 buttons INPUT_PULLUP→GND (play/pause32, next33, prev25, vol+26, vol−27). Serial logs 115200. VN unifont. TESTED OK on hardware 2026-07-06.

**OLED swap gotchas**: SSD1306 0.96" I2C boards vary — (1) **pin header order differs** (`GND VCC SCL SDA` vs `VCC GND SCL SDA`); wire by the PRINTED label not position (swap = dead screen; hit 2026-07-06). (2) address 0x3C default, some 0x3D → `oled.setI2CAddress(0x3D<<1)`. (3) "IIC/SPI combo" boards have a back jumper selecting mode. Debug w/ I2C scanner (Wire.begin(21,22)). Same-res 1.3" SH1106 = swap constructor only.

**POWER gotcha (root cause of SD inserted↔removed flip-flop)**: YX6300 VCC MUST be **VIN (~5V; VIN=USB-5V via diode ≈4.6–4.7V when USB-powered)** — 3V3 under-powers its SD+DAC → brownout → SD re-init loop. OLED VCC = **3V3** (low current + matches ESP32 3.3V I2C logic level; 5V would pull SDA/SCL out of spec). **Common GND** all 3. Weak USB port / cheap-long cable also browns out (SD init spike) → use good cable, cột thẳng Mac. Fixed hardware-side by moving YX6300 to VIN.

**SD cold-boot gotcha** (fixed melodybox #8): cắm nguồn (cold) → ESP32+YX6300 boot cùng lúc, module cần ~1-2s mount SD → query lúc 500ms timeout → "No SD card"; flash lại "fix" giả vì module vẫn powered qua upload (SD đã mount). Fix: settle 1500ms + 5 boot tries + `loop()` re-detect mỗi 2s khi `!sdOk` (self-heal, ko cần reflash). Cùng pattern [[rainybox-flash-workflow]] cold-start.

**Firmware fixes for the same flip-flop** (belt-and-suspenders): kuino v0.3.1 `readFrame` validates frame header `FF 06` (rejects misaligned/garbage frames → no phantom cmd); melodybox de-dups SD state (`sdPresent` guard, log only real transitions). See [[kuino-manifest-version-in-tagged-commit]] (mp3 read API query/poll), [[iot-trust-pio-ci-not-clang]].

**YX5300 protocol limits**: NO filename listing — only track COUNT (query 0x48), current index (0x4C), status (0x42); events SD-in 0x3A / SD-out 0x3B / track-finished 0x3D. **Volume 0–30** fixed by chip firmware (DFPlayer family), not dB; >30 ignored (kuino clamps). Louder = external amp (PAM8403) or higher-dB speaker.
