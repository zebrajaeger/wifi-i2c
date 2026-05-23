# i2c-rest-api Specification

## Purpose
TBD - created by archiving change add-i2c-rest-api. Update Purpose after archive.
## Requirements
### Requirement: REST API starts in WiFi station mode
The controller SHALL start the I2C REST API after it successfully connects to the configured WiFi network as a station.

#### Scenario: WiFi connection succeeds
- **WHEN** the controller boots with valid WiFi credentials and connects to the configured network
- **THEN** the controller starts HTTP REST routes for I2C operations

#### Scenario: Controller enters provisioning mode
- **WHEN** the controller starts the provisioning access point
- **THEN** the controller does not expose I2C REST API routes through the provisioning portal

### Requirement: I2C bus is initialized for master operations
The controller SHALL initialize the ESP32 I2C bus as master before handling I2C REST operations.

#### Scenario: API mode starts
- **WHEN** the controller starts the I2C REST API
- **THEN** the controller initializes the I2C master bus before accepting scan, read, or write requests

### Requirement: API scans for I2C devices
The controller SHALL provide an endpoint that scans the I2C bus for attached 7-bit devices and returns the discovered addresses.

#### Scenario: Client scans the bus
- **WHEN** a client sends `GET /api/i2c/scan`
- **THEN** the controller probes valid 7-bit I2C addresses and returns a JSON response containing discovered addresses

#### Scenario: No devices acknowledge
- **WHEN** a client scans the bus and no addresses acknowledge
- **THEN** the controller returns a successful JSON response with an empty device list

### Requirement: API writes bytes to an I2C address
The controller SHALL provide an endpoint that writes caller-supplied bytes to a validated I2C address.

#### Scenario: Client writes bytes
- **WHEN** a client sends `POST /api/i2c/write` with a valid address and byte array
- **THEN** the controller writes the bytes to the I2C bus and returns a JSON response with the I2C transmission status

#### Scenario: Client submits invalid write payload
- **WHEN** a client sends `POST /api/i2c/write` with an invalid address, empty byte array, non-byte value, or payload exceeding the configured limit
- **THEN** the controller rejects the request before touching the I2C bus and returns a JSON error response

### Requirement: API reads bytes from an I2C address
The controller SHALL provide an endpoint that reads a caller-specified number of bytes from a validated I2C address.

#### Scenario: Client reads bytes
- **WHEN** a client sends `POST /api/i2c/read` with a valid address and read length
- **THEN** the controller requests bytes from the I2C address and returns a JSON response containing the bytes that were read

#### Scenario: Client submits invalid read payload
- **WHEN** a client sends `POST /api/i2c/read` with an invalid address or read length outside the configured limit
- **THEN** the controller rejects the request before touching the I2C bus and returns a JSON error response

### Requirement: API supports write-then-read transactions
The controller SHALL provide an endpoint for register-style I2C transactions that write prefix bytes before reading response bytes from the same address.

#### Scenario: Client performs write-read transaction
- **WHEN** a client sends `POST /api/i2c/write-read` with a valid address, prefix byte array, and read length
- **THEN** the controller writes the prefix bytes, reads the requested response bytes, and returns a JSON response containing transmission status and read bytes

#### Scenario: Prefix write fails
- **WHEN** the prefix write in a write-read transaction receives an I2C error
- **THEN** the controller returns a JSON error response and does not report the transaction as successful

### Requirement: API uses consistent JSON responses
The controller SHALL return JSON responses for I2C REST API requests.

#### Scenario: Request succeeds
- **WHEN** an I2C REST request succeeds
- **THEN** the response includes `ok: true` and operation-specific result data

#### Scenario: Request fails
- **WHEN** an I2C REST request fails validation or encounters an I2C transmission error
- **THEN** the response includes `ok: false` and a machine-readable error field

