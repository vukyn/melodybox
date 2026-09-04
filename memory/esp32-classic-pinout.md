---
name: esp32-classic-pinout
description: ESP32-WROOM-32 classic dev board pinout reference (I2C/SPI/strapping/safe pins) for rainybox
metadata: 
  node_type: memory
  type: reference
---

Classic **ESP32-WROOM-32** dev board pinout (source: Last Minute Engineers "ESP32 Dev Board Pinout"). On the user's board the silkscreen `Dxx` label == GPIO number (D18=GPIO18, D19=GPIO19). Used by [[rainybox-flash-workflow]] / [[rainybox-onboarded]].

**rainybox OLED wiring on this board**: SDA=GPIO21 (D21), SCL=GPIO22 (D22) — the hardware-default I2C pins (config.h `OLED_SDA 21` / `OLED_SCL 22`). (Earlier tried GPIO18/19 which also work via remap, but settled on the defaults.)

**Key pin facts (classic ESP32):**
- Default hardware **I2C**: SDA=GPIO21, SCL=GPIO22 (but remappable to any GPIO via Wire.begin).
- Default **VSPI**: MOSI=23, MISO=19, SCK=18, CS=5. HSPI: 14/12/13/15.
- **Input-only** (no output, no pullup): GPIO34, 35, 36(VP), 39(VN). Don't use for SCL/SDA/outputs.
- **Strapping** pins (avoid or use carefully): GPIO0, 2, 5, 12(MTDI), 15.
- **Flash** (do NOT use): GPIO6–11.
- **DAC**: GPIO25, 26. **ADC1**: 32–39. **ADC2**: 0,2,4,12–15,25–27 (ADC2 unusable while WiFi on).
- **Touch**: GPIO0,2,4,12–15,27,32,33.
- Power: 3.3V, VIN(5V), GND. **EN** = reset button; **BOOT** = GPIO0 (hold for download mode).
- No native USB → serial/flash over UART bridge (CP2102/CH340); auto-reset via DTR/RTS, usually no button dance. platformio `board = esp32dev`, no `ARDUINO_USB_CDC_ON_BOOT`, 4MB flash + `huge_app.csv` partition (Vietnamese unifont needs the room).
