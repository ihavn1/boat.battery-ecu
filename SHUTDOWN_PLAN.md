# Plan: GPIO27 "gem-og-sov" i boat.battery-ecu

Overført fra mønsteret i `boat.engine-ecu` (ShutdownCoordinator / ShutdownInterruptMonitor / EngineHoursPersistence).

## Mønster fra boat.engine-ecu

- `IClock` / `ISleepController` — framework-frie interfaces.
- `ShutdownCoordinator` — ren logik der debouncer et lavt signal, gemmer, og først derefter kalder `enterDeepSleep()`.
- `ShutdownInterruptMonitor` — ISR + dedikeret FreeRTOS-task på GPIO-pin, stopper WiFi før gemning.
- Persistence direkte via `Preferences` med write-then-read-back verifikation.

## Tilpasning til boat.battery-ecu

1. **Nye interfaces**: `IClock`, `ISleepController` — genbrugt uændret.
2. **`ShutdownCoordinator`** tilpasset til at gemme to batterier (house + starter) i stedet for én tæller. Kalder `BatteryMonitor::save_state()` på begge, og først når *begge* lykkes, går den videre til dyb søvn.
3. **`ShutdownMonitor`**: ISR på GPIO27 (bekræftet ledig pin) + FreeRTOS-task, stopper WiFi, poller coordinatoren indtil færdig.
4. **`shutdown_helper`**: wiring-funktion (`setupShutdownMonitor()`) der samler `EspClock`, `EspDeepSleepController`, coordinator og monitor — samme mønster som `battery_helper.cpp`.
5. **`main.cpp`**: fanger `BatteryMonitor*` for house/starter og forbinder dem til `setupShutdownMonitor(..., pin=27, debounce=0ms)`.
6. **Eksisterende periodisk/PUT-gemning bevares uændret** — GPIO27 er et supplement, ikke en erstatning.
7. **Tests**: native testsuite der spejler `test_engine_hours.cpp` med fakes for ur, sleep-controller, sensor og storage.

## Valg truffet uden svar fra bruger

- Debounce-værdi: `0 ms` (samme som engine-ecu).
- Rækkefølge: stop WiFi først, gem derefter (samme som engine-ecu).

## Status

Planen er implementeret og verificeret:

- `include/shutdown_interfaces.h`
- `include/shutdown_coordinator.h` (header-only)
- `include/shutdown_monitor.h` / `src/shutdown_monitor.cpp`
- `include/shutdown_helper.h` / `src/shutdown_helper.cpp`
- `test/test_shutdown_coordinator/test_shutdown_coordinator.cpp` (6 tests)
- `src/main.cpp` opdateret til at wire alt sammen

Verifikation:
- `pio test -e native` → 122/122 tests bestået
- `pio run -e az-delivery-devkit-v4` → build og link lykkedes
