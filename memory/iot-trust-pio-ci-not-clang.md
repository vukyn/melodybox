---
name: iot-trust-pio-ci-not-clang
description: "kuino/firmware — IDE clang shows false Arduino.h/undeclared errors; trust pio ci [SUCCESS] not clang"
metadata: 
  node_type: memory
  type: feedback
---

For kuino + any PlatformIO/Arduino firmware repo, the IDE's clang diagnostics emit false `'Arduino.h' file not found`, `undeclared WiFi/millis/String/JsonDocument`, etc. — clang has no PlatformIO ESP32 toolchain include paths.

**Why:** Real compilation goes through `pio` (xtensa-esp toolchain + LDF-resolved libs), not the IDE's clang. clang can't see Arduino/ESP32/U8g2/ArduinoJson headers.

**How to apply:** Trust `pio ci <sketch> -l . -b esp32-s3-devkitc-1` ending in `[SUCCESS]` (or `pio run`) as the compile gate — ignore clang's file-not-found/undeclared noise. Direct C++ analog of [[gopls-stale-diagnostics-multirepo]] (trust `go build` not gopls). See [[rainybox-onboarded]], [[rainybox-flash-workflow]].
