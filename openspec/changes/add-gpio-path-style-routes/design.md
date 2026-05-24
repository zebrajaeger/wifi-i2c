## Context

The GPIO REST API currently uses operation endpoints with the pin passed as a query parameter or JSON body, for example `GET /api/gpio/adc?pin=34` and `POST /api/gpio/write` with `{ "pin": 13, "value": 1 }`. The user wants the API to also support a resource-oriented shape such as `/api/gpio/pin/34/adc`.

The firmware uses the Arduino `WebServer`, which does not provide parameterized route templates. Path-style GPIO routes therefore need a small dispatcher that parses `/api/gpio/pin/<pin>/<operation>` and forwards to shared operation logic.

## Goals / Non-Goals

**Goals:**

- Support `/api/gpio/pin/<pin>/adc` for ADC reads.
- Add the same path-style pattern for existing pin-specific GPIO operations where it is natural:
  - `GET /api/gpio/pin/<pin>/read`
  - `POST /api/gpio/pin/<pin>/configure`
  - `POST /api/gpio/pin/<pin>/write`
  - `POST /api/gpio/pin/<pin>/dac`
- Keep existing endpoints such as `/api/gpio/adc?pin=<n>` working.
- Reuse the same validation, hardware access, and response payloads for both endpoint styles.
- Document both endpoint styles, with path-style routes as the preferred pin-specific form.

**Non-Goals:**

- No change to GPIO capability rules, safe pin allowlist, ADC resolution, or DAC value semantics.
- No removal of existing query/body endpoints.
- No new HTTP framework or runtime dependency.

## Decisions

1. Add a path dispatcher for `/api/gpio/pin/<pin>/<operation>`.

   Rationale: Arduino `WebServer` route registration is exact-match oriented, so a dispatcher keeps path parsing contained without introducing a new web server library.

   Alternative considered: Register every likely pin-specific route explicitly. This would duplicate the safe pin list in routing and make future pin list changes more error-prone.

2. Refactor handlers so pin extraction is separate from operation execution.

   Rationale: Existing query/body endpoints and new path-style endpoints should share the same validation and response code. A small helper that supplies the parsed pin avoids behavioral drift.

   Alternative considered: Implement separate path-style handlers for each operation. This would be quick but risks inconsistent errors and response shapes.

3. Keep existing endpoints as compatibility aliases.

   Rationale: Current README examples and user scripts may already call the query/body endpoints. Adding path-style routes should be additive.

   Alternative considered: Replace the old endpoints. That would be a breaking API change and is not required for the requested behavior.

4. Prefer path-style examples in docs while mentioning existing endpoints remain supported.

   Rationale: The new route shape should be discoverable and become the recommended form for pin-specific operations, but compatibility remains useful.

## Risks / Trade-offs

- Path parsing errors could produce confusing 404/400 responses -> validate the segment count, `pin` literal, numeric pin value, and operation name explicitly.
- Dispatcher code could conflict with other future `/api/gpio/...` routes -> only intercept paths under `/api/gpio/pin/` and let other requests fall through to existing not-found behavior.
- Shared handler refactoring could regress existing query/body endpoints -> verify both route styles for ADC and at least one digital write/read path during implementation.
