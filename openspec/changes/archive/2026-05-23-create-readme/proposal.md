## Why

The project currently has firmware behavior and OpenSpec history, but no top-level README for a new developer or user to understand what the controller does and how to build or exercise it. A concise README will make the WiFi provisioning and I2C REST capabilities discoverable without reading the implementation first.

## What Changes

- Add a top-level `README.md` for the `wifi-i2c` firmware project.
- Document the controller purpose, supported modes, and main capabilities.
- Include setup/build/upload/monitor commands using PlatformIO.
- Document the WiFi provisioning flow and I2C REST endpoints at a practical usage level.
- Include brief verification examples such as scanning the I2C bus.

## Capabilities

### New Capabilities
- `project-readme`: Covers the required content and usability expectations for the top-level project README.

### Modified Capabilities

None.

## Impact

- Adds documentation only: `README.md`.
- No firmware behavior, REST API contract, dependencies, build settings, or provisioning flow changes are expected.
