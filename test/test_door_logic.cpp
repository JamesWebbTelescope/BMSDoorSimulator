// Native test for the DoorSim state machine using an Arduino stub.
// Build: g++ -std=c++17 -Iinclude test/test_door_logic.cpp -o door_test

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
  String(const char* p) : s(p) {}
  String(const String& o) : s(o.s) {}
  String& operator=(const String& o) { s = o.s; return *this; }
  bool operator==(const char* p) const { return s == p; }
  const char* c_str() const { return s.c_str(); }
  bool isEmpty() const { return s.empty(); }
  int indexOf(char c) const {
    size_t p = s.find(c);
    return p == std::string::npos ? -1 : (int)p;
  }
};
uint64_t millis64() { return stub_millis; }

#define DOOR_TRAVEL_TIME_MS 5000
#define COMMAND_APPLY_DELAY_MS 1000
#define DOOR_HISTORY_MAX 20
#include "../include/DoorSim.h"
#include "../src/DoorSim.cpp"

// ---- helpers ----
#define CHECK(cond)                                 \
  do {                                              \
    if (!(cond)) {                                  \
      printf("FAIL line %d: %s\n", __LINE__, #cond);\
      fails++;                                      \
    }                                               \
  } while (0)

static int fails = 0;
static DoorSim* activeDoor = nullptr;
// Steps time in 100ms ticks, like the firmware loop's delay(5)+update().
static void advance(uint64_t ms) {
  while (ms > 0) {
    uint64_t step = ms < 100 ? ms : 100;
    stub_millis += step;
    ms -= step;
    if (activeDoor) activeDoor->update();
  }
}

int main() {
  // 1. open from closed: apply delay -> opening -> travel -> open
  {
    DoorSim d;
    activeDoor = &d;
    d.begin();
    CHECK(d.state() == DoorState::CLOSED);
    CHECK(d.position() == 0);
    CHECK(d.command("open"));
    advance(500);
    CHECK(d.state() == DoorState::CLOSED);  // command not yet applied
    advance(600);                            // past apply delay
    CHECK(d.state() == DoorState::OPENING);
    advance(2500);
    CHECK(d.state() == DoorState::OPENING);
    // ~halfway (travel started at the apply tick)
    CHECK(d.position() >= 50 && d.position() <= 52);
    advance(2500);
    CHECK(d.state() == DoorState::OPEN);
    CHECK(d.position() == 100);
  }

  // 2. close: full cycle back to closed
  {
    DoorSim d;
    activeDoor = &d;
    d.begin();
    d.command("open");
    advance(1000 + 5000);
    CHECK(d.state() == DoorState::OPEN);
    CHECK(d.command("close"));
    advance(1000);
    CHECK(d.state() == DoorState::CLOSING);
    advance(5000);
    CHECK(d.state() == DoorState::CLOSED);
    CHECK(d.position() == 0);
  }

  // 3. hold while opening freezes at current position
  {
    DoorSim d;
    activeDoor = &d;
    d.begin();
    d.command("open");
    advance(1000);  // opening starts
    advance(2000);  // 40% traveled
    CHECK(d.position() == 40);
    CHECK(d.command("hold"));
    advance(1000);  // hold applies
    CHECK(d.state() == DoorState::HOLD);
    int frozen = d.position();
    advance(3000);
    CHECK(d.state() == DoorState::HOLD);
    CHECK(d.position() == frozen);  // no drift
    // resume open from hold
    CHECK(d.command("open"));
    advance(1000);
    CHECK(d.state() == DoorState::OPENING);
    advance(5000);
    CHECK(d.state() == DoorState::OPEN);
    CHECK(d.position() == 100);
  }

  // 4. invalid command rejected
  {
    DoorSim d;
    activeDoor = &d;
    d.begin();
    CHECK(!d.command("unlock"));
    CHECK(d.state() == DoorState::CLOSED);
  }

  // 5. redundant command is a no-op accepted
  {
    DoorSim d;
    activeDoor = &d;
    d.begin();
    CHECK(d.command("open"));
    advance(1000);
    CHECK(d.command("open"));  // already opening
    advance(5000);
    CHECK(d.state() == DoorState::OPEN);
  }

  // 6. history records transitions in order
  {
    DoorSim d;
    activeDoor = &d;
    d.begin();
    d.command("open");
    advance(1000 + 5000);
    CHECK(d.historySize() == 3);  // closed, opening, open
    CHECK(d.history()[0].state == DoorState::CLOSED);
    CHECK(d.history()[1].state == DoorState::OPENING);
    CHECK(d.history()[2].state == DoorState::OPEN);
  }

  activeDoor = nullptr;
  if (fails == 0) {
    printf("All tests passed.\n");
    return 0;
  }
  printf("%d test(s) failed.\n", fails);
  return 1;
}
