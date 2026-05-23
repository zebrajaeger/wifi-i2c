## 1. Research Current API And Device Basics

- [ ] 1.1 Review `README.md` and `src/i2c_rest_api.*` for current endpoint names, JSON payloads, and response shapes.
- [ ] 1.2 Confirm MCP4725 common I2C address assumptions and the byte format for setting a 12-bit DAC output value.
- [ ] 1.3 Decide whether the requested folder name `examples/mcp4425/` should be used as-is while clearly titling the content as MCP4725.

## 2. Example Content

- [ ] 2.1 Create the `examples/` directory if it does not exist.
- [ ] 2.2 Create the `examples/mcp4425/` subdirectory.
- [ ] 2.3 Add a self-contained MCP4725 example README under `examples/mcp4425/`.
- [ ] 2.4 Document required placeholders: controller IP, I2C address, VCC/reference voltage, wiring, and safe output assumptions.
- [ ] 2.5 Include a scan example using `GET /api/i2c/scan`.
- [ ] 2.6 Include write examples using `POST /api/i2c/write` for low, mid-scale, and full-scale 12-bit DAC output values.
- [ ] 2.7 Explain how the 12-bit DAC value maps to the MCP4725 data bytes and approximate output voltage.

## 3. Verification

- [ ] 3.1 Verify all example REST endpoint names match the current firmware routes.
- [ ] 3.2 Verify JSON examples are valid and use byte values in the accepted 0-255 range.
- [ ] 3.3 Confirm the change does not modify firmware source files or PlatformIO configuration.
- [ ] 3.4 Optionally run an I2C scan against a connected controller if hardware is available.
