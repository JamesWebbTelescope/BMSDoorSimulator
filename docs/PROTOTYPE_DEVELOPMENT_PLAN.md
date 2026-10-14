# Prototype Development Plan: BMS Door Simulator - Additional Features

**Project:** BMS Door Simulator (ESP32-S3)
**Date:** 2026-10-14
**Status:** Plan Approved - Awaiting Implementation
**Base Commit:** e657506

---

## 📌 Executive Summary

### Overview
This plan extends the existing BMS Door Simulator (ESP32-S3) with three critical features from the requirements document:
1. **Environment File for Secrets** - Secure WiFi credential management
2. **LED Status Indicators** - Physical door status visualization
3. **Python Testing Interface** - Automated testing via HTTP

### Total Estimated Effort
| Feature | Est. Time | Complexity |
|---------|-----------|------------|
| F1: Environment File | 2-4 hours | Medium |
| F2: LED Indicators | 3-5 hours | Medium |
| F3: Python Testing Project | 4-6 hours | Medium |
| **Total** | **9-15 hours** | **Medium** |

### Key Dependencies
- **Hardware:** ESP32-S3 development board, 3 LEDs, resistors, breadboard
- **Software:** PlatformIO, Arduino framework, Python 3.8+, `requests` library
- **External:** None (local-only operation per project scope)

### Critical Path
```
F1: Environment File → F2: LED Indicators → F3: Python Testing Project → DOCS
```
*F1 and F2 can technically be developed in parallel, but F3 depends on the simulator being fully functional.*

---

## ✅ Confirmed Features

### Feature 1: Environment File for Secrets
- **ID:** F1
- **Description:** Store WiFi credentials (SSID, password) in a local `.env` file in the project root. This file must be read at startup and added to `.gitignore` to prevent credential leaks.
- **Priority:** High
- **Complexity:** Medium
- **Dependencies:** None (standalone feature)
- **Acceptance Criteria:**
  - [ ] `.env` file created with WIFI_SSID and WIFI_PASSWORD keys
  - [ ] `.env` file added to `.gitignore`
  - [ ] Parser function reads `.env` file at startup
  - [ ] Parser handles missing file gracefully (falls back to defaults)
  - [ ] Parser handles missing keys gracefully (falls back to defaults)
  - [ ] Parser trims whitespace from values
  - [ ] Parser handles comments (lines starting with #)
  - [ ] WiFi credentials loaded from `.env` override config.h defaults
  - [ ] All existing WiFi functionality remains operational
  - [ ] No hardcoded credentials in source files

### Feature 2: LED Status Indicators
- **ID:** F2
- **Description:** Add three LEDs connected to GPIO pins 1, 2, and 3 to visually indicate door status with ACTIVE HIGH logic.
- **Priority:** High
- **Complexity:** Medium
- **Dependencies:** F1 (environment file, though technically independent)
- **Acceptance Criteria:**
  - [ ] GPIO 1 (GREEN) lights when door state is OPEN
  - [ ] GPIO 2 (YELLOW) lights when door state is OPENING or CLOSED
  - [ ] GPIO 3 (RED) lights when door state is CLOSED
  - [ ] Only one LED is active at a time
  - [ ] LED states update within 100ms of state changes
  - [ ] ACTIVE HIGH logic implemented (LED ON = HIGH signal)
  - [ ] LED initialization happens in `setup()` before WiFi connection
  - [ ] LED update happens in `loop()` or state change handler
  - [ ] All existing door simulation logic remains unchanged

### Feature 3: Python Testing Interface
- **ID:** F3
- **Description:** Create a separate Python project in `/tests` folder that connects to the simulator via HTTP and provides a testing interface for sending door commands.
- **Priority:** High
- **Complexity:** Medium
- **Dependencies:** F1, F2 (simulator must be fully functional)
- **Acceptance Criteria:**
  - [ ] Python project structure created in `/tests` folder
  - [ ] `requirements.txt` includes `requests` library
  - [ ] Main test script (`test_simulator.py`) can send all door commands
  - [ ] Test script can read door state via API
  - [ ] Test script can read door history via API
  - [ ] Test script handles HTTP errors gracefully
  - [ ] Test script has configurable simulator IP/port
  - [ ] Test script validates response status codes
  - [ ] Example usage documented in README or comments
  - [ ] All tests pass against running simulator

---

## 🚀 Development Roadmap

### Phase 1: Foundation (Estimated: 2-4 hours)
*Focus: Environment file support and project infrastructure*

- [ ] **Step 1.1:** Update `.gitignore` to include `.env` file
  - **Task:** Add `.env` and `.env.*` patterns to `.gitignore`
  - **Complexity:** Low
  - **Dependencies:** None
  - **Acceptance Criteria:** `.env` files are ignored by git

- [ ] **Step 1.2:** Create environment file parser utility
  - **Task:** Create `include/EnvParser.h` and `src/EnvParser.cpp`
  - **Complexity:** Medium
  - **Dependencies:** None
  - **Acceptance Criteria:** Parser can read key-value pairs from `.env` file
  - **File Location:** `include/EnvParser.h`, `src/EnvParser.cpp`

- [ ] **Step 1.3:** Integrate environment parser into main startup
  - **Task:** Call env parser in `setup()` before WiFi connection
  - **Complexity:** Low
  - **Dependencies:** Step 1.2
  - **Acceptance Criteria:** WiFi credentials loaded from `.env` override defaults
  - **File Location:** `src/main.cpp`

- [ ] **Step 1.4:** Update `config.h` to use environment-loaded values
  - **Task:** Modify WiFi credential defines to check env values first
  - **Complexity:** Low
  - **Dependencies:** Step 1.3
  - **Acceptance Criteria:** Credentials from `.env` take precedence
  - **File Location:** `include/config.h`

- [ ] **Step 1.5:** Create example `.env` file template
  - **Task:** Create `.env.example` with placeholder values
  - **Complexity:** Low
  - **Dependencies:** None
  - **Acceptance Criteria:** `.env.example` committed to repo with example structure
  - **File Location:** `.env.example`

### Phase 2: Hardware Integration (Estimated: 3-5 hours)
*Focus: LED status indicators*

- [ ] **Step 2.1:** Add LED pin definitions to config
  - **Task:** Define GPIO pin constants for LEDs in `config.h`
  - **Complexity:** Low
  - **Dependencies:** None
  - **Acceptance Criteria:** Pin definitions exist: LED_GREEN_PIN=1, LED_YELLOW_PIN=2, LED_RED_PIN=3
  - **File Location:** `include/config.h`

- [ ] **Step 2.2:** Create LED manager class
  - **Task:** Create `include/DoorLedIndicator.h` and `src/DoorLedIndicator.cpp`
  - **Complexity:** Medium
  - **Dependencies:** Step 2.1
  - **Acceptance Criteria:**
    - Class initializes LED pins as OUTPUT
    - Class has method to update LEDs based on DoorState
    - Class implements ACTIVE HIGH logic
    - Only one LED active at a time
  - **File Location:** `include/DoorLedIndicator.h`, `src/DoorLedIndicator.cpp`

- [ ] **Step 2.3:** Integrate LED manager into main
  - **Task:** Instantiate and update LED manager in `setup()` and `loop()`
  - **Complexity:** Low
  - **Dependencies:** Step 2.2
  - **Acceptance Criteria:** LEDs reflect door state correctly
  - **File Location:** `src/main.cpp`

- [ ] **Step 2.4:** Add LED state mapping logic
  - **Task:** Map DoorState enum to LED states
  - **Complexity:** Low
  - **Dependencies:** Step 2.2
  - **Acceptance Criteria:**
    - OPEN → GREEN
    - OPENING or CLOSED → YELLOW
    - CLOSED → RED
  - **File Location:** `src/DoorLedIndicator.cpp`

- [ ] **Step 2.5:** Verify LED functionality
  - **Task:** Manual testing with physical LEDs
  - **Complexity:** Low
  - **Dependencies:** Step 2.3
  - **Acceptance Criteria:** Visual confirmation of correct LED behavior

### Phase 3: Testing Infrastructure (Estimated: 4-6 hours)
*Focus: Python testing project*

- [ ] **Step 3.1:** Create Python project structure
  - **Task:** Create `/tests` folder with `__init__.py`
  - **Complexity:** Low
  - **Dependencies:** None
  - **Acceptance Criteria:** `/tests` folder exists with Python package structure

- [ ] **Step 3.2:** Create requirements file
  - **Task:** Create `tests/requirements.txt` with `requests` dependency
  - **Complexity:** Low
  - **Dependencies:** Step 3.1
  - **Acceptance Criteria:** `requirements.txt` contains `requests>=2.25.0`
  - **File Location:** `tests/requirements.txt`

- [ ] **Step 3.3:** Create door simulator client class
  - **Task:** Create `tests/door_client.py` with HTTP client methods
  - **Complexity:** Medium
  - **Dependencies:** Step 3.2
  - **Acceptance Criteria:**
    - Class can send commands (open, close, hold)
    - Class can get door state
    - Class can get door history
    - Class handles HTTP errors
    - Class has configurable base URL
  - **File Location:** `tests/door_client.py`

- [ ] **Step 3.4:** Create test script
  - **Task:** Create `tests/test_simulator.py` with test scenarios
  - **Complexity:** Medium
  - **Dependencies:** Step 3.3
  - **Acceptance Criteria:**
    - Test opens door and verifies state
    - Test closes door and verifies state
    - Test hold command and verifies state
    - Test intermediate states (opening, closing)
    - Test error handling (invalid commands)
  - **File Location:** `tests/test_simulator.py`

- [ ] **Step 3.5:** Create example usage script
  - **Task:** Create `tests/example_usage.py` demonstrating all features
  - **Complexity:** Low
  - **Dependencies:** Step 3.4
  - **Acceptance Criteria:** Script demonstrates all door operations
  - **File Location:** `tests/example_usage.py`

- [ ] **Step 3.6:** Create Python README
  - **Task:** Create `tests/README.md` with setup and usage instructions
  - **Complexity:** Low
  - **Dependencies:** Step 3.5
  - **Acceptance Criteria:** Documentation explains how to run tests
  - **File Location:** `tests/README.md`

### Phase 4: Documentation & Finalization (Estimated: 1-2 hours)
*Focus: Documentation updates*

- [ ] **Step 4.1:** Update DECISIONS.md
  - **Task:** Document all new design decisions
  - **Complexity:** Low
  - **Dependencies:** All previous steps
  - **Acceptance Criteria:**
    - Environment file decision documented
    - LED indicator decision documented
    - Python testing decision documented
  - **File Location:** `DECISIONS.md`

- [ ] **Step 4.2:** Update MEMORY.md
  - **Task:** Record all changes made during implementation
  - **Complexity:** Low
  - **Dependencies:** All previous steps
  - **Acceptance Criteria:** All actions logged with files changed and verification
  - **File Location:** `MEMORY.md`

- [ ] **Step 4.3:** Update main README.md
  - **Task:** Add sections for new features
  - **Complexity:** Low
  - **Dependencies:** All previous steps
  - **Acceptance Criteria:**
    - Environment file usage documented
    - LED wiring instructions added
    - Python testing project referenced
  - **File Location:** `README.md`

- [ ] **Step 4.4:** Verify all existing tests still pass
  - **Task:** Run PlatformIO build and native unit tests
  - **Complexity:** Low
  - **Dependencies:** All previous steps
  - **Acceptance Criteria:**
    - `pio run` succeeds
    - Native tests pass (6 scenarios)
    - Python tests pass against running simulator

---

## 🧪 Testing Matrix

| Feature | Unit Tests | Integration Tests | Manual Verification |
|---------|------------|-------------------|---------------------|
| **F1: Environment File** | Parser reads valid .env file; handles missing file; handles missing keys; trims whitespace; ignores comments | Parser integrates with WiFi setup; credentials override defaults; file not committed to git | Visual: Check serial output shows correct SSID from .env |
| **F2: LED Indicators** | LED class initializes pins; LED class maps states correctly; only one LED active at a time | LEDs update with door state changes; ACTIVE HIGH logic works | Visual: LEDs change color with door state changes |
| **F3: Python Testing** | Client sends valid commands; client reads state; client reads history; client handles errors | Client connects to running simulator; all commands work end-to-end | Visual: Run test script, observe simulator response |

### Test Data Requirements

#### Unit Tests (C++ - Native)
- **F1:** Test env file with all keys, missing keys, empty file, comments, whitespace
- **F2:** Test LED mappings for all DoorState values
- **F3:** Not applicable (Python project)

#### Integration Tests
- **F1:** Start simulator with `.env` file present, verify WiFi connects with env credentials
- **F2:** Start simulator with LEDs connected, send commands via API, verify LED changes
- **F3:** Run Python test script against running simulator, verify all commands work

#### Manual Verification Procedures

**F1: Environment File**
1. Create `.env` file with valid credentials
2. Verify `.env` is in `.gitignore`
3. Build and flash simulator
4. Check serial output shows correct SSID
5. Verify WiFi connects successfully

**F2: LED Indicators**
1. Connect LEDs to GPIO 1 (GREEN), 2 (YELLOW), 3 (RED) with appropriate resistors
2. Build and flash simulator
3. Send `open` command via API
4. Verify GREEN LED lights, others off
5. Send `close` command via API
6. Verify RED LED lights during closing
7. Verify YELLOW LED lights when door is CLOSED
8. Send `hold` command during travel
9. Verify YELLOW LED lights

**F3: Python Testing Project**
1. Install Python 3.8+
2. `cd tests && pip install -r requirements.txt`
3. Build and flash simulator, note IP address
4. Run `python test_simulator.py --ip <SIMULATOR_IP>`
5. Verify all tests pass
6. Run `python example_usage.py --ip <SIMULATOR_IP>`
7. Verify all examples work correctly

---

## ⚠️ Risk Assessment

### Technical Risks

| Risk | Impact | Likelihood | Mitigation Strategy | Fallback Plan |
|------|--------|------------|---------------------|---------------|
| **ESP32-S3 GPIO conflicts** | High | Low | Use well-documented GPIO pins (1, 2, 3); verify not used by WiFi/serial | Change to alternative pins (4, 5, 6) |
| **Environment file parsing errors** | Medium | Low | Implement robust parser with error handling | Fall back to config.h defaults |
| **LED hardware issues** | Medium | Medium | Document required hardware (LEDs, resistors) | Provide troubleshooting guide |
| **Python dependency conflicts** | Low | Low | Pin exact versions in requirements.txt | Use virtual environment |
| **Network connectivity issues** | Medium | Medium | Python client implements retry logic | Manual IP configuration option |

### Dependency Risks

| Dependency | Risk | Mitigation | Fallback |
|------------|------|------------|----------|
| **PlatformIO** | Build system changes | Pin PlatformIO version in project | Use alternative build system |
| **ArduinoJson v7** | Library updates | Pin specific version in platformio.ini | Downgrade or fork library |
| **Python requests** | Library deprecation | Pin version in requirements.txt | Use urllib or httpx |
| **ESP32-S3 hardware** | Supply chain issues | Document multiple compatible boards | Use alternative ESP32 variant |

### Timeline Risks

| Risk | Mitigation Strategy | Contingency Plan |
|------|---------------------|------------------|
| **Hardware delivery delay** | Order hardware early; use simulation for software development | Develop LED logic without physical hardware first |
| **Integration issues between features** | Develop incrementally with frequent testing | Isolate features, fix integration issues separately |
| **Underestimated complexity** | Break down into smaller steps; regular progress checks | Adjust timeline, prioritize critical features |

---

## 📋 File Changes Summary

### New Files to Create
```
.env.example                           # Example environment file
.gitignore (updated)                   # Add .env patterns
include/EnvParser.h                    # Environment parser header
src/EnvParser.cpp                      # Environment parser implementation
include/DoorLedIndicator.h             # LED indicator header
src/DoorLedIndicator.cpp               # LED indicator implementation
tests/__init__.py                     # Python package init
tests/requirements.txt                # Python dependencies
tests/door_client.py                  # Python client class
tests/test_simulator.py               # Python test script
tests/example_usage.py                # Python examples
tests/README.md                       # Python project documentation
```

### Modified Files
```
include/config.h                       # Add LED pin defines, env integration
src/main.cpp                          # Integrate env parser, LED manager
.gitignore                            # Add .env and .env.*
README.md                             # Add new feature documentation
DECISIONS.md                          # Document new decisions
MEMORY.md                             # Record all changes
```

---

## ✅ Quality Gates

- [ ] Every feature has at least one unit test requirement
- [ ] Every feature has at least one integration test requirement
- [ ] All dependencies are documented and have fallback plans
- [ ] Every step has clear acceptance criteria
- [ ] User has explicitly confirmed the final feature set ✅
- [ ] Risk assessment includes mitigation strategies
- [ ] Timeline estimates are realistic and include buffers

---

## 🎯 Next Steps

### Immediate (Start Now)
1. **Implement F1: Environment File Support**
   - Create `.env.example`
   - Update `.gitignore`
   - Create `EnvParser.h` and `EnvParser.cpp`
   - Integrate into `main.cpp` and `config.h`

### Subsequent
2. **Implement F2: LED Status Indicators**
   - Add LED pin definitions to `config.h`
   - Create `DoorLedIndicator.h` and `DoorLedIndicator.cpp`
   - Integrate into `main.cpp`

### Final
3. **Implement F3: Python Testing Project**
   - Create `/tests` folder structure
   - Create all Python files
   - Test against running simulator

### Verification
4. **Update Documentation**
   - Update `DECISIONS.md`
   - Update `MEMORY.md`
   - Update `README.md`

---

## 📞 Support & Feedback

For questions or issues during implementation:
- Check DECISIONS.md for design rationale
- Check MEMORY.md for previous implementation details
- Review existing code in include/ and src/ for patterns
- Consult PlatformIO documentation for ESP32-S3 specifics

---

*Document Version: 1.0*
*Last Updated: 2026-10-14*
*Status: Approved - Ready for Implementation*
