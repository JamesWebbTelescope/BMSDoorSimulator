#include <WiFi.h>
#include "config.h"
#include "DoorSim.h"
#include "DoorApiServer.h"

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

void connectWifi() {
  Serial.printf("[DoorSim] Connecting to Wi-Fi SSID '%s'...\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
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
