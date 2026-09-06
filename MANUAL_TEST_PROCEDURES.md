# Manual Test Procedures

These procedures verify the deployed Signal K battery paths and persistence behavior.

## Prerequisites

- ESP32 running the current firmware
- Signal K server accessible at `http://localhost:3000`
- Serial monitor available with `pio device monitor`
- HTTP requests use `{"value": ...}` and `PUT`

The firmware uses a 12 V nominal-voltage conversion for capacity values. Capacity paths use joules; the internal coulomb counter remains in amp-hours.

## Signal K Paths

For each battery, the published paths are:

```text
electrical.batteries.<id>.voltage
electrical.batteries.<id>.current
electrical.batteries.<id>.temperature
electrical.batteries.<id>.capacity.nominal
electrical.batteries.<id>.capacity.actual
electrical.batteries.<id>.capacity.remaining
electrical.batteries.<id>.capacity.stateOfCharge
```

Use `house` or `starter` for `<id>`. Current is positive out of the battery. Temperature is Kelvin. Capacity is joules and SOC is a ratio from 0 to 1.

The custom efficiency paths are:

```text
electrical.batteries.<id>.configuration.chargeEfficiency
electrical.batteries.<id>.configuration.dischargeEfficiency
```

## Test 1: Capacity and SOC

```bash
# Set remaining energy to 50% of a 200 Ah, 12 V house battery.
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/capacity/remaining \
  -H "Content-Type: application/json" -d '{"value": 4320000}'

curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/capacity/remaining
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/capacity/stateOfCharge
```

Expected: remaining capacity is `4320000 J` and SOC is approximately `0.5`. Test clamping with `0` and a value above nominal capacity.

## Test 2: Capacity Configuration

```bash
# House nominal capacity: 200 Ah * 12 V * 3600 = 8640000 J.
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/capacity/nominal \
  -H "Content-Type: application/json" -d '{"value": 8640000}'

# House actual usable capacity: 160 Ah * 12 V * 3600 = 6912000 J.
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/capacity/actual \
  -H "Content-Type: application/json" -d '{"value": 6912000}'
```

Expected: the actual-capacity update changes the SOC denominator and remaining value is clamped when necessary.

## Test 3: Efficiency Configuration

```bash
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/configuration/chargeEfficiency \
  -H "Content-Type: application/json" -d '{"value": 85}'

curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/configuration/dischargeEfficiency \
  -H "Content-Type: application/json" -d '{"value": 95}'
```

Expected: values are clamped to `0` through `100` and affect subsequent coulomb integration.

## Test 4: Persistence

Set distinct capacity and efficiency values for both batteries, reset the ESP32, and query the same paths again:

```bash
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/capacity/remaining
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/configuration/chargeEfficiency
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/starter/capacity/remaining
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/starter/configuration/dischargeEfficiency
```

Expected: all values survive reboot.

## Test 5: Sensors and Direction

```bash
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/voltage
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/current
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/temperature
```

Expected: voltage is in volts, current is positive when leaving the battery, and temperature is in Kelvin.

## Test 6: Emergency Shutdown

Set distinct remaining capacities for both batteries, pull GPIO27 low to simulate imminent power loss, and allow the shutdown path to run. After a normal power cycle, query both `capacity/remaining` paths.

Expected: both values were persisted before deep sleep. GPIO27 is active low and uses an external signal source.

## Test Record

```text
Date: ___________
Firmware Version: ___________
Signal K Version: ___________

Capacity and SOC:       [ ] PASS  [ ] FAIL
Capacity configuration: [ ] PASS  [ ] FAIL
Efficiency settings:    [ ] PASS  [ ] FAIL
Persistence:            [ ] PASS  [ ] FAIL
Sensors and direction:  [ ] PASS  [ ] FAIL
Emergency shutdown:     [ ] PASS  [ ] FAIL

Notes:
_________________________________
_________________________________
```
