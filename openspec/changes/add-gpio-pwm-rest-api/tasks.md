## 1. Firmware API

- [ ] 1.1 Run GitNexus impact analysis for GPIO route registration, GPIO list serialization, and affected GPIO output handlers before editing firmware symbols.
- [ ] 1.2 Extend GPIO pin definitions and runtime state with PWM capability, active PWM metadata, and channel assignment data.
- [ ] 1.3 Add PWM request validation for pin, frequency, resolution, duty, channel availability, and platform support.
- [ ] 1.4 Implement hardware PWM helpers using ESP32 LEDC/high-resolution timer APIs.
- [ ] 1.5 Add REST handlers for starting/updating PWM, reading PWM state, and stopping PWM.
- [ ] 1.6 Add operation endpoints and path-style routes for PWM while preserving existing GPIO behavior.

## 2. Documentation And Example

- [ ] 2.1 Update `README.md` with PWM discovery, start/read/stop endpoints, payloads, response shape, and safety notes.
- [ ] 2.2 Extend `examples/gpio/gpio.js` with PWM start/read/stop commands and path-style support.
- [ ] 2.3 Update `examples/gpio/package.json` scripts for PWM commands.
- [ ] 2.4 Update `examples/gpio/README.md` with PWM examples and hardware cautions.

## 3. Verification

- [ ] 3.1 Build the firmware with PlatformIO.
- [ ] 3.2 Flash the controller and verify existing GPIO endpoints still work.
- [ ] 3.3 Verify PWM start, read, update, and stop through operation endpoints.
- [ ] 3.4 Verify PWM start/read/stop through `/api/gpio/pin/<pin>/pwm` path-style routes.
- [ ] 3.5 Verify the GPIO Node.js example can exercise PWM commands.
- [ ] 3.6 Validate the updated OpenSpec capability.
