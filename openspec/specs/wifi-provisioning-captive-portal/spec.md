# wifi-provisioning-captive-portal Specification

## Purpose
TBD - created by archiving change add-wifi-provisioning-captive-portal. Update Purpose after archive.
## Requirements
### Requirement: Boot connects to configured WiFi
The controller SHALL load saved WiFi credentials on boot and attempt to connect to the configured network before entering provisioning mode.

#### Scenario: Saved network is reachable
- **WHEN** saved WiFi credentials exist and the configured network is reachable
- **THEN** the controller connects as a WiFi station and does not start the provisioning access point

#### Scenario: No saved credentials exist
- **WHEN** the controller boots without saved WiFi credentials
- **THEN** the controller starts provisioning mode

#### Scenario: Saved network is unreachable
- **WHEN** saved WiFi credentials exist but the configured network cannot be reached within the connection timeout
- **THEN** the controller starts provisioning mode

### Requirement: Provisioning mode starts a local access point
The controller SHALL start a local WiFi access point when provisioning mode is active.

#### Scenario: Access point is available for setup
- **WHEN** provisioning mode starts
- **THEN** the controller exposes a WiFi access point that a smartphone or computer can join for setup

### Requirement: Captive portal is served through the access point
The controller SHALL serve a captive portal while provisioning mode is active.

#### Scenario: Client opens the portal
- **WHEN** a client connected to the provisioning access point requests a web page
- **THEN** the controller presents the WiFi provisioning portal

#### Scenario: Client requests an arbitrary hostname
- **WHEN** a client connected to the provisioning access point resolves or opens an arbitrary hostname
- **THEN** the controller redirects or resolves the request to the local provisioning portal

### Requirement: Portal lists nearby WiFi networks by signal strength
The captive portal SHALL display discovered nearby WiFi networks as SSIDs sorted by signal strength from strongest to weakest.

#### Scenario: Networks are discovered
- **WHEN** the captive portal is loaded and WiFi networks are found
- **THEN** the SSID list is shown with the strongest signal at the top

#### Scenario: Duplicate SSIDs are discovered
- **WHEN** multiple scan results use the same SSID
- **THEN** the SSID list shows that SSID once using the strongest detected signal

#### Scenario: No networks are discovered
- **WHEN** the captive portal is loaded and no WiFi networks are found
- **THEN** the portal shows an empty or unavailable network state without saving credentials

### Requirement: Portal accepts and stores WiFi credentials
The captive portal SHALL allow the user to select an SSID, enter a password, and save the credentials persistently.

#### Scenario: User saves credentials
- **WHEN** the user selects an SSID, enters a password, and submits the form
- **THEN** the controller stores the SSID and password for use on the next boot

#### Scenario: User submits without SSID
- **WHEN** the user submits the form without selecting an SSID
- **THEN** the controller rejects the submission and keeps provisioning mode active

### Requirement: Portal can restart the controller
The captive portal SHALL provide a reboot action so the user can restart the controller after saving credentials.

#### Scenario: User triggers reboot
- **WHEN** the user activates the reboot action in the captive portal
- **THEN** the controller restarts and attempts to connect using the saved WiFi credentials during boot

