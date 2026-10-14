# MEMORY.md

Chronological record of code changes in the BMS Door Simulator project.
Newest entries last. Each entry lists the change, the files touched, and
how it was verified.

## Change log

### 1. Initial project scaffold — commit `d4bcb54`

Created a PlatformIO project from the empty repository (initial commit
`99ded51` contained only a README stub and `.gitattributes`).

**Files added:**

- `platformio.ini` — ESP32-S3 (`esp32-s3-devkitc-1`), Arduino framework,
  ArduinoJson v7 dependency.
- `include/config.h` — Wi-Fi credentials (placeholder, overridable via
  build `-D` flags), `BMS_PARTITION_ID` / `DOOR_POINT_ID` placeholder IDs,
  timing constants (`DOOR_TRAVEL_TIME_MS`, `COMMAND_APPLY_DELAY_MS`),
  `DOOR_HISTORY_MAX`.
- `include/DoorSim.h`, `src/DoorSim.cpp` — door state machine:
  states `OPEN/CLOSED/OPENING/CLOSING/HOLD/UNKNOWN`, position tracking
  (0–100), pending-command queue with apply delay, travel with hold and
  resume, fixed-size transition history.
- `include/DoorApiServer.h`, `src/DoorApiServer.cpp` — local HTTP server
  with Building X-shaped routes: `PATCH` point (commands `open`, `close`,
  `hold` → `202`; invalid value → `400`), `GET` point (state + position),
  `GET .../values` (transition history), JSON:API error bodies.
- `include/millis64.h`, `src/main.cpp` — 64-bit millisecond counter,
  Wi-Fi station join, serial logging of the assigned IP, setup/loop.
- `test/test_door_logic.cpp`, `test/stubs/{Arduino.h,ArduinoJson.h}` —
  native host unit tests for the state machine using Arduino stubs.
- `.gitignore` — excludes `.pio/` build output.
- `README.md` — overview, build/flash instructions, API reference,
  behavior notes.

**Fixes during development:**

- Moved `millis64()` from `main.cpp` into a shared header
  (`include/millis64.h`) so `DoorSim.cpp` could use it.
- Simplified the hold-freeze position calculation (removed a redundant
  ternary; clamped elapsed time before `map`).
- Made `update()` fall through to travel progression in the same tick a
  pending command applies, so travel starts without an extra loop delay.
- Test timing changed to 100 ms ticks (matching the firmware loop) and the
  halfway position expectation relaxed to 50–52 to account for the apply
  tick.

**Verified by:** `pio run` (SUCCESS, RAM 13.6%, flash 22.1%) and native
tests via
`g++ -std=c++17 -Iinclude -Itest/stubs test/test_door_logic.cpp -o door_test`
(6 scenarios, all passing).

### 2. README project structure — commit `e2e8fe9`

Added a "Project structure" section to `README.md` documenting the file
tree and the role of each file (headers in `include/`, firmware in `src/`,
native tests and Arduino stubs in `test/`).

**Files changed:** `README.md`

### 3. Documentation files — this change

Added two documentation files:

- `DECISIONS.md` — design and architecture decision log (simulation scope,
  platform choice, API shape, timing model, testability split, history
  buffer design) with context and consequences for each entry.
- `MEMORY.md` — this chronological change log, intended to be updated with
  every future code change.

**Files added:** `DECISIONS.md`, `MEMORY.md`

## Conventions to keep

- Update this file (and `DECISIONS.md` for new design decisions) with every
  code change.
- Keep `DoorSim` free of Wi-Fi/HTTP dependencies so the native tests keep
  working.
- Keep placeholder credentials/IDs in `include/config.h`; never commit
  real credentials.
- Verify changes with both `pio run` and the native test build.

### 4. Prototype development plan — this change

Created comprehensive development plan for implementing new features from
plan.txt requirements document.

**Features planned:**
- F1: Environment file support for WiFi secrets
- F2: LED status indicators (GPIO 1, 2, 3 with ACTIVE HIGH)
- F3: Python testing project in /tests folder

**Files added:**
- `docs/PROTOTYPE_DEVELOPMENT_PLAN.md` — Complete development plan with
  executive summary, confirmed features, development roadmap, testing matrix,
  risk assessment, and next steps

**Decisions documented:**
- DECISIONS.md entries 7, 8, 9 added for credential management, LED indicators,
  and Python testing interface

**User confirmations:**
- All three features confirmed for implementation
- .env file location: project root
- .env file: added to .gitignore
- LED behavior: ACTIVE HIGH (non-configurable)
- LED pins: GPIO 1 (GREEN), 2 (YELLOW), 3 (RED)
- Python project location: /tests subfolder

**Next action:** Begin implementation with F1 (Environment File Support)
