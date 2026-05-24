## ADDED Requirements

### Requirement: API controls PWM outputs
The controller SHALL provide REST endpoints that start, update, read, and stop PWM output on supported PWM-capable GPIO pins using hardware high-resolution timer/PWM support.

#### Scenario: Client starts PWM output
- **WHEN** a client sends `POST /api/gpio/pwm` or `POST /api/gpio/pin/<n>/pwm` with a supported PWM-capable GPIO, `frequencyHz`, `resolutionBits`, and `duty`
- **THEN** the controller configures hardware PWM for that pin and returns a JSON response containing `ok: true`, pin number, frequency, resolution, duty, maximum duty, channel metadata, and current pin parameters

#### Scenario: Client updates active PWM output
- **WHEN** a client sends another valid PWM request for a pin that is already running PWM through REST
- **THEN** the controller updates the existing hardware PWM output and returns the updated PWM state

#### Scenario: Client reads PWM state
- **WHEN** a client sends `GET /api/gpio/pwm?pin=<n>` or `GET /api/gpio/pin/<n>/pwm` for a supported GPIO
- **THEN** the controller returns whether PWM is active and, when active, the current frequency, resolution, duty, maximum duty, and channel metadata

#### Scenario: Client stops PWM output
- **WHEN** a client sends `POST /api/gpio/pwm/stop` or `POST /api/gpio/pin/<n>/pwm/stop` for a GPIO with REST-managed PWM state
- **THEN** the controller stops PWM on that pin, updates runtime state, and returns the normalized pin parameters

#### Scenario: Client submits invalid PWM request
- **WHEN** a client requests PWM for a missing, unsupported, or non-PWM-capable GPIO, an unsupported frequency or resolution, a duty outside the selected resolution range, an unavailable PWM channel, or an unsupported platform
- **THEN** the controller rejects the request before applying hardware changes and returns a JSON error response

## MODIFIED Requirements

### Requirement: API lists supported GPIO pins
The controller SHALL provide an endpoint that lists GPIO pins available for REST control together with each pin's current parameters and capabilities, including whether the pin supports ADC reads, internal DAC writes, or PWM output.

#### Scenario: Client lists GPIO pins
- **WHEN** a client sends `GET /api/gpio`
- **THEN** the controller returns a JSON response containing supported GPIO entries with pin number, current mode, pull-up state, pull-down state, value, input/output capabilities, ADC capability, DAC capability, PWM capability, last DAC value when available, and current PWM state when available

#### Scenario: Reserved pins are excluded or marked unavailable
- **WHEN** the controller returns the GPIO list
- **THEN** pins reserved for boot, flash, I2C, UART, or board-critical functions are not offered as controllable pins or are explicitly marked unavailable

### Requirement: API tracks runtime GPIO state
The controller SHALL track each REST-managed GPIO pin's runtime configuration and last written output values for the current boot.

#### Scenario: Client lists pins after configuration
- **WHEN** a client configures, digitally writes, DAC-writes, or starts/stops PWM on a GPIO and then sends `GET /api/gpio`
- **THEN** the listed pin entry reflects the latest REST-managed mode, pull configuration, digital value, last digital output value, last DAC value when available, and PWM state when available

### Requirement: API supports path-style GPIO pin routes
The controller SHALL provide path-style routes under `/api/gpio/pin/<pin>/` for supported pin-specific GPIO operations while preserving the existing operation endpoints.

#### Scenario: Client reads ADC value through path-style route
- **WHEN** a client sends `GET /api/gpio/pin/34/adc` for a supported ADC-capable GPIO
- **THEN** the controller returns the same successful ADC JSON response shape as `GET /api/gpio/adc?pin=34`

#### Scenario: Client reads digital value through path-style route
- **WHEN** a client sends `GET /api/gpio/pin/<n>/read` for a supported readable GPIO
- **THEN** the controller returns the same successful digital read JSON response shape as `GET /api/gpio/read?pin=<n>`

#### Scenario: Client configures pin through path-style route
- **WHEN** a client sends `POST /api/gpio/pin/<n>/configure` with the same JSON body accepted by `POST /api/gpio/configure` except that the pin comes from the path
- **THEN** the controller applies the configuration and returns the same successful configuration JSON response shape

#### Scenario: Client writes digital output through path-style route
- **WHEN** a client sends `POST /api/gpio/pin/<n>/write` with the same JSON body accepted by `POST /api/gpio/write` except that the pin comes from the path
- **THEN** the controller writes the digital value and returns the same successful write JSON response shape

#### Scenario: Client writes DAC value through path-style route
- **WHEN** a client sends `POST /api/gpio/pin/<n>/dac` with the same JSON body accepted by `POST /api/gpio/dac` except that the pin comes from the path
- **THEN** the controller writes the DAC value and returns the same successful DAC JSON response shape

#### Scenario: Client controls PWM through path-style route
- **WHEN** a client sends `POST /api/gpio/pin/<n>/pwm`, `GET /api/gpio/pin/<n>/pwm`, or `POST /api/gpio/pin/<n>/pwm/stop`
- **THEN** the controller provides the same PWM behavior and response shape as the corresponding operation endpoint

#### Scenario: Client uses invalid path-style route
- **WHEN** a client sends a request to `/api/gpio/pin/<pin>/<operation>` with a non-numeric pin, unsupported operation, unsupported HTTP method, missing body field, or pin capability mismatch
- **THEN** the controller rejects the request before touching hardware and returns a JSON error response

### Requirement: README documents GPIO REST usage
The repository README SHALL document the GPIO REST API endpoints, payloads, response shapes, and safety considerations.

#### Scenario: User reads project documentation
- **WHEN** a user opens `README.md`
- **THEN** the documentation explains how to list GPIOs, configure mode and pulls, read a digital GPIO, write an output, read an ADC GPIO, write an internal DAC output, start/read/stop PWM output, use path-style pin routes such as `/api/gpio/pin/34/adc`, and avoid unsafe pins

### Requirement: GPIO example is provided
The repository SHALL include a runnable GPIO REST example under `examples/gpio/`.

#### Scenario: User tries GPIO REST from Node.js
- **WHEN** a user opens the GPIO example directory
- **THEN** the example provides a Node.js script, package scripts, and documentation for listing, configuring, digitally reading, writing, ADC-reading, DAC-writing, PWM control, and demonstrating path-style GPIO pin routes through the REST API
