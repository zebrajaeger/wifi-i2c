## Context

The firmware exposes generic I2C scan, write, read, and write-read REST endpoints. The repository already includes a runnable MCP4725 DAC example; an ADS1115 ADC example should follow the same style while demonstrating register-style reads.

The ADS1115 is a configurable ADC. A useful example needs to show more than a raw read: it should write the configuration register, wait for single-shot conversion, read the conversion register, and convert the signed raw value to a voltage based on the selected gain.

## Goals / Non-Goals

**Goals:**
- Add `examples/ads1115/` with a README, `ads1115.js`, and `package.json`.
- Document ADS1115 assumptions such as controller IP, I2C address, channel, gain, data rate, and input voltage limits.
- Show scan usage through `GET /api/i2c/scan`.
- Show register-style ADS1115 usage through `POST /api/i2c/write-read`.
- Provide a runnable Node.js CLI equivalent in polish to the MCP4725 example.
- Keep all examples dependency-free and based on Node.js built-in APIs.

**Non-Goals:**
- No firmware source changes.
- No new device-specific REST routes.
- No npm dependency installation requirement.
- No exhaustive ADS1115 driver covering every comparator and data-rate mode.

## Decisions

- Use `examples/ads1115/` as the directory name.
  - Rationale: The requested device name is unambiguous and unlike the MCP4725 path there is no legacy typo to preserve.
  - Alternative considered: A shared `examples/i2c-devices/` directory. That would be less direct for users browsing examples.

- Provide both a README and a runnable Node.js file.
  - Rationale: The README teaches the REST transactions, while the script gives users something they can run immediately.
  - Alternative considered: README-only snippets. The user specifically asked for a Node.js example that can be directly tested.

- Demonstrate single-shot reads from AIN0 by default.
  - Rationale: Single-shot mode is safer and easier to reason about in an HTTP example than continuous conversion.
  - Alternative considered: Continuous conversion polling. That adds timing/state complexity that is not needed for a first example.

- Use `/api/i2c/write-read` for conversion register reads after writing the register pointer.
  - Rationale: ADS1115 register access is a natural fit for the existing write-then-read endpoint.
  - Alternative considered: Separate `/write` then `/read` requests. That risks another master changing the register pointer between calls and is a weaker demonstration of the current API.

## Risks / Trade-offs

- ADS1115 inputs can be damaged if voltages exceed supply or selected range. -> Document voltage limits and defaults clearly.
- ADS1115 address depends on ADDR pin wiring. -> Use `0x48` as the common default and instruct users to confirm with scan.
- Conversion timing depends on data rate. -> Use a conservative wait in the script and document it as configurable.
- Raw-to-voltage conversion depends on PGA full-scale range. -> Keep gain choices explicit in code and README.
