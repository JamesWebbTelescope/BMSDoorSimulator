#pragma once

#include <Arduino.h>

// Environment file parser for reading key-value pairs from .env files
// Supports simple KEY=VALUE format with optional whitespace and comments

class EnvParser {
 public:
  // Initialize the parser (does not read file yet)
  EnvParser() = default;

  // Parse a string containing .env file content
  // Returns true if parsing was successful
  bool parseContent(const String& content);

  // Read and parse .env file from the given path
  // On ESP32: reads from LittleFS
  // On host: not supported (returns false)
  // Returns true if file was successfully opened and parsed
  bool readFile(const char* filepath = "/.env");

  // Get a value by key, returns empty string if not found
  String getValue(const char* key) const;

  // Get a value by key with a default fallback
  String getValue(const char* key, const char* defaultValue) const;

  // Check if a key exists in the parsed environment
  bool hasKey(const char* key) const;

  // Get the number of key-value pairs parsed
  size_t count() const { return count_; }

  // Clear all parsed key-value pairs
  void clear();

 private:
  struct KeyValuePair {
    String key;
    String value;
  };

  KeyValuePair pairs_[32];  // Maximum 32 key-value pairs
  size_t count_ = 0;

  // Helper to trim whitespace from both ends of a string
  static String trim(const String& str);

  // Helper to check if a line is a comment
  static bool isComment(const String& line);
};
