## Context

The firmware project already includes implemented capabilities for WiFi provisioning, captive portal setup, and an I2C REST API, but the repository root does not provide a README. Existing knowledge is split across source files, OpenSpec specs, and prior change artifacts.

The README should serve two audiences: a developer who wants to build or flash the firmware, and a user/tester who wants to understand the controller modes and verify the I2C API after WiFi setup.

## Goals / Non-Goals

**Goals:**
- Add a concise top-level `README.md` that explains the project purpose and main behavior.
- Document the PlatformIO workflow for build, upload, and serial monitoring.
- Summarize WiFi provisioning, AP fallback, captive portal behavior, and I2C REST endpoints.
- Provide practical example requests for smoke testing, especially I2C scan.
- Link concepts to the current firmware behavior without duplicating implementation details excessively.

**Non-Goals:**
- No firmware code changes.
- No changes to REST API routes, request/response schemas, WiFi provisioning behavior, or build configuration.
- No exhaustive hardware manual or device-specific I2C command catalog.

## Decisions

- Keep the README operational and concise.
  - Rationale: This repository is a firmware project; readers need the fastest path to understand, build, flash, and test.
  - Alternative considered: A long architecture document. That would duplicate OpenSpec artifacts and be harder to keep current.

- Include endpoint examples but avoid promising device-specific semantics beyond generic I2C bus access.
  - Rationale: Attached I2C devices can vary, while the controller API remains generic.
  - Alternative considered: Documenting the currently observed device address as a fixed expectation. That would make the README brittle.

- Mention provisioning and API modes separately.
  - Rationale: The controller intentionally exposes the I2C REST API only after successful WiFi station connection; provisioning mode is for setup.
  - Alternative considered: A single "web server" section. That would blur an important operational boundary.

## Risks / Trade-offs

- README can drift from firmware behavior if future changes skip documentation updates. -> Keep content high-level and add tasks for verifying commands and endpoint names against source.
- Hardware setup details may be incomplete for unknown I2C peripherals. -> Document generic bus behavior and leave device-specific details out of scope.
- Network addresses are environment-specific. -> Use placeholders and mark observed addresses as examples only.
