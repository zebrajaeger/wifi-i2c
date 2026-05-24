## 1. Firmware API

- [x] 1.1 Run GitNexus impact analysis for GPIO route registration, GPIO list serialization, and affected GPIO output handlers before editing firmware symbols.
- [x] 1.2 Extend GPIO pin definitions and runtime state with PWM capability, active PWM metadata, and channel assignment data.
- [x] 1.3 Add PWM request validation for pin, frequency, resolution, duty, channel availability, and platform support.
- [x] 1.4 Implement hardware PWM helpers using ESP32 LEDC/high-resolution timer APIs.
- [x] 1.5 Add REST handlers for starting/updating PWM, reading PWM state, and stopping PWM.
- [x] 1.6 Add operation endpoints and path-style routes for PWM while preserving existing GPIO behavior.

## 2. Documentation And Example

- [x] 2.1 Update `README.md` with PWM discovery, start/read/stop endpoints, payloads, response shape, and safety notes.
- [x] 2.2 Extend `examples/gpio/gpio.js` with PWM start/read/stop commands and path-style support.
- [x] 2.3 Update `examples/gpio/package.json` scripts for PWM commands.
- [x] 2.4 Update `examples/gpio/README.md` with PWM examples and hardware cautions.
- [x] 2.5 Add a servo helper example that maps angle or pulse width to PWM duty.

## 3. Verification

- [x] 3.1 Build the firmware with PlatformIO.
- [x] 3.2 Flash the controller and verify existing GPIO endpoints still work.
- [x] 3.3 Verify PWM start, read, update, and stop through operation endpoints.
- [x] 3.4 Verify PWM start/read/stop through `/api/gpio/pin/<pin>/pwm` path-style routes.
- [x] 3.5 Verify the GPIO Node.js example can exercise PWM commands.
- [x] 3.6 Validate the updated OpenSpec capability.
- [x] 3.7 Verify the servo helper command builds a valid PWM request.
