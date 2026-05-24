## Context

The GPIO REST API already controls safe allowlisted GPIO pins for digital I/O, ADC reads, internal DAC writes, and path-style pin routes. PWM is another controller output mode that should live in the same GPIO REST module because it is pin-centric, must share the safe pin allowlist, and needs to report its runtime state alongside other GPIO parameters.

On ESP32/Arduino, hardware PWM is typically exposed through the LEDC peripheral. LEDC uses hardware timers and channels, supports configurable frequency and duty resolution, and avoids software-timed GPIO toggling. The implementation should use that hardware path so PWM keeps running independently of HTTP request handling.

## Goals / Non-Goals

**Goals:**

- Add REST control for PWM output on safe output-capable GPIO pins.
- Use ESP32 hardware PWM/high-resolution timer support through LEDC APIs.
- Allow clients to start or update PWM with frequency, duty, and resolution parameters.
- Allow clients to read current REST-managed PWM state and stop PWM on a pin.
- Include PWM capability and current PWM state in `GET /api/gpio`.
- Support both operation endpoints and path-style routes, for example `/api/gpio/pwm` and `/api/gpio/pin/13/pwm`.
- Update README and the existing GPIO example.

**Non-Goals:**

- No software PWM fallback.
- No PWM capture/input measurement.
- No servo-specific angle abstraction.
- No guarantee that every output-capable pin is PWM-capable on every future board; capability must be explicit.
- No change to existing digital, ADC, DAC, or path-style behavior.

## Decisions

1. Add PWM as GPIO capability metadata and runtime state.

   Rationale: Clients already discover safe pins through `GET /api/gpio`. Adding `pwmCapable` and a `pwm` state object keeps discovery in one place and avoids hard-coded board assumptions in clients.

   Alternative considered: Provide a separate `/api/pwm/pins` endpoint. That would split GPIO capability discovery and duplicate pin metadata.

2. Use LEDC hardware PWM channels and timers on ESP32.

   Rationale: LEDC is the ESP32 hardware PWM peripheral exposed by the Arduino framework. It uses hardware timers/channels and is appropriate for high-resolution PWM output without CPU-driven toggling.

   Alternative considered: Toggle GPIOs from firmware loops or software timers. That would be less stable and would interfere with HTTP handling and WiFi work.

3. Allocate one LEDC channel per active REST-managed PWM pin.

   Rationale: The allowlist is small and channel assignment can remain deterministic. A pin that already has an assigned channel can be updated in place; stopping PWM frees or marks the channel inactive.

   Alternative considered: Reassign channels on every write. That makes state harder to reason about and can briefly glitch outputs.

4. Accept raw duty values with explicit resolution.

   Rationale: LEDC duty range depends on resolution bits. A request shape with `frequencyHz`, `resolutionBits`, and `duty` maps directly to hardware behavior and keeps client-side intent precise.

   Alternative considered: Accept only percentage duty. Percent is ergonomic but loses exact raw control and requires rounding. A later helper can be added in examples without changing firmware semantics.

5. Keep path-style and operation endpoints behaviorally identical.

   Rationale: Existing GPIO API now supports both styles. PWM should follow that convention to keep the API predictable.

## Risks / Trade-offs

- PWM can conflict with prior digital or DAC state on the same pin -> Starting PWM records the mode as PWM and updates GPIO list state so clients can see the active output mode.
- Invalid frequency/resolution combinations can be unsupported by LEDC -> Validate request bounds before attaching hardware and return JSON errors for unsupported values.
- LEDC APIs differ between ESP32 Arduino framework versions -> Implement against the APIs available in the current PlatformIO environment and isolate calls behind small helpers.
- PWM output can affect attached hardware immediately -> Documentation and examples should tell users to list pins first and connect loads safely.
