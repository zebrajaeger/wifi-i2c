## Context

The repository already includes examples for concrete I2C devices that are driven through the controller's generic REST API. The requested IA219 example is treated as an INA219 example, matching the common I2C current, voltage, and power monitor. The example should match the existing device-example pattern: a dedicated folder, README, Node.js CLI, and no firmware changes.

The existing I2C REST API supports bus scanning and write-then-read register transactions. The INA219 exposes register-style measurements and requires calibration before current and power registers are meaningful, so the example needs to document both raw voltage reads and optional calibration.

## Goals / Non-Goals

**Goals:**
- Add an `examples/ina219/` directory with a README, runnable Node.js CLI, and package scripts.
- Demonstrate `GET /api/i2c/scan` to confirm the INA219 is visible.
- Demonstrate register reads through `POST /api/i2c/write-read`.
- Demonstrate optional calibration writes through `POST /api/i2c/write` so current and power registers can be read.
- Convert raw shunt-voltage, bus-voltage, current, and power registers into readable engineering units.
- Keep the example dependency-free so it can run with Node.js 18 or newer.

**Non-Goals:**
- No firmware route changes or new device-specific REST endpoints.
- No PlatformIO or build configuration changes.
- No automatic selection of board-specific current limits beyond conservative CLI defaults.
- No persistent calibration storage on the controller.

## Decisions

1. Place the example under `examples/ina219/`.

   This keeps the device example structure consistent with `examples/mcp4425/`, `examples/ads1115/`, and `examples/ina3221/`. Alternative considered: combining INA219 and INA3221 into one INA-family example, but the register maps and calibration behavior differ enough to deserve separate scripts.

2. Use the existing generic I2C REST endpoints.

   The example will call `/api/i2c/scan`, `/api/i2c/write-read`, and optionally `/api/i2c/write` for calibration. Alternative considered: adding an INA219-specific firmware endpoint, but that would mix device logic into the controller and weaken the generic REST API story.

3. Make calibration explicit.

   Bus voltage and shunt voltage can be read from raw registers without calibration. Current and power registers depend on the calibration register, so the CLI will write a calibration value only when the user requests calibrated reads. Alternative considered: always writing a default calibration value, but keeping it explicit avoids surprising writes to connected hardware.

4. Provide practical defaults but expose overrides.

   The CLI will default to address `0x40`, shunt resistor `0.1 ohm`, and current LSB `100 uA`, while allowing overrides for address, shunt resistance, and current LSB. This matches many breakout examples while still letting users adapt to their board.

5. Keep output readable and scriptable.

   The script will print a concise table plus JSON details, similar to the INA3221 example. This gives quick terminal feedback and preserves raw REST responses for debugging.

## Risks / Trade-offs

- The user typed `IA219`, which appears to mean `INA219` -> The change name and files use `ina219`, and the README can mention this assumption if useful.
- INA219 addresses vary by board jumpers -> The README and CLI default to `0x40` but support address override and recommend scanning first.
- Current and power conversion depends on calibration choices -> The script will make calibration explicit and document how `current-lsb` and the calibration register interact.
- Writing calibration changes device state -> Calibration write is opt-in and limited to the INA219 calibration register.
- Measurement ranges depend on the shunt and circuit -> The README will call out voltage/current safety limits and ask users to verify their shunt resistor.
