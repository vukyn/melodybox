# melodybox

Offline ESP32-classic MP3 player. No WiFi — reads tracks from the SD card of a
YX6300 serial MP3 module, driven by 5 push buttons, with an SSD1306 OLED for the
now-playing display. Consumes the shared
[kuino](https://github.com/vukyn/kuino) library (mp3 / display / button helpers).

## Hardware
- ESP32 classic (WROOM-32, board `esp32dev`, 4MB flash, no PSRAM)
- SSD1306 0.96" 128x64 I2C OLED
- YX6300 / YX5300 (Catalex) serial MP3 module + micro-SD card of tracks
- 5 push buttons (active-low to GND, internal pull-ups)

## Wiring
| Signal            | ESP32 GPIO | Notes                              |
|-------------------|------------|------------------------------------|
| OLED SDA          | 21         | HW-default I2C                     |
| OLED SCL          | 22         | HW-default I2C                     |
| MP3 RX (from mod) | 16         | ESP32 receives on this pin         |
| MP3 TX (to mod)   | 17         | ESP32 transmits on this pin        |
| Button PLAY/PAUSE | 32         | play / pause toggle                |
| Button NEXT       | 33         | next track                         |
| Button PREV       | 25         | previous track                     |
| Button VOL+       | 26         | volume up                          |
| Button VOL-       | 27         | volume down                        |

YX6300 UART is on HardwareSerial2 (UART2) at 9600 baud. Wire ESP32 RX (GPIO16)
to the module's TX, and ESP32 TX (GPIO17) to the module's RX. Buttons connect
their GPIO to GND when pressed (`INPUT_PULLUP`, active-low). Set `TRACK_COUNT` in
`config.h` to the number of files on the card so next/prev wrap correctly.

## Build / flash
```bash
cp include/config.h.example include/config.h   # then adjust pins / TRACK_COUNT
pio run                 # build (pulls kuino from git + U8g2)
pio run -t upload       # flash
pio device monitor      # serial log (115200)
```

## kuino
Firmware helpers (mp3, display, button — no wifi/httpjson here, this device is
offline) live in kuino and are pinned by tag in `platformio.ini`
(`kuino.git#vX.Y.Z`). Bump the pin to adopt a new kuino release.
