# DECISIONS.md

Design and architecture decisions for the BMS Door Simulator, with context
and rationale. Entries are ordered by project phase.

## 1. Simulation scope: local network only, no Siemens cloud connection

**Date:** Initial setup
**Status:** Accepted

**Context:** The Siemens Building X Operations API documentation
(https://developer.siemens.com/building-x-openness/api/building-operations/quickstart.html)
is available as a real-world reference, but the simulator must run standalone
on the local network without any dependency on Siemens infrastructure or
credentials.

**Decision:** The device simulates only the *door-side* of the interaction:
it exposes the API surface a BMS client would talk to (commanding points,
reading point state and values) over local HTTP. There is no OAuth client,
no outbound HTTPS to `buildingx.siemens.com`, and no token handling.

**Consequences:** No credentials are needed to run the simulator. The
interface intentionally mirrors the Building X Operations API shape
(JSON:API-style paths and payloads) so a real BMS integration client can be
pointed at the simulator with minimal changes.

## 2. Platform: PlatformIO + Arduino framework on ESP32-S3

**Date:** Initial setup
**Status:** Accepted

**Context:** The target hardware is an ESP32-S3. The choice is between
ESP-IDF and Arduino/PlatformIO.

**Decision:** Arduino framework managed by PlatformIO
(`platformio.ini`, board `esp32-s3-devkitc-1`), with
`bblanchon/ArduinoJson@^7` as the only external library.

**Consequences:** Fast development, a familiar HTTP stack (`WebServer`),
simple build/flash workflow (`pio run -t upload`). ArduinoJson v7 is used
via the `JsonDocument` API.

## 3. API shape: mirror the Building X Operations API locally

**Date:** Initial setup
**Status:** Accepted

**Context:** The Building X quickstart defines `PATCH` on a point for
commanding (`data.attributes.pointValue.value`), `GET` on a point for its
state, and `GET .../values` for timestamped point values.

**Decision:** The simulator serves the same paths locally:

- `PATCH /api/openness/operations/partitions/<PARTITION>/points/<POINT>`
  with body `{"data":{"attributes":{"pointValue":{"value":"open|close|hold"}}}}`
  → `202`
- `GET .../points/<POINT>` → current state and position (0–100)
- `GET .../points/<POINT>/values` → timestamped transition history

Content type is `application/vnd.api+json`, and errors are returned in a
JSON:API `errors` array. IDs (`BMS_PARTITION_ID`, `DOOR_POINT_ID`) are
placeholder constants in `include/config.h`.

**Consequences:** Commands return `202`, not `200`, matching the Building X
semantics that a command is accepted but the point state is not reflected
immediately. Unsupported command values are rejected with `400`.

## 4. Timing model: command-apply delay plus physical travel time

**Date:** Initial setup
**Status:** Accepted

**Context:** The Building X documentation notes a delay between accepting a
command and the point state reflecting it. A real door also takes physical
time to move, and can be held at an intermediate position.

**Decision:** Two delays in `include/config.h`:

- `COMMAND_APPLY_DELAY_MS` (1000 ms): a command is queued and only applied
  to the state machine after this delay.
- `DOOR_TRAVEL_TIME_MS` (5000 ms): full travel time between closed and
  fully open.

States: `closed`, `opening`, `open`, `closing`, `hold`, `unknown`. The door
tracks a continuous position (0 = closed, 100 = open) so `hold` freezes at
the current position and a later `open`/`close` resumes travel from there,
proportionally to the remaining distance. A command that matches the
current state or direction is accepted as a no-op.

**Consequences:** Clients observe realistic intermediate states
(`opening`/`closing`) before the final state, and a `hold` behaves like a
real door, not an instant reset.

## 5. State machine decoupled from the ESP32/Wi-Fi layer

**Date:** Initial setup
**Status:** Accepted

**Context:** The door logic should be verifiable without flashing hardware.

**Decision:** `DoorSim` (`include/DoorSim.h`, `src/DoorSim.cpp`) is a plain
C++ class depending only on `Arduino.h` basics (`millis`, `map`,
`constrain`, `String`). All Wi-Fi and HTTP handling lives separately in
`main.cpp` and `DoorApiServer`. The native test in `test/` compiles
`DoorSim.cpp` directly against small Arduino stubs.

**Consequences:** The state machine is unit-testable on the host in seconds
(`g++`), while the firmware build (`pio run`) stays clean. A 64-bit
`millis64()` counter (`include/millis64.h`) is used for history timestamps
to avoid the 49-day wrap of Arduino's 32-bit `millis()`.

## 6. History buffer: fixed-size ring, newest wins

**Date:** Initial setup
**Status:** Accepted

**Context:** `GET .../values` should report state transitions since boot,
but the ESP32-S3 has limited RAM.

**Decision:** A fixed-size array of `DOOR_HISTORY_MAX` (20) transitions.
When full, entries shift down and the newest replaces the oldest. Timestamps
are milliseconds since boot (rendered as RFC3339-like strings in the API).

**Consequences:** Bounded memory, no dynamic allocation in the hot path.
The history only covers the last 20 transitions; longer history would need
persistent storage (deliberately out of scope).

## 7. Credential management: environment file for secrets

**Date:** 2026-10-14
**Status:** Accepted

**Context:** Per project requirements, all secrets including WiFi name and
password must be kept in a local environment file rather than hardcoded in
source files. This prevents accidental credential commits and enables
easier configuration across different environments.

**Decision:** Use a `.env` file in the project root with simple key-value
format. Create a parser utility (`EnvParser`) that reads this file at startup
and overrides default credentials from `config.h`. The `.env` file is added
to `.gitignore`; an example template `.env.example` is committed for reference.

**Consequences:** Improved security by keeping credentials out of version
control. The parser must handle missing files gracefully by falling back to
`config.h` defaults. Requires one-time manual file creation for new users.

## 8. Status visualization: LED indicators for door state

**Date:** 2026-10-14
**Status:** Accepted

**Context:** Physical feedback for door status is required. Per requirements,
LED indicators should show: GREEN for OPEN, YELLOW for OPENING/CLOSED,
RED for CLOSED.

**Decision:** Connect three LEDs to GPIO pins 1 (GREEN), 2 (YELLOW), 3 (RED)
using ACTIVE HIGH logic. Create a `DoorLedIndicator` class that manages these
LEDs and updates them based on the current `DoorState`. Only one LED is
active at a time. The LED manager is initialized in `setup()` and updated
in `loop()` or on state changes.

**Consequences:** Requires hardware assembly (LEDs + resistors + breadboard).
The door state machine (`DoorSim`) remains decoupled from LED logic, maintaining
testability. GPIO pins 1, 2, 3 were chosen as they are typically available on
ESP32-S3 and don't conflict with core functionality.

## 9. Testing interface: separate Python project

**Date:** 2026-10-14
**Status:** Accepted

**Context:** A separate project is needed to test the simulator by sending
commands and verifying responses. This enables automated testing without
modifying the embedded firmware.

**Decision:** Create a Python project in the `/tests` subfolder that uses
the `requests` library to connect to the simulator's HTTP API. The project
includes: a client class (`door_client.py`), test scripts (`test_simulator.py`),
example usage (`example_usage.py`), and documentation (`README.md`).

**Consequences:** Enables easy testing and integration with CI/CD pipelines.
The Python project is separate from the firmware, allowing independent updates.
Requires Python 3.8+ and the `requests` library.
