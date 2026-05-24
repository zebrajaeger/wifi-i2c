## 1. Firmware API

- [x] 1.1 Extend GPIO pin definitions, runtime state, and list response with DAC capability and last DAC value metadata.
- [x] 1.2 Add DAC value validation and JSON response helpers in the GPIO REST module.
- [x] 1.3 Register and implement `POST /api/gpio/dac` using ESP32 internal DAC-capable pins.

## 2. Documentation And Example

- [x] 2.1 Update `README.md` with DAC discovery, endpoint usage, response shape, and safety/precision notes.
- [x] 2.2 Extend `examples/gpio/gpio.js` and package scripts with a DAC write command.
- [x] 2.3 Update `examples/gpio/README.md` with DAC write usage.

## 3. Verification

- [x] 3.1 Build the firmware with PlatformIO.
- [x] 3.2 Verify the GPIO example still supports list, configure, read, write, ADC, and the new DAC command.
- [x] 3.3 If hardware is connected, flash and smoke-test DAC writes against GPIO25 or GPIO26.
