## 1. Research Current API And ADS1115 Basics

- [ ] 1.1 Review `README.md` and `src/i2c_rest_api.*` for current endpoint names, JSON payloads, and response shapes.
- [ ] 1.2 Review the MCP4725 example structure to mirror README, Node.js CLI, and package script style.
- [ ] 1.3 Confirm ADS1115 common address assumptions, pointer/config/conversion register usage, and raw signed conversion format.
- [ ] 1.4 Decide default example settings for address, channel, gain/full-scale range, data rate, and conversion wait time.

## 2. Example Content

- [ ] 2.1 Create the `examples/ads1115/` subdirectory.
- [ ] 2.2 Add a self-contained ADS1115 README under `examples/ads1115/`.
- [ ] 2.3 Document required placeholders: controller IP, I2C address, channel, gain, data rate, input wiring, and safe voltage limits.
- [ ] 2.4 Include a scan example using `GET /api/i2c/scan`.
- [ ] 2.5 Include REST examples for writing the ADS1115 configuration register and reading the conversion register.
- [ ] 2.6 Explain how raw ADS1115 bytes become a signed integer and approximate voltage.
- [ ] 2.7 Add a runnable `ads1115.js` Node.js CLI for scan and single-shot reads.
- [ ] 2.8 Add a `package.json` with direct npm scripts for scanning and reading.

## 3. Verification

- [ ] 3.1 Verify all example REST endpoint names match the current firmware routes.
- [ ] 3.2 Verify JSON examples are valid and byte values are in the accepted 0-255 range.
- [ ] 3.3 Verify the Node.js file parses successfully without third-party dependencies.
- [ ] 3.4 Confirm the change does not modify firmware source files or PlatformIO configuration.
- [ ] 3.5 Optionally run an I2C scan against a connected controller if hardware is available.
