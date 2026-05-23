## ADDED Requirements

### Requirement: Repository has a top-level README
The repository SHALL include a top-level `README.md` that introduces the `wifi-i2c` firmware project.

#### Scenario: Reader opens the repository
- **WHEN** a reader opens the repository root
- **THEN** a `README.md` is available and explains that the controller provides WiFi provisioning and I2C access over a REST API

### Requirement: README documents firmware operating modes
The README SHALL describe the normal WiFi station/API mode and the provisioning access point/captive portal mode.

#### Scenario: Reader learns boot behavior
- **WHEN** a reader reviews the README
- **THEN** the README explains that saved reachable WiFi credentials start API mode and missing or unreachable credentials start provisioning mode

### Requirement: README documents build and device workflow
The README SHALL include the project-standard PlatformIO commands needed to build, upload, and monitor the firmware.

#### Scenario: Developer prepares firmware
- **WHEN** a developer follows the README setup instructions
- **THEN** the README provides commands for building, flashing, and serial monitoring the ESP32 firmware

### Requirement: README documents the I2C REST API
The README SHALL summarize the available I2C REST API operations and provide practical request examples.

#### Scenario: User verifies I2C bus access
- **WHEN** a user wants to test the controller after WiFi connection
- **THEN** the README includes an example for scanning the I2C bus through the REST API

### Requirement: README documents provisioning usage
The README SHALL explain how a user connects to the provisioning access point, selects an SSID, saves credentials, and reboots the controller.

#### Scenario: User configures WiFi
- **WHEN** a user needs to configure WiFi credentials
- **THEN** the README provides the provisioning access point name, portal behavior, credential save flow, and reboot expectation
