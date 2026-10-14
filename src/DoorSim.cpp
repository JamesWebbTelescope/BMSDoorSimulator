#include "DoorSim.h"
#include "millis64.h"

void DoorSim::begin() {
  setState(DoorState::CLOSED);
}

const char* DoorSim::stateToString(DoorState s) {
  switch (s) {
    case DoorState::OPEN: return "open";
    case DoorState::CLOSED: return "closed";
    case DoorState::OPENING: return "opening";
    case DoorState::CLOSING: return "closing";
    case DoorState::HOLD: return "hold";
    default: return "unknown";
  }
}

void DoorSim::recordHistory(DoorState s) {
  if (historySize_ < DOOR_HISTORY_MAX) {
    history_[historySize_].timestampMs = millis64();
    history_[historySize_].state = s;
    historySize_++;
  } else {
    for (size_t i = 1; i < DOOR_HISTORY_MAX; i++) {
      history_[i - 1] = history_[i];
    }
    history_[DOOR_HISTORY_MAX - 1].timestampMs = millis64();
    history_[DOOR_HISTORY_MAX - 1].state = s;
  }
}

void DoorSim::setState(DoorState s) {
  state_ = s;
  recordHistory(s);
}

bool DoorSim::command(const String& value) {
  DoorState requested;
  if (value == "open") {
    requested = DoorState::OPEN;
  } else if (value == "close") {
    requested = DoorState::CLOSED;
  } else if (value == "hold") {
    requested = DoorState::HOLD;
  } else {
    return false;
  }

  // No-op cases: already at the target or already moving the same way.
  if (state_ == requested) return true;
  if (requested == DoorState::OPEN && state_ == DoorState::OPENING) return true;
  if (requested == DoorState::CLOSED && state_ == DoorState::CLOSING) return true;

  // Commands are not applied immediately, mirroring the Building X API
  // note about command application delays.
  pendingState_ = requested;
  commandApplyAtMs_ = millis64() + COMMAND_APPLY_DELAY_MS;
  hasPending_ = true;
  return true;
}

void DoorSim::update() {
  uint64_t now = millis64();

  if (hasPending_ && now >= commandApplyAtMs_) {
    hasPending_ = false;
    DoorState requested = pendingState_;

    if (requested == DoorState::HOLD) {
      if (state_ == DoorState::OPENING || state_ == DoorState::CLOSING) {
        // Freeze travel at the current position.
        long elapsed = (long)(now - travelStartMs_);
        if (elapsed > DOOR_TRAVEL_TIME_MS) elapsed = DOOR_TRAVEL_TIME_MS;
        position_ = map(elapsed, 0L, (long)DOOR_TRAVEL_TIME_MS,
                       travelFromPosition_, travelTarget_);
        position_ = constrain(position_, 0, 100);
        setState(DoorState::HOLD);
      } else if (state_ == DoorState::OPEN || state_ == DoorState::CLOSED) {
        setState(DoorState::HOLD);
      }
      return;
    }
    // OPEN/CLOSE: fall through so travel also progresses this tick.

    int target = (requested == DoorState::OPEN) ? 100 : 0;
    if (state_ == DoorState::OPEN || state_ == DoorState::CLOSED ||
        state_ == DoorState::HOLD) {
      if (position_ == target) {
        setState(requested);
        return;
      }
    }
    // Start (or restart) travel from the current position.
    travelFromPosition_ = position_;
    travelTarget_ = target;
    travelStartMs_ = now;
    setState(target > travelFromPosition_ ? DoorState::OPENING
                                          : DoorState::CLOSING);
    return;
  }

  if (state_ == DoorState::OPENING || state_ == DoorState::CLOSING) {
    long elapsed = (long)(now - travelStartMs_);
    if (elapsed >= DOOR_TRAVEL_TIME_MS) {
      position_ = travelTarget_;
      setState(travelTarget_ == 100 ? DoorState::OPEN : DoorState::CLOSED);
    } else {
      position_ = map(elapsed, 0L, (long)DOOR_TRAVEL_TIME_MS,
                      travelFromPosition_, travelTarget_);
      position_ = constrain(position_, 0, 100);
    }
  }
}
