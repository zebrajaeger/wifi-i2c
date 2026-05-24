## ADDED Requirements

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

#### Scenario: Client uses invalid path-style route
- **WHEN** a client sends a request to `/api/gpio/pin/<pin>/<operation>` with a non-numeric pin, unsupported operation, unsupported HTTP method, missing body field, or pin capability mismatch
- **THEN** the controller rejects the request before touching hardware and returns a JSON error response

## MODIFIED Requirements

### Requirement: README documents GPIO REST usage
The repository README SHALL document the GPIO REST API endpoints, payloads, response shapes, and safety considerations.

#### Scenario: User reads project documentation
- **WHEN** a user opens `README.md`
- **THEN** the documentation explains how to list GPIOs, configure mode and pulls, read a digital GPIO, write an output, read an ADC GPIO, write an internal DAC output, use path-style pin routes such as `/api/gpio/pin/34/adc`, and avoid unsafe pins

### Requirement: GPIO example is provided
The repository SHALL include a runnable GPIO REST example under `examples/gpio/`.

#### Scenario: User tries GPIO REST from Node.js
- **WHEN** a user opens the GPIO example directory
- **THEN** the example provides a Node.js script, package scripts, and documentation for listing, configuring, digitally reading, writing, ADC-reading, DAC-writing, and demonstrating path-style GPIO pin routes through the REST API
