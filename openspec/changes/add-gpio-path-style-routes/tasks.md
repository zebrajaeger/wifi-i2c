## 1. Firmware Routing

- [x] 1.1 Run GitNexus impact analysis for GPIO route registration and affected GPIO handlers before editing firmware symbols.
- [x] 1.2 Add a `/api/gpio/pin/<pin>/<operation>` dispatcher that validates path shape, pin parsing, operation names, and HTTP methods.
- [x] 1.3 Refactor GPIO operation handling so query/body endpoints and path-style endpoints share validation, hardware access, and JSON response helpers.
- [x] 1.4 Implement path-style support for digital read, configure, digital write, ADC read, and DAC write.

## 2. Documentation And Example

- [x] 2.1 Update `README.md` to document path-style GPIO routes and keep existing endpoint compatibility clear.
- [x] 2.2 Update `examples/gpio/README.md` with path-style route examples.
- [x] 2.3 Extend `examples/gpio/gpio.js` only if useful to demonstrate or test path-style requests without duplicating core commands.

## 3. Verification

- [x] 3.1 Build the firmware with PlatformIO.
- [x] 3.2 Verify existing query/body endpoints still work for at least ADC read and one digital operation.
- [x] 3.3 Verify path-style routes work for `/api/gpio/pin/34/adc` and at least one digital or DAC operation.
- [x] 3.4 Validate the updated OpenSpec capability.
