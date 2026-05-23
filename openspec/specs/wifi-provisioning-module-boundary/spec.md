# wifi-provisioning-module-boundary Specification

## Purpose
TBD - created by archiving change extract-wifi-provisioning-module. Update Purpose after archive.
## Requirements
### Requirement: WiFi provisioning has a dedicated module boundary
The firmware SHALL keep WiFi provisioning implementation details in a dedicated module instead of embedding them in the firmware entry point.

#### Scenario: Firmware entry point selects provisioning mode
- **WHEN** the controller boots and saved WiFi credentials are missing or unreachable
- **THEN** `main.cpp` starts provisioning mode through the WiFi provisioning module interface

#### Scenario: Firmware entry point selects API mode
- **WHEN** the controller boots and successfully connects to the configured WiFi network
- **THEN** `main.cpp` starts API mode without depending on captive portal implementation details

### Requirement: Provisioning module owns captive portal behavior
The WiFi provisioning module SHALL own AP fallback, DNS captive portal processing, portal route registration, SSID scanning, credential saving, and reboot handling.

#### Scenario: Provisioning mode runs
- **WHEN** provisioning mode is active
- **THEN** provisioning loop work is serviced through the WiFi provisioning module rather than direct DNS or portal calls in `main.cpp`

### Requirement: Existing WiFi provisioning behavior is preserved
The refactor SHALL preserve the externally visible WiFi provisioning behavior defined by `wifi-provisioning-captive-portal`.

#### Scenario: User provisions WiFi after refactor
- **WHEN** a user connects to the provisioning AP and opens the captive portal
- **THEN** the portal still lists nearby SSIDs, accepts credentials, saves them, and offers controller reboot behavior

