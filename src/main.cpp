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
bool sdOk       = false;         // true once the module reports a readable SD
int  trackCount = 0;             // real track count from the module (0 = unknown/empty)
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
// SD / track detection — the source of truth is the module, queried at boot
// and again whenever a card is (re)inserted. No hardcoded track count.
// ---------------------------------------------------------------------------
void queryTracks() {
  uint16_t count = 0;
  bool ok = false;
  for (int i = 0; i < 3 && !ok; i++) {  // retry: the module can be slow to answer
    if (player.queryFileCount(count)) ok = true;
    else delay(200);
  }

  if (ok) {
    sdOk = true;
    trackCount = count;
    if (count > 0)
      Serial.printf("[melodybox] SD detected: %d tracks\n", trackCount);
    else
      Serial.println("[melodybox] SD detected but no tracks");
  } else {
    sdOk = false;
    trackCount = 0;
    Serial.println("[melodybox] SD not detected");
  }

  if (trackIndex > trackCount) trackIndex = 1;
}

// ---------------------------------------------------------------------------
// Rendering
// ---------------------------------------------------------------------------
void render() {
  oled.clearBuffer();

  kuino::display::drawHeader(oled, "melodybox", FONT_VN);
  oled.drawHLine(0, 16, 128);

  // No card / empty card get a dedicated screen instead of the now-playing UI.
  if (!sdOk) {
    kuino::display::drawScroll(oled, 40, "No SD card", FONT_VN, 0);
    oled.sendBuffer();
    return;
  }
  if (trackCount == 0) {
    kuino::display::drawScroll(oled, 40, "No tracks", FONT_VN, 0);
    oled.sendBuffer();
    return;
  }

  // Title line carries the real index/total; status line shows play state + vol.
  kuino::display::drawScroll(oled, 40,
                             String("Track ") + trackIndex + "/" + trackCount,
                             FONT_VN, marquee);
  oled.setFont(FONT_SMALL);
  String status = String(playing ? ">= PLAY" : "|| PAUSE") + "   vol " + volume;
  oled.drawStr(0, 62, status.c_str());

  oled.sendBuffer();
}

// ---------------------------------------------------------------------------
// Actions — each mutates local state and the module together. Playback actions
// are no-ops until a card with tracks is present.
// ---------------------------------------------------------------------------
void togglePlay() {
  if (!sdOk || trackCount == 0) return;
  playing = !playing;
  playing ? player.play() : player.pause();
  Serial.printf("[btn] play/pause -> %s (track %d)\n",
                playing ? "PLAY" : "PAUSE", trackIndex);
}

void nextTrack() {
  if (!sdOk || trackCount == 0) return;
  trackIndex = trackIndex % trackCount + 1;
  player.playIndex(trackIndex);
  playing = true;
  marquee = 0;
  Serial.printf("[btn] next -> track %d/%d\n", trackIndex, trackCount);
}

void prevTrack() {
  if (!sdOk || trackCount == 0) return;
  trackIndex = (trackIndex - 2 + trackCount) % trackCount + 1;
  player.playIndex(trackIndex);
  playing = true;
  marquee = 0;
  Serial.printf("[btn] prev -> track %d/%d\n", trackIndex, trackCount);
}

void volumeUp() {
  if (volume < 30) {
    volume++;
    player.volume(volume);
  }
  Serial.printf("[btn] vol+ -> %d\n", volume);
}

void volumeDown() {
  if (volume > 0) {
    volume--;
    player.volume(volume);
  }
  Serial.printf("[btn] vol- -> %d\n", volume);
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
// Module events (track finished, SD card in/out). SD in/out re-detects the
// track count live so the screen reflects the current card.
// ---------------------------------------------------------------------------
void checkMp3Events() {
  uint8_t cmd;
  uint16_t param;
  if (!player.poll(cmd, param)) return;

  if (cmd == kuino::mp3::YX5300::EVT_TRACK_FINISHED) {
    Serial.printf("[mp3] track %u finished\n", param);
  } else if (cmd == kuino::mp3::YX5300::EVT_SD_INSERTED) {
    if (!sdOk) {
      Serial.println("[mp3] SD inserted");
      queryTracks();
    }
  } else if (cmd == kuino::mp3::YX5300::EVT_SD_REMOVED) {
    if (sdOk) {
      Serial.println("[mp3] SD removed");
      sdOk = false;
      trackCount = 0;
      playing = false;
    }
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
  delay(500);  // let the YX6300 boot before querying it

  Serial.println("[melodybox] booting...");
  queryTracks();  // detect SD + real track count (no hardcoded fallback)

  player.volume(volume);
  // Leave paused — the user presses play to start.
}

void loop() {
  checkButtons();
  checkMp3Events();
  marquee += 1;
  render();
  delay(40);  // ~25fps for a smooth marquee
}
