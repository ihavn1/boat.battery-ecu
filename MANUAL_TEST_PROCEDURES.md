# Manual Test Procedures

Use these procedures to verify functionality based on the test specifications in `test/`.

## Prerequisites
- ESP32 connected and running firmware
- Signal K server accessible
- Serial monitor running: `pio device monitor`

## Test 1: Ah Value Setting & Clamping

```bash
# Set Ah within range (should succeed)
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah \
  -H "Content-Type: application/json" -d '{"value": 180.5}'

# Expected: Ah = 180.5, visible in Signal K

# Set Ah above capacity (should clamp to 200)
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah \
  -H "Content-Type: application/json" -d '{"value": 250.0}'

# Expected: Ah = 200.0 (clamped to capacity)

# Set Ah below zero (should clamp to 0)
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah \
  -H "Content-Type: application/json" -d '{"value": -10.0}'

# Expected: Ah = 0.0 (clamped to minimum)
```

**Pass Criteria:** Ah values clamp correctly to [0, capacity] range

---

## Test 2: Efficiency Configuration

```bash
# Set charge efficiency to 85%
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah/chargeEfficiency \
  -H "Content-Type: application/json" -d '{"value": 85.0}'

# Set discharge efficiency to 95%
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah/dischargeEfficiency \
  -H "Content-Type: application/json" -d '{"value": 95.0}'

# Test clamping: Set efficiency > 100%
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah/chargeEfficiency \
  -H "Content-Type: application/json" -d '{"value": 150.0}'

# Expected: Efficiency = 100% (clamped)

# Test clamping: Set efficiency < 0%
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah/chargeEfficiency \
  -H "Content-Type: application/json" -d '{"value": -10.0}'

# Expected: Efficiency = 0% (clamped)
```

**Pass Criteria:** Efficiencies clamp to [0, 100] range

---

## Test 3: NVS Persistence

```bash
# Set a unique Ah value
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah \
  -H "Content-Type: application/json" -d '{"value": 175.5}'

# Set unique efficiencies
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah/chargeEfficiency \
  -H "Content-Type: application/json" -d '{"value": 87.0}'

curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah/dischargeEfficiency \
  -H "Content-Type: application/json" -d '{"value": 93.0}'

# Restart ESP32 (press reset button)
# After boot, check Signal K:

curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah/chargeEfficiency
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah/dischargeEfficiency
```

**Pass Criteria:** All values persist across reboot

---

## Test 4: Capacity Management

```bash
# Set current capacity (degraded battery: 160Ah from original 200Ah)
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah/capacity \
  -H "Content-Type: application/json" -d '{"value": 160.0}'

# Set Ah to new capacity
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah \
  -H "Content-Type: application/json" -d '{"value": 160.0}'

# Check SOC (should be 100%)
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/stateOfCharge

# Expected: SOC = 100% (160Ah / 160Ah capacity)

# Try to set Ah above new capacity (should clamp to 160)
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah \
  -H "Content-Type: application/json" -d '{"value": 180.0}'

# Expected: Ah = 160.0 (clamped to current capacity)

# Test capacity range clamping (min 0.1Ah)
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah/capacity \
  -H "Content-Type: application/json" -d '{"value": 0.05}'

# Expected: Capacity = 0.1Ah (clamped to minimum)

# Test capacity range clamping (max 10000Ah)
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah/capacity \
  -H "Content-Type: application/json" -d '{"value": 15000.0}'

# Expected: Capacity = 10000Ah (clamped to maximum)
```

**Pass Criteria:** Capacity updates correctly, Ah clamped to capacity, SOC recalculates

---

## Test 5: SOC Calculation

```bash
# Reset to known state: 200Ah capacity, 100Ah current
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah/capacity \
  -H "Content-Type: application/json" -d '{"value": 200.0}'

curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah \
  -H "Content-Type: application/json" -d '{"value": 100.0}'

# Check SOC
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/stateOfCharge

# Expected: SOC = 50% (100Ah / 200Ah)

# Test at full capacity
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah \
  -H "Content-Type: application/json" -d '{"value": 200.0}'

# Expected: SOC = 100%

# Test at empty
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah \
  -H "Content-Type: application/json" -d '{"value": 0.0}'

# Expected: SOC = 0%
```

**Pass Criteria:** SOC formula `(Ah / Capacity) × 100` works correctly

---

## Test 6: Integration Over Time

```bash
# Set known state: 50Ah, 100% efficiencies
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah \
  -H "Content-Type: application/json" -d '{"value": 50.0}'

curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah/chargeEfficiency \
  -H "Content-Type: application/json" -d '{"value": 100.0}'

# Connect a load (e.g., 5A draw) to house battery
# Monitor Ah value over time in Signal K

# After 1 hour with 5A draw:
# Expected: Ah decreased by ~5Ah (50Ah → 45Ah)

# Connect charger (e.g., 10A charge)
# After 30 minutes with 10A charge:
# Expected: Ah increased by ~5Ah (45Ah → 50Ah)
```

**Pass Criteria:** Ah integrates current over time correctly

---

## Test 7: Starter Battery (Short Key Test)

```bash
# Verify starter battery uses short config key "start"
# All operations should work identically to house battery

curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/starter/ah \
  -H "Content-Type: application/json" -d '{"value": 105.0}'

# Restart ESP32
# Verify persistence

curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/starter/ah

# Expected: 105.0Ah persisted
```

**Pass Criteria:** Starter battery ("start" key) persists correctly within NVS 15-char limit

---

## Test 8: Temperature Sensors

```bash
# Monitor temperature readings in Signal K
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/temperature
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/starter/temperature

# Expected: Valid temperature readings in Kelvin
# Typical range: 273K to 333K (0°C to 60°C)

# Physical test: Heat/cool sensor, verify readings change
```

**Pass Criteria:** Temperature sensors read and update every 2 seconds

---

## Test 9: Signal K Path Validation

Verify all paths are accessible:

```bash
# House battery paths
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/voltage
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/current
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/power
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/ah
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/stateOfCharge
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/temperature

# Starter battery paths
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/starter/voltage
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/starter/current
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/starter/power
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/starter/ah
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/starter/stateOfCharge
curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/starter/temperature
```

**Pass Criteria:** All paths return valid data

---

## Test Record Template

Copy this for each test run:

```
Date: ___________
Firmware Version: ___________
Signal K Version: ___________

Test 1 - Ah Clamping:          [ ] PASS  [ ] FAIL
Test 2 - Efficiency Config:    [ ] PASS  [ ] FAIL
Test 3 - NVS Persistence:      [ ] PASS  [ ] FAIL
Test 4 - Capacity Management:  [ ] PASS  [ ] FAIL
Test 5 - SOC Calculation:      [ ] PASS  [ ] FAIL
Test 6 - Integration Over Time:[ ] PASS  [ ] FAIL
Test 7 - Starter Battery:      [ ] PASS  [ ] FAIL
Test 8 - Temperature Sensors:  [ ] PASS  [ ] FAIL
Test 9 - Signal K Paths:       [ ] PASS  [ ] FAIL

Notes:
_________________________________
_________________________________
_________________________________
```
