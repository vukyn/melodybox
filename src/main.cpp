// melodybox — offline ESP32-classic MP3 player.
// SSD1306 OLED (HW I2C) + YX6300 serial MP3 module (UART2) + 5 push buttons.
// No WiFi: consumes only kuino's mp3 / display / button helpers.
#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#include <kuino/mp3.h>
#include <kuino/display.h>
#include <kuino/button.h>

#include "config.h"

// ---------------------------------------------------------------------------
// Fonts (match rainybox: VN unifont for text, small ASCII font for status).
// ---------------------------------------------------------------------------
#define FONT_VN    u8g2_font_unifont_t_vietnamese2
#define FONT_SMALL u8g2_font_6x12_tr

// ---------------------------------------------------------------------------
// Devices
// ---------------------------------------------------------------------------
U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);
kuino::mp3::YX5300 player;

// ---------------------------------------------------------------------------
// Player state (kept in sync with the module by every action)
// ---------------------------------------------------------------------------
int  trackIndex = 1;             // 1-based track number
bool playing    = false;         // start paused — user presses play
int  volume     = START_VOLUME;  // 0..30
int  marquee    = 0;             // marquee scroll offset (px)

// ---------------------------------------------------------------------------
// Per-button debounce state (one set per physical button)
// ---------------------------------------------------------------------------
struct ButtonState {
  int reading = HIGH;
  int stable = HIGH;
  unsigned long edge = 0;
};
ButtonState btnPlay, btnNext, btnPrev, btnVolUp, btnVolDn;

// ---------------------------------------------------------------------------
// Rendering
// ---------------------------------------------------------------------------
void render() {
  oled.clearBuffer();

  kuino::display::drawHeader(oled, "melodybox", FONT_VN);
  oled.drawHLine(0, 16, 128);

  kuino::display::drawScroll(oled, 40, String("Track ") + trackIndex, FONT_VN,
                             marquee);

  oled.setFont(FONT_SMALL);
  String status = String(playing ? ">= PLAY" : "|| PAUSE") + "   vol " + volume;
  oled.drawStr(0, 62, status.c_str());

  oled.sendBuffer();
}

// ---------------------------------------------------------------------------
// Actions — each mutates local state and the module together
// ---------------------------------------------------------------------------
void togglePlay() {
  playing = !playing;
  playing ? player.play() : player.pause();
}

void nextTrack() {
  trackIndex = trackIndex % TRACK_COUNT + 1;
  player.playIndex(trackIndex);
  playing = true;
  marquee = 0;
}

void prevTrack() {
  trackIndex = (trackIndex - 2 + TRACK_COUNT) % TRACK_COUNT + 1;
  player.playIndex(trackIndex);
  playing = true;
  marquee = 0;
}

void volumeUp() {
  if (volume < 30) {
    volume++;
    player.volume(volume);
  }
}

void volumeDown() {
  if (volume > 0) {
    volume--;
    player.volume(volume);
  }
}

// ---------------------------------------------------------------------------
// Input
// ---------------------------------------------------------------------------
void checkButtons() {
  if (kuino::button::pressed(BTN_PLAY, btnPlay.reading, btnPlay.stable,
                             btnPlay.edge)) {
    togglePlay();
  }
  if (kuino::button::pressed(BTN_NEXT, btnNext.reading, btnNext.stable,
                             btnNext.edge)) {
    nextTrack();
  }
  if (kuino::button::pressed(BTN_PREV, btnPrev.reading, btnPrev.stable,
                             btnPrev.edge)) {
    prevTrack();
  }
  if (kuino::button::pressed(BTN_VOLUP, btnVolUp.reading, btnVolUp.stable,
                             btnVolUp.edge)) {
    volumeUp();
  }
  if (kuino::button::pressed(BTN_VOLDN, btnVolDn.reading, btnVolDn.stable,
                             btnVolDn.edge)) {
    volumeDown();
  }
}

// ---------------------------------------------------------------------------
// Boot
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);

  Wire.begin(OLED_SDA, OLED_SCL);
  oled.begin();
  oled.setContrast(255);

  pinMode(BTN_PLAY, INPUT_PULLUP);
  pinMode(BTN_NEXT, INPUT_PULLUP);
  pinMode(BTN_PREV, INPUT_PULLUP);
  pinMode(BTN_VOLUP, INPUT_PULLUP);
  pinMode(BTN_VOLDN, INPUT_PULLUP);

  player.begin(Serial2, MP3_RX, MP3_TX);
  player.volume(volume);
  // Leave paused — the user presses play to start.
}

void loop() {
  checkButtons();
  marquee += 1;
  render();
  delay(40);  // ~25fps for a smooth marquee
}
