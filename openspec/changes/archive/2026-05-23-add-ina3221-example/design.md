## Context

The repository already includes examples for using concrete I2C devices through the controller's generic REST API. The INA3221 is a three-channel current and bus-voltage monitor that is already connected, so users need a device-specific guide and script that exercise the existing API without requiring firmware changes.

The existing I2C REST API supports bus scanning, raw writes, raw reads, and write-then-read register transactions. The INA3221 uses register-style access, so the example can build on the same pattern as the ADS1115 example.

## Goals / Non-Goals

**Goals:**
- Add an `examples/ina3221/` directory with a README, runnable Node.js CLI, and package scripts.
- Demonstrate `GET /api/i2c/scan` to confirm the INA3221 is visible.
- Demonstrate INA3221 register reads via `POST /api/i2c/write-read`.
- Convert raw bus-voltage and shunt-voltage register values into volts, millivolts, and optional current estimates.
- Keep the example dependency-free so it can run with Node.js 18 or newer.

**Non-Goals:**
- No firmware route changes or new device-specific REST endpoints.
- No PlatformIO or build configuration changes.
- No persistent INA3221 configuration management beyond optional raw config reads.
- No calibration storage or board-specific current limit enforcement.

## Decisions

1. Place the example under `examples/ina3221/`.

   This mirrors the existing `examples/ads1115/` structure and keeps device-specific examples discoverable by folder name. Alternative considered: adding INA3221 content to the root README, but that would make the root documentation too device-heavy.

2. Use the existing generic I2C REST endpoints.

   The INA3221 example will call `/api/i2c/scan` and `/api/i2c/write-read` rather than adding firmware-level device abstractions. This keeps the example focused on proving how raw I2C devices can be driven externally. Alternative considered: adding a device-specific firmware endpoint such as `/api/ina3221/read`, but that would expand controller responsibilities and duplicate logic that belongs in client examples.

3. Provide a dependency-free Node.js CLI.

   Node.js 18 includes `fetch`, so the example does not need third-party dependencies. The CLI will support scan-only mode, reading one channel, reading all three channels, address override, shunt resistor override, and optional current calculation. Alternative considered: using an INA3221 npm package, but the purpose is to demonstrate the REST I2C API, not local host I2C access.

4. Document register constants and conversions in both README and script.

   The README will describe the key INA3221 registers: config register `0x00`, channel shunt registers `0x01`, `0x03`, `0x05`, and channel bus registers `0x02`, `0x04`, `0x06`. The script will convert signed shunt voltage using `40 uV` per bit and bus voltage using `8 mV` per bit. Current will be calculated only when a shunt resistor value is supplied.

5. Keep the example safe and observational by default.

   The first implementation will read measurements and scan the bus. It will not write configuration registers unless a future change explicitly adds that behavior. This reduces risk on already connected hardware and still validates the REST read flow.

## Risks / Trade-offs

- INA3221 breakout boards may use different I2C addresses -> The README and CLI will default to `0x40` but allow address override and recommend scanning first.
- Current calculation depends on external shunt resistor values -> The script will require an explicit `--shunt-ohms` option before reporting current, and otherwise report only shunt voltage.
- Register conversion details can be easy to misuse -> The README will show the raw byte handling and formulas, while the script centralizes conversion helpers.
- The example cannot validate actual hardware wiring during proposal time -> Implementation tasks include running syntax checks and, if a controller IP is available, scanning/reading the connected INA3221.
