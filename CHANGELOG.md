# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## Unreleased

### Added
- Environment parser class in `include/EnvParser.h` and `src/EnvParser.cpp` for reading .env files
- Unit tests for EnvParser in `test/test_env_parser.cpp` with 15 test scenarios
- LittleFS filesystem support via `board_build.filesystem = littlefs` in platformio.ini
- Environment file path constant `ENV_FILE_PATH` in config.h
- LED pin definitions (LED_GREEN_PIN, LED_YELLOW_PIN, LED_RED_PIN) in config.h
- Wokwi project diagram configuration in diagram.json
- Project plan document in docs/plan.txt
- Wokwi simulation configuration via wokwi.toml for ESP32-S3 firmware
- VSCode PlatformIO debug configurations in .vscode/launch.json
- VSCode C/C++ IntelliSense configuration in .vscode/c_cpp_properties.json
- PlatformIO IDE extension recommendation in .vscode/extensions.json
- Prototype development plan document in docs/PROTOTYPE_DEVELOPMENT_PLAN.md
- DECISIONS.md entries 7, 8, 9 for credential management, LED indicators, and Python testing interface
- MEMORY.md section 4 documenting prototype development plan and confirmed features

### Changed
- `.gitignore` to exclude .env, .env.*, and data/.env files
- `.vscode/c_cpp_properties.json` to add LittleFS include path and reorganize library paths
- `src/main.cpp` to integrate EnvParser, initialize LittleFS, and override WiFi credentials from .env file
- `include/config.h` to support runtime credential override from .env file
- `DECISIONS.md` with decisions 10, 11, 12 documenting environment file storage, parser design, and credential override mechanism
- `MEMORY.md` with section 5 documenting Phase 1: Environment File Support implementation

## 0.1.0 - 2026-10-14

### Added
- Initial ESP32-S3 BMS door simulator implementation
- Project structure and documentation
