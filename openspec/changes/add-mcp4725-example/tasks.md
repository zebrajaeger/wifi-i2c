## 1. Research Current API And Device Basics

- [x] 1.1 Review `README.md` and `src/i2c_rest_api.*` for current endpoint names, JSON payloads, and response shapes.
- [x] 1.2 Confirm MCP4725 common I2C address assumptions and the byte format for setting a 12-bit DAC output value.
- [x] 1.3 Decide whether the requested folder name `examples/mcp4425/` should be used as-is while clearly titling the content as MCP4725.

## 2. Example Content

- [x] 2.1 Create the `examples/` directory if it does not exist.
- [x] 2.2 Create the `examples/mcp4425/` subdirectory.
- [x] 2.3 Add a self-contained MCP4725 example README under `examples/mcp4425/`.
- [x] 2.4 Document required placeholders: controller IP, I2C address, VCC/reference voltage, wiring, and safe output assumptions.
- [x] 2.5 Include a scan example using `GET /api/i2c/scan`.
- [x] 2.6 Include write examples using `POST /api/i2c/write` for low, mid-scale, and full-scale 12-bit DAC output values.
- [x] 2.7 Explain how the 12-bit DAC value maps to the MCP4725 data bytes and approximate output voltage.
- [x] 2.8 Include a runnable `mcp4725.js` Node.js example for scan, DAC byte calculation, and write requests.
- [x] 2.9 Include a `package.json` with direct npm scripts for scanning and writing.

## 3. Verification

- [x] 3.1 Verify all example REST endpoint names match the current firmware routes.
- [x] 3.2 Verify JSON examples are valid and use byte values in the accepted 0-255 range.
- [x] 3.3 Confirm the change does not modify firmware source files or PlatformIO configuration.
- [x] 3.4 Optionally run an I2C scan against a connected controller if hardware is available.
- [x] 3.5 Verify the Node.js example uses the current REST endpoint names and request payload shape.
- [x] 3.6 Verify the Node.js file parses successfully without third-party dependencies.
