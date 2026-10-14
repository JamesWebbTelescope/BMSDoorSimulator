# BMS Door Simulator (ESP32-S3)

Simulates a door device as it would be seen by a Siemens Building X BMS.
Receives door commands over the local network and returns statuses with
realistic travel delays. No connection to the actual Siemens BMS is made.

## Build & flash

Requires [PlatformIO](https://platformio.org/).

```sh
pio run                 # build
pio run -t upload       # flash
pio device monitor      # serial log (115200)
```

## API

The device joins Wi-Fi (see `include/config.h`) and serves a JSON:API-style
HTTP interface on port 80, mirroring the Building X Operations API shape:

### Command the door

```
PATCH /api/openness/operations/partitions/<PARTITION>/points/<POINT_ID>
Content-Type: application/vnd.api+json

{
  "data": {
    "type": "Point",
    "id": "<POINT_ID>",
    "attributes": {
      "pointValue": { "value": "open" }
    }
  }
}
```

`value` may be `open`, `close`, or `hold` (values other than these are
rejected with `400`).

### Read the current state

```
GET /api/openness/operations/partitions/<PARTITION>/points/<POINT_ID>
```

Returns the door state: one of `open`, `closed`, `opening`, `closing`,
`hold`, `unknown`.

### Read status history

```
GET /api/openness/operations/partitions/<PARTITION>/points/<POINT_ID>/values
```

Returns the recorded state transitions (timestamped) since boot, oldest
first.

## Behavior

- Commands take effect with realistic door travel delays (see
  `include/config.h`): the door reports `opening`/`closing` while moving,
  then `open`/`closed` when travel completes.
- A `hold` command stops the door at its current position and reports
  `hold`. Subsequent `open`/`close` commands resume travel from the
  current position.
- A command received while the door is already moving in the same
  direction is accepted but has no additional effect.

## Reference

The Siemens Building X Operations API is used only as a real-world
reference for the interface shape:
https://developer.siemens.com/building-x-openness/api/building-operations/quickstart.html
