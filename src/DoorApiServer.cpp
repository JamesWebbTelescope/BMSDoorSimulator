#include "DoorApiServer.h"
#include "config.h"

namespace {

String urlDecode(const String& s) {
  String out;
  out.reserve(s.length());
  char buf[3] = {0};
  for (size_t i = 0; i < s.length(); i++) {
    if (s[i] == '%' && i + 2 < s.length()) {
      buf[0] = s[i + 1];
      buf[1] = s[i + 2];
      out += (char)strtol(buf, nullptr, 16);
      i += 2;
    } else if (s[i] == '+') {
      out += ' ';
    } else {
      out += s[i];
    }
  }
  return out;
}

}  // namespace

String DoorApiServer::extractPointId(const String& path,
                                     bool withValuesSuffix) {
  const String prefix =
      String("/api/openness/operations/partitions/") + BMS_PARTITION_ID +
      "/points/";
  if (!path.startsWith(prefix)) return "";
  String rest = path.substring(prefix.length());
  if (withValuesSuffix) {
    if (!rest.endsWith("/values")) return "";
    rest.remove(rest.length() - 7);
    return urlDecode(rest);
  }
  if (rest.indexOf('/') != -1) return "";
  return urlDecode(rest);
}

void DoorApiServer::sendJson(int code, const String& body) {
  server_.send(code, "application/vnd.api+json", body);
}

void DoorApiServer::sendError(int code, const char* title,
                              const char* detail) {
  JsonDocument doc;
  JsonArray errors = doc["errors"].to<JsonArray>();
  JsonObject err = errors.add<JsonObject>();
  err["status"] = String(code);
  err["title"] = title;
  err["detail"] = detail;
  String body;
  serializeJson(doc, body);
  sendJson(code, body);
}

void DoorApiServer::handleGetPoint() {
  String pointId = extractPointId(server_.uri(), false);
  if (pointId.isEmpty() || pointId != DOOR_POINT_ID) {
    sendError(404, "Not Found", "Unknown point id.");
    return;
  }

  JsonDocument doc;
  JsonObject data = doc["data"].to<JsonObject>();
  data["type"] = "Point";
  data["id"] = DOOR_POINT_ID;
  JsonObject attrs = data["attributes"].to<JsonObject>();
  attrs["state"] = DoorSim::stateToString(door_.state());
  attrs["position"] = door_.position();
  String body;
  serializeJson(doc, body);
  sendJson(200, body);
}

void DoorApiServer::handlePatchPoint() {
  String pointId = extractPointId(server_.uri(), false);
  if (pointId.isEmpty() || pointId != DOOR_POINT_ID) {
    sendError(404, "Not Found", "Unknown point id.");
    return;
  }

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, server_.arg("plain"));
  if (err) {
    sendError(400, "Invalid Document", "Malformed JSON body.");
    return;
  }
  JsonObject data = doc["data"];
  if (data.isNull() || !data["attributes"]["pointValue"]["value"].is<const char*>()) {
    sendError(400, "Invalid Document",
              "Missing data.attributes.pointValue.value.");
    return;
  }
  String value = data["attributes"]["pointValue"]["value"].as<String>();

  if (!door_.command(value)) {
    String detail = "Unsupported command value: " + value +
                    ". Expected open, close or hold.";
    sendError(400, "Invalid Value", detail.c_str());
    return;
  }

  // Building X answers 202: the command is accepted but the point state is
  // not reflected immediately.
  sendJson(202, "{}");
}

void DoorApiServer::handleGetValues() {
  String pointId = extractPointId(server_.uri(), true);
  if (pointId.isEmpty() || pointId != DOOR_POINT_ID) {
    sendError(404, "Not Found", "Unknown point id.");
    return;
  }

  JsonDocument doc;
  JsonArray data = doc["data"].to<JsonArray>();
  for (size_t i = 0; i < door_.historySize(); i++) {
    const DoorHistoryEntry& e = door_.history()[i];
    JsonObject item = data.add<JsonObject>();
    item["type"] = "PointValue";
    JsonObject attrs = item["attributes"].to<JsonObject>();
    attrs["timestamp"] = String((uint32_t)(e.timestampMs / 1000)) + "000";
    attrs["value"] = DoorSim::stateToString(e.state);
  }
  String body;
  serializeJson(doc, body);
  sendJson(200, body);
}

void DoorApiServer::registerHandlers() {
  const String pointPath =
      String("/api/openness/operations/partitions/") + BMS_PARTITION_ID +
      "/points/" + DOOR_POINT_ID;
  server_.on(pointPath, HTTP_GET, [this] { handleGetPoint(); });
  server_.on(pointPath, HTTP_PATCH, [this] { handlePatchPoint(); });
  server_.on(pointPath + "/values", HTTP_GET, [this] { handleGetValues(); });
  server_.onNotFound([this] { sendError(404, "Not Found", "Unknown route."); });
}

void DoorApiServer::begin() {
  registerHandlers();
  server_.begin();
}
