#pragma once

#include <Arduino.h>

// 64-bit millisecond counter; Arduino's millis() wraps after ~49 days.
uint64_t millis64();
