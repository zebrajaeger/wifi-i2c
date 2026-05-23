## Context

The firmware already exposes generic I2C master operations through the REST API. The README documents the endpoints, but there is no concrete device example that shows how to translate a real peripheral command into REST calls.

The requested example should live under `examples/mcp4425/` and demonstrate MCP4725 DAC usage through the existing API. The folder name follows the request text, while the content should consistently identify the device as MCP4725.

## Goals / Non-Goals

**Goals:**
- Add an `examples/` directory with a self-contained MCP4725 example under `examples/mcp4425/`.
- Explain wiring assumptions and environment-specific placeholders without requiring firmware changes.
- Show how to scan the bus and identify a likely MCP4725 address.
- Show how to write 12-bit DAC values using the existing `/api/i2c/write` endpoint.
- Include copyable PowerShell commands that match the current REST API.

**Non-Goals:**
- No changes to firmware code or REST endpoints.
- No new host-side dependency or generated binary artifact.
- No exhaustive MCP4725 driver library.
- No assumption that every setup uses the same I2C address, VCC, or output voltage range.

## Decisions

- Use Markdown as the primary example artifact.
  - Rationale: The current repository documentation is Markdown-based and the example is mainly instructional.
  - Alternative considered: A script-only example. That would be less readable for users learning the REST payload format.

- Include optional command snippets rather than a required automation script.
  - Rationale: PowerShell examples are immediately usable on the current Windows workflow and mirror the README.
  - Alternative considered: Python examples. That would add a language/runtime expectation not otherwise required by the project.

- Document MCP4725 fast-mode DAC writes through `/api/i2c/write`.
  - Rationale: The existing API writes arbitrary byte arrays, which is enough to demonstrate setting the DAC output.
  - Alternative considered: Adding device-specific firmware routes. That would expand the controller API and is outside this documentation change.

## Risks / Trade-offs

- The folder name `mcp4425` differs from the MCP4725 device name. -> Make the example title and text clearly say MCP4725 so readers do not confuse the target device.
- MCP4725 boards may use different addresses depending on wiring. -> Use `0x60` as a common default and instruct users to confirm with `/api/i2c/scan`.
- DAC output depends on supply voltage and wiring. -> Treat voltage calculations as examples and document the assumptions.
- REST writes can affect real hardware output. -> Include a short caution to start with safe values and verify wiring before writing full-scale output.
