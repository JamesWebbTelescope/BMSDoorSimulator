#include "EnvParser.h"

#if defined(ESP32) || defined(ESP8266)
#include <LittleFS.h>
#define HAVE_FILESYSTEM 1
#else
#define HAVE_FILESYSTEM 0
#endif

// Default .env file path
static const char* DEFAULT_ENV_PATH = "/.env";

String EnvParser::trim(const String& str) {
  if (str.isEmpty()) return str;
  
  size_t start = 0;
  while (start < str.length() && (str[start] == ' ' || str[start] == '\t')) {
    start++;
  }
  
  if (start >= str.length()) return String();
  
  size_t end = str.length() - 1;
  while (end > start && (str[end] == ' ' || str[end] == '\t')) {
    end--;
  }
  
  return str.substring(start, end + 1);
}

bool EnvParser::isComment(const String& line) {
  String trimmed = trim(line);
  return trimmed.isEmpty() || trimmed.startsWith("#");
}

bool EnvParser::parseContent(const String& content) {
  clear();

  // Split content into lines
  int start = 0;
  int end = 0;
  
  while (end < content.length() && count_ < 32) {
    end = content.indexOf('\n', start);
    if (end == -1) {
      end = content.length();
    }
    
    String line = content.substring(start, end);
    
    // Remove carriage return if present
    if (!line.isEmpty() && line[line.length() - 1] == '\r') {
      line = line.substring(0, line.length() - 1);
    }

    // Skip comments and empty lines
    if (isComment(line)) {
      start = end + 1;
      continue;
    }

    // Find the equals sign
    int equalPos = line.indexOf('=');
    if (equalPos == -1) {
      start = end + 1;
      continue;
    }

    // Split into key and value
    String key = trim(line.substring(0, equalPos));
    String value = trim(line.substring(equalPos + 1));

    // Skip empty keys
    if (key.isEmpty()) {
      start = end + 1;
      continue;
    }

    // Store the pair
    pairs_[count_].key = key;
    pairs_[count_].value = value;
    count_++;
    
    start = end + 1;
  }
  
  return true;
}

bool EnvParser::readFile(const char* filepath) {
  clear();

#if HAVE_FILESYSTEM
  // Try to initialize filesystem (LittleFS for ESP32)
  if (!LittleFS.begin()) {
    return false;
  }

  // Open the file
  File file = LittleFS.open(filepath, "r");
  if (!file) {
    return false;
  }

  // Read entire file content
  String content = String();
  while (file.available()) {
    char c = file.read();
    content += c;
  }
  
  file.close();
  
  // Parse the content
  return parseContent(content);
#else
  // Native/host build - not supported for reading actual files
  return false;
#endif
}

String EnvParser::getValue(const char* key) const {
  return getValue(key, "");
}

String EnvParser::getValue(const char* key, const char* defaultValue) const {
  for (size_t i = 0; i < count_; i++) {
    if (pairs_[i].key.equals(key)) {
      return pairs_[i].value;
    }
  }
  return String(defaultValue);
}

bool EnvParser::hasKey(const char* key) const {
  for (size_t i = 0; i < count_; i++) {
    if (pairs_[i].key.equals(key)) {
      return true;
    }
  }
  return false;
}

void EnvParser::clear() {
  count_ = 0;
}
