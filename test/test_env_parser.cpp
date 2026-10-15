// Native test for the EnvParser class using an Arduino stub.
// Build: g++ -std=c++17 -Iinclude -Itest/stubs test/test_env_parser.cpp -o env_parser_test

#include <cstdio>
#include <cstdint>
#include <string>

// ---- Arduino stub ----
static uint64_t stub_millis = 0;
unsigned long millis() { return (unsigned long)stub_millis; }
long map(long x, long a, long b, long c, long d) {
  if (b == a) return c;
  return c + (x - a) * (d - c) / (b - a);
}
int constrain(int v, int lo, int hi) { return v < lo ? lo : (v > hi ? hi : v); }

class String {
  std::string s;

 public:
  String() = default;
  String(const char* p) : s(p ? p : "") {}
  String(const String& o) : s(o.s) {}
  String& operator=(const String& o) { s = o.s; return *this; }
  String& operator=(const char* p) { s = p ? p : ""; return *this; }
  bool operator==(const char* p) const { return s == (p ? p : ""); }
  bool operator==(const String& o) const { return s == o.s; }
  bool operator!=(const char* p) const { return s != (p ? p : ""); }
  const char* c_str() const { return s.c_str(); }
  bool isEmpty() const { return s.empty(); }
  int length() const { return (int)s.length(); }
  char charAt(size_t i) const { return i < s.length() ? s[i] : 0; }
  String substring(size_t start, size_t end) const {
    if (start >= s.length()) return String();
    if (end > s.length()) end = s.length();
    return String(s.substr(start, end - start).c_str());
  }
  String substring(size_t start) const {
    if (start >= s.length()) return String();
    return String(s.substr(start).c_str());
  }
  int indexOf(char c) const {
    size_t p = s.find(c);
    return p == std::string::npos ? -1 : (int)p;
  }
  int indexOf(char c, size_t from) const {
    if (from >= s.length()) return -1;
    size_t p = s.find(c, from);
    return p == std::string::npos ? -1 : (int)p;
  }
  bool startsWith(const char* p) const {
    if (!p) return false;
    return s.rfind(p, 0) == 0;
  }
  String operator+(const String& o) const { return String((s + o.s).c_str()); }
  String operator+(const char* p) const { return String((s + (p ? p : "")).c_str()); }
  String& operator+=(const String& o) { s += o.s; return *this; }
  String& operator+=(const char* p) { s += (p ? p : ""); return *this; }
  bool endsWith(const char* p) const {
    if (!p) return false;
    size_t plen = strlen(p);
    return s.length() >= plen && s.compare(s.length() - plen, plen, p) == 0;
  }
};

// String concatenation with char
String operator+(const char* p, const String& o) {
  return String((std::string(p ? p : "") + o.c_str()).c_str());
}

// ---- File stub for native build (not used, just for compilation) ----
class File {
public:
  File() = default;
  bool operator!() const { return false; }
  bool available() { return false; }
  char read() { return 0; }
  String readStringUntil(char) { return String(); }
  void close() {}
  static File open(const char*, const char*) { return File(); }
};

namespace LittleFS {
  bool begin() { return false; }
}

using ::LittleFS;

#define DOOR_TRAVEL_TIME_MS 5000
#define COMMAND_APPLY_DELAY_MS 1000
#define DOOR_HISTORY_MAX 20
#include "../include/EnvParser.h"
#include "../src/EnvParser.cpp"

// ---- helpers ----
#define CHECK(cond)                                 \
  do {                                              \
    if (!(cond)) {                                  \
      printf("FAIL line %d: %s\n", __LINE__, #cond);\
      fails++;                                      \
    }                                               \
  } while (0)

static int fails = 0;

int main() {
  // Test 1: Parse empty content
  {
    EnvParser parser;
    CHECK(parser.parseContent(""));
    CHECK(parser.count() == 0);
    CHECK(!parser.hasKey("WIFI_SSID"));
    CHECK(parser.getValue("WIFI_SSID").isEmpty());
    CHECK(parser.getValue("WIFI_SSID", "default") == "default");
  }

  // Test 2: Parse simple key-value pairs
  {
    EnvParser parser;
    CHECK(parser.parseContent("WIFI_SSID=MyNetwork\nWIFI_PASSWORD=MyPass123"));
    CHECK(parser.count() == 2);
    CHECK(parser.hasKey("WIFI_SSID"));
    CHECK(parser.hasKey("WIFI_PASSWORD"));
    CHECK(parser.getValue("WIFI_SSID") == "MyNetwork");
    CHECK(parser.getValue("WIFI_PASSWORD") == "MyPass123");
  }

  // Test 3: Parse with whitespace trimming
  {
    EnvParser parser;
    CHECK(parser.parseContent("  WIFI_SSID  =  MyNetwork  \nWIFI_PASSWORD=  MyPass123  "));
    CHECK(parser.count() == 2);
    CHECK(parser.getValue("WIFI_SSID") == "MyNetwork");
    CHECK(parser.getValue("WIFI_PASSWORD") == "MyPass123");
  }

  // Test 4: Parse with comments
  {
    EnvParser parser;
    CHECK(parser.parseContent("# This is a comment\nWIFI_SSID=MyNetwork\n# Another comment\nWIFI_PASSWORD=MyPass123"));
    CHECK(parser.count() == 2);
    CHECK(parser.getValue("WIFI_SSID") == "MyNetwork");
    CHECK(parser.getValue("WIFI_PASSWORD") == "MyPass123");
  }

  // Test 5: Parse with empty lines
  {
    EnvParser parser;
    CHECK(parser.parseContent("WIFI_SSID=MyNetwork\n\n\nWIFI_PASSWORD=MyPass123\n"));
    CHECK(parser.count() == 2);
    CHECK(parser.getValue("WIFI_SSID") == "MyNetwork");
    CHECK(parser.getValue("WIFI_PASSWORD") == "MyPass123");
  }

  // Test 6: Parse with missing keys
  {
    EnvParser parser;
    CHECK(parser.parseContent("WIFI_SSID=MyNetwork"));
    CHECK(parser.count() == 1);
    CHECK(parser.hasKey("WIFI_SSID"));
    CHECK(!parser.hasKey("WIFI_PASSWORD"));
    CHECK(parser.getValue("WIFI_PASSWORD").isEmpty());
    CHECK(parser.getValue("WIFI_PASSWORD", "default_pass") == "default_pass");
  }

  // Test 7: Parse with values containing spaces
  {
    EnvParser parser;
    CHECK(parser.parseContent("WIFI_SSID=My Network\nWIFI_PASSWORD=My Pass 123"));
    CHECK(parser.count() == 2);
    CHECK(parser.getValue("WIFI_SSID") == "My Network");
    CHECK(parser.getValue("WIFI_PASSWORD") == "My Pass 123");
  }

  // Test 8: Parse with CRLF line endings (Windows)
  {
    EnvParser parser;
    CHECK(parser.parseContent("WIFI_SSID=MyNetwork\r\nWIFI_PASSWORD=MyPass123\r\n"));
    CHECK(parser.count() == 2);
    CHECK(parser.getValue("WIFI_SSID") == "MyNetwork");
    CHECK(parser.getValue("WIFI_PASSWORD") == "MyPass123");
  }

  // Test 9: Parse with empty key (should be skipped)
  {
    EnvParser parser;
    CHECK(parser.parseContent("=Invalid\nWIFI_SSID=MyNetwork"));
    CHECK(parser.count() == 1);
    CHECK(parser.getValue("WIFI_SSID") == "MyNetwork");
  }

  // Test 10: Parse with line without equals sign (should be skipped)
  {
    EnvParser parser;
    CHECK(parser.parseContent("InvalidLine\nWIFI_SSID=MyNetwork"));
    CHECK(parser.count() == 1);
    CHECK(parser.getValue("WIFI_SSID") == "MyNetwork");
  }

  // Test 11: Clear and re-parse
  {
    EnvParser parser;
    CHECK(parser.parseContent("WIFI_SSID=Network1"));
    CHECK(parser.count() == 1);
    parser.clear();
    CHECK(parser.count() == 0);
    CHECK(parser.parseContent("WIFI_SSID=Network2"));
    CHECK(parser.getValue("WIFI_SSID") == "Network2");
  }

  // Test 12: Multiple equals signs in value
  {
    EnvParser parser;
    CHECK(parser.parseContent("EQUATION=a=b=c"));
    CHECK(parser.count() == 1);
    CHECK(parser.getValue("EQUATION") == "a=b=c");
  }

  // Test 13: Tab characters in whitespace
  {
    EnvParser parser;
    CHECK(parser.parseContent("\tWIFI_SSID\t=\tMyNetwork\t"));
    CHECK(parser.count() == 1);
    CHECK(parser.getValue("WIFI_SSID") == "MyNetwork");
  }

  // Test 14: Empty value
  {
    EnvParser parser;
    CHECK(parser.parseContent("EMPTY_VALUE=\nWIFI_SSID=MyNetwork"));
    CHECK(parser.count() == 2);
    CHECK(parser.getValue("EMPTY_VALUE") == "");
    CHECK(parser.getValue("WIFI_SSID") == "MyNetwork");
  }

  // Test 15: Full .env file example
  {
    EnvParser parser;
    String content = 
      "# BMS Door Simulator Configuration\n"
      "WIFI_SSID=MyNetwork\n"
      "WIFI_PASSWORD=MySecretPass\n"
      "\n"
      "# Additional settings\n"
      "BMS_PARTITION_ID=part-123\n"
      "DOOR_POINT_ID=door-456\n";
    CHECK(parser.parseContent(content));
    CHECK(parser.count() == 4);
    CHECK(parser.getValue("WIFI_SSID") == "MyNetwork");
    CHECK(parser.getValue("WIFI_PASSWORD") == "MySecretPass");
    CHECK(parser.getValue("BMS_PARTITION_ID") == "part-123");
    CHECK(parser.getValue("DOOR_POINT_ID") == "door-456");
  }

  if (fails == 0) {
    printf("All EnvParser tests passed.\n");
    return 0;
  }
  printf("%d test(s) failed.\n", fails);
  return 1;
}
