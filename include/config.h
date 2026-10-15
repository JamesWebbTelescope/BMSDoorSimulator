#pragma once

// Wi-Fi credentials. These can be overridden at runtime by values from .env file
// or set at build time via -D flags.
#ifndef WIFI_SSID
#define WIFI_SSID "BMS-SIM"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "changeme"
#endif

// HTTP server port for the JSON:API-style interface.
#define DOOR_API_PORT 80

// Placeholder identifiers mimicking a Building X partition and door point.
// These are arbitrary; any value works since there is no real BMS backend.
#define BMS_PARTITION_ID "00000000-0000-0000-0000-000000000001"
#define DOOR_POINT_ID "a850ec9a-daf6-4630-9c22-af680e82b3a4"

// Door travel delays (milliseconds). These represent the physical time a
// real door takes to move; statuses reflect intermediate states while
// moving and the final state once travel completes.
#define DOOR_TRAVEL_TIME_MS 5000

// Delay before a command is reflected in the point state, mimicking the
// Building X API note that state changes are not applied immediately.
#define COMMAND_APPLY_DELAY_MS 1000

// Maximum number of state transitions kept in the history buffer.
#define DOOR_HISTORY_MAX 20

// Environment file path in LittleFS
#define ENV_FILE_PATH "/.env"

// LED pin definitions for status indicators
#define LED_GREEN_PIN 1    // GREEN LED for OPEN state
#define LED_YELLOW_PIN 2   // YELLOW LED for OPENING/CLOSED states
#define LED_RED_PIN 3      // RED LED for CLOSED state
