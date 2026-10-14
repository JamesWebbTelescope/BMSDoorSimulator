#pragma once

#include <WebServer.h>
#include <ArduinoJson.h>
#include "DoorSim.h"

class DoorApiServer {
 public:
  explicit DoorApiServer(DoorSim& door) : door_(door), server_(DOOR_API_PORT) {}

  void begin();
  void handleClient() { server_.handleClient(); }

 private:
  void registerHandlers();
  // Extracts the point id from a path like
  // /api/openness/operations/partitions/<PARTITION>/points/<POINT_ID>[/values]
  // Returns an empty String if the path does not match.
  static String extractPointId(const String& path, bool withValuesSuffix);
  void sendJson(int code, const String& body);
  void sendError(int code, const char* title, const char* detail);
  void handleGetPoint();
  void handlePatchPoint();
  void handleGetValues();

  DoorSim& door_;
  WebServer server_;
};
