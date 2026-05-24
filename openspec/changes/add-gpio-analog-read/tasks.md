## 1. Firmware API

- [ ] 1.1 Extend GPIO pin definitions and list response with analog capability metadata.
- [ ] 1.2 Add analog-read validation and JSON response helpers in the GPIO REST module.
- [ ] 1.3 Register and implement `GET /api/gpio/analog?pin=<n>` using safe analog-capable pins.

## 2. Documentation And Example

- [ ] 2.1 Update `README.md` with analog GPIO discovery, endpoint usage, response shape, and safety notes.
- [ ] 2.2 Extend `examples/gpio/gpio.js` and package scripts with an analog-read command.
- [ ] 2.3 Update `examples/gpio/README.md` with analog-read usage.

## 3. Verification

- [ ] 3.1 Build the firmware with PlatformIO.
- [ ] 3.2 Verify the GPIO example still supports list, configure, read, write, and the new analog command.
- [ ] 3.3 If hardware is connected, flash and smoke-test analog reads against an analog-capable GPIO.
