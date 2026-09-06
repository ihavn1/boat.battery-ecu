# Node-RED Configuration Guide

This guide configures the deployed battery firmware through the Signal K HTTP API.

## Current Paths

Replace `<id>` with `house` or `starter`:

```text
electrical.batteries.<id>.capacity.remaining
electrical.batteries.<id>.capacity.actual
electrical.batteries.<id>.capacity.nominal
electrical.batteries.<id>.capacity.stateOfCharge
electrical.batteries.<id>.configuration.chargeEfficiency
electrical.batteries.<id>.configuration.dischargeEfficiency
```

Capacity values are joules. The firmware converts them to and from its internal amp-hour model using a 12 V nominal voltage. SOC is a ratio from 0 to 1. Efficiency values are percentages from 0 to 100.

## HTTP Request Node

Use a Function node before an HTTP Request node:

```javascript
msg.url = "http://localhost:3000/signalk/v1/api/vessels/self/" + msg.path;
msg.method = "PUT";
msg.payload = { value: msg.value };
msg.headers = { "Content-Type": "application/json" };
return msg;
```

Configure the HTTP Request node to use `msg.method`, `msg.url`, and a JSON request body.

## Examples

### Set Remaining Capacity

```javascript
msg.path = "electrical/batteries/house/capacity/remaining";
msg.value = 4320000;
return msg;
```

### Set Actual Capacity

```javascript
msg.path = "electrical/batteries/house/capacity/actual";
msg.value = 6912000;
return msg;
```

### Set Nominal Capacity

```javascript
msg.path = "electrical/batteries/house/capacity/nominal";
msg.value = 8640000;
return msg;
```

### Set Charge and Discharge Efficiency

```javascript
msg.path = "electrical/batteries/house/configuration/chargeEfficiency";
msg.value = 95;
return msg;
```

```javascript
msg.path = "electrical/batteries/house/configuration/dischargeEfficiency";
msg.value = 98;
return msg;
```

Duplicate these flows for the starter battery by replacing `house` with `starter`.

## Direct curl Examples

```bash
curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/capacity/remaining \
  -H "Content-Type: application/json" -d '{"value": 4320000}'

curl -X PUT http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/configuration/chargeEfficiency \
  -H "Content-Type: application/json" -d '{"value": 95}'

curl http://localhost:3000/signalk/v1/api/vessels/self/electrical/batteries/house/capacity/stateOfCharge
```

## Troubleshooting

- `405 Method Not Allowed`: use `PUT`, not `POST`, for configuration changes.
- Capacity values appear incorrect: ensure the request uses joules, not amp-hours.
- SOC is not updated: verify the `capacity.actual` and `capacity.remaining` paths.
- Values do not survive reboot: verify the response and confirm the ESP32 has completed its save path.

The firmware persists manual changes immediately. GPIO27 emergency shutdown persistence is also available for the final power-loss window.
