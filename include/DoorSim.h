#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

enum class DoorState {
  OPEN,
  CLOSED,
  OPENING,
  CLOSING,
  HOLD,
  UNKNOWN,
};

struct DoorHistoryEntry {
  uint64_t timestampMs;
  DoorState state;
};

class DoorSim {
 public:
  DoorSim() = default;

  void begin();
  // Advance the simulated door. Call frequently (e.g. from loop()).
  void update();
  // Handle a command string: "open", "close" or "hold".
  // Returns true if the command was accepted.
  bool command(const String& value);
  DoorState state() const { return state_; }
  // Simulated door position, 0 = fully closed, 100 = fully open.
  int position() const { return position_; }
  const DoorHistoryEntry* history() const { return history_; }
  size_t historySize() const { return historySize_; }

  static const char* stateToString(DoorState s);

 private:
  void setState(DoorState s);
  void recordHistory(DoorState s);

  DoorState state_ = DoorState::UNKNOWN;
  int position_ = 0;  // start closed
  uint64_t travelStartMs_ = 0;
  int travelFromPosition_ = 0;
  int travelTarget_ = 0;
  uint64_t commandApplyAtMs_ = 0;
  DoorState pendingState_ = DoorState::UNKNOWN;
  bool hasPending_ = false;

  DoorHistoryEntry history_[DOOR_HISTORY_MAX];
  size_t historySize_ = 0;
};
