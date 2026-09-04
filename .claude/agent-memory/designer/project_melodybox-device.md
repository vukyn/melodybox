---
name: melodybox-device
description: melodybox = offline ESP32-classic MP3 player firmware; feature-doc mock standard + hardware truth
metadata:
  type: project
---

melodybox is an OFFLINE ESP32-classic (WROOM-32, board `esp32dev`) MP3 player firmware repo — C++/Arduino/PlatformIO, NOT a Go service. No WiFi. Consumes shared kuino lib (mp3/display/button modules only).

**Why:** it's a hardware/firmware repo like rainybox; the Go platform template, DI, Chakra UI, and code-review-graph do NOT apply.

**How to apply:** firmware feature docs follow the rainybox standard — `demo/<feature>.html` self-contained HTML (inline CSS/JS, dark theme mirrored from `rainybox/demo/rainybox-nowplaying.html`), sections: Usecase → BOM → wiring SVG → pin map → data flow → animated OLED sim. These are the deliverable (no React port, no coder handoff). See [[demo-mock-source-of-truth]].

Hardware truth (from README/config.h/main.cpp):
- SSD1306 128×64 OLED on HW I2C: SDA=GPIO21, SCL=GPIO22, addr 0x3C
- YX6300/YX5300 MP3 module on UART2 (Serial2) 9600 baud: ESP32 RX=GPIO16 ← module TX, ESP32 TX=GPIO17 → module RX (crossover)
- 5 buttons active-low INPUT_PULLUP: PLAY=32, NEXT=33, PREV=25, VOL+=26, VOL-=27
- START_VOLUME=18 (0–30), TRACK_COUNT=20; boots PAUSED; render() = drawHeader "melodybox" + HLine y16 + drawScroll "Track N" y40 + status ">= PLAY"/"|| PAUSE" + vol
- First feature-doc mock: `demo/melodybox-player.html`
