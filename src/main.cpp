#include <WiFi.h>
#include <LittleFS.h>
#include "config.h"
#include "DoorSim.h"
#include "DoorApiServer.h"
#include "EnvParser.h"

uint64_t millis64() {
  static uint32_t low = 0, high = 0;
  uint32_t now = millis();
  if (now < low) high++;
  low = now;
  return ((uint64_t)high << 32) | now;
}

namespace {
DoorSim door;
DoorApiServer* apiServer = nullptr;
String wifiSSID = WIFI_SSID;
String wifiPassword = WIFI_PASSWORD;

void connectWifi() {
  Serial.printf("[DoorSim] Connecting to Wi-Fi SSID '%s'...\n", wifiSSID.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();
  Serial.printf("[DoorSim] Connected, IP: %s\n",
                WiFi.localIP().toString().c_str());
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(250);
  Serial.println("\n[DoorSim] BMS door simulator starting (local-only, no Siemens BMS connection)");
  Serial.printf("[DoorSim] Partition: %s\n", BMS_PARTITION_ID);
  Serial.printf("[DoorSim] Point:    %s\n", DOOR_POINT_ID);

  // Initialize LittleFS for reading environment file
  if (!LittleFS.begin()) {
    Serial.println("[DoorSim] LittleFS initialization failed! Using default credentials.");
  } else {
    // Read environment file
    EnvParser envParser;
    if (envParser.readFile(ENV_FILE_PATH)) {
      Serial.println("[DoorSim] Environment file loaded.");
      
      // Override WiFi credentials if present in .env file
      if (envParser.hasKey("WIFI_SSID")) {
        wifiSSID = envParser.getValue("WIFI_SSID", WIFI_SSID);
        Serial.printf("[DoorSim] Using WIFI_SSID from .env: %s\n", wifiSSID.c_str());
      }
      if (envParser.hasKey("WIFI_PASSWORD")) {
        wifiPassword = envParser.getValue("WIFI_PASSWORD", WIFI_PASSWORD);
        Serial.printf("[DoorSim] Using WIFI_PASSWORD from .env\n");
      }
    } else {
      Serial.println("[DoorSim] No environment file found. Using default credentials.");
    }
  }

  connectWifi();
  door.begin();
  apiServer = new DoorApiServer(door);
  apiServer->begin();
  Serial.println("[DoorSim] HTTP API ready on port 80");
}

void loop() {
  door.update();
  apiServer->handleClient();
  delay(5);
}
