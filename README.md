# wifi-i2c

Firmware for an ESP32-based WiFi-to-I2C controller. The controller joins a configured WiFi network and exposes I2C master operations and selected GPIOs through a small HTTP REST API. If WiFi credentials are missing or unreachable, it starts a provisioning access point with a captive portal.

## What It Does

- Connects to saved WiFi credentials on boot.
- Falls back to a local setup access point when WiFi is not configured or unavailable.
- Serves a captive portal for selecting a nearby SSID, saving credentials, and rebooting.
- Starts an I2C REST API after successful WiFi station connection.
- Supports I2C scan, write, read, and write-then-read transactions.
- Starts a GPIO REST API after successful WiFi station connection.
- Supports listing safe GPIOs, configuring mode and pulls, reading digital inputs, writing digital outputs, and reading analog-capable inputs.

## Hardware And Firmware

- Board target: ESP32 Dev Module (`esp32dev`)
- Framework: Arduino for ESP32
- Build system: PlatformIO
- I2C role: ESP32 acts as I2C master
- HTTP server port: `80`

The concrete I2C wiring, GPIO wiring, and attached devices depend on your hardware setup. The I2C API uses 7-bit I2C addresses and validates addresses before touching the bus. The GPIO API exposes only an allowlist of pins and rejects unsupported or reserved pins.

## Build, Flash, Monitor

Install PlatformIO, connect the ESP32 over USB, then run:

```powershell
pio run
```

Flash the firmware:

```powershell
pio run -t upload
```

Open the serial monitor:

```powershell
pio device monitor -b 115200
```

The project currently uses `upload_speed = 1500000` and `monitor_speed = 115200` in `platformio.ini`.

## Local OpenSpec And GitNexus Tooling

The repository includes a root `package.json` only for local development tooling around OpenSpec and GitNexus. It is not part of the ESP32 firmware runtime.

Install the local tools with:

```powershell
npm install
```

Useful scripts:

```powershell
npm run openspec
npm run gn:analyze
npm run gn:wiki
```

The `.git/hooks/pre-commit` hook runs `npm run gn:analyze` before each commit. If GitNexus analysis fails, the commit is aborted so the local code intelligence data stays current.

## Operating Modes

### WiFi Station And API Mode

On boot, the controller loads saved credentials from non-volatile preferences. If the configured WiFi is reachable within the connection timeout, the controller joins that network and starts the I2C and GPIO REST APIs.

The serial log prints the assigned IP address. Use that IP as `<controller-ip>` in API requests:

```text
http://<controller-ip>/api/i2c/scan
```

### Provisioning Mode

If there are no saved credentials, or the saved network cannot be reached, the controller starts provisioning mode:

- Access point SSID: `wifi-i2c-setup`
- Portal IP: `192.168.4.1`
- Captive portal DNS: arbitrary hostnames are resolved or redirected to the portal

Connect a phone or computer to `wifi-i2c-setup`. The captive portal lists nearby WiFi networks sorted by signal strength, strongest first. Select an SSID, enter the password, save the credentials, then use the reboot button. On the next boot, the controller attempts to join the saved WiFi and start API mode.

## I2C REST API

The API is available only in WiFi station mode after a successful WiFi connection. Responses are JSON and include `ok: true` on success or `ok: false` with an `error` field on validation or bus errors.

### Scan

Find attached 7-bit I2C devices:

```powershell
Invoke-RestMethod -Uri http://<controller-ip>/api/i2c/scan
```

Example response:

```json
{
  "ok": true,
  "devices": [
    {
      "address": 96,
      "hex": "0X60"
    }
  ],
  "count": 1
}
```

The address and device count are examples. Your bus may return different devices or an empty list.

### Write

Write bytes to an I2C address:

```powershell
Invoke-RestMethod `
  -Uri http://<controller-ip>/api/i2c/write `
  -Method Post `
  -ContentType "application/json" `
  -Body '{"address":"0x60","bytes":[0,1,2]}'
```

Payload fields:

- `address`: I2C address as a number or `0x`-prefixed string
- `bytes`: array of byte values, 0 through 255

### Read

Read bytes from an I2C address:

```powershell
Invoke-RestMethod `
  -Uri http://<controller-ip>/api/i2c/read `
  -Method Post `
  -ContentType "application/json" `
  -Body '{"address":"0x60","length":4}'
```

Payload fields:

- `address`: I2C address as a number or `0x`-prefixed string
- `length`: number of bytes to read

### Write Then Read

Perform a register-style transaction by writing prefix bytes without a stop condition, then reading response bytes:

```powershell
Invoke-RestMethod `
  -Uri http://<controller-ip>/api/i2c/write-read `
  -Method Post `
  -ContentType "application/json" `
  -Body '{"address":"0x60","prefix":[0],"length":2}'
```

Payload fields:

- `address`: I2C address as a number or `0x`-prefixed string
- `prefix`: bytes written before the read
- `length`: number of bytes to read after the prefix write

## GPIO REST API

The GPIO API is available only in WiFi station mode after a successful WiFi connection. Responses are JSON and include `ok: true` on success or `ok: false` with an `error` field on validation errors.

GPIO access is intentionally limited to a safe allowlist. The API does not expose pins used for flash, boot strapping, UART, or the default I2C bus. Always call `GET /api/gpio` first and choose a pin listed as output-capable before driving external hardware or a pin listed as analog-capable before reading analog values.

Analog reads are exposed only for allowlisted ADC1-capable pins so they remain usable while WiFi is active. The returned values are raw ADC measurements and may need scaling, calibration, or filtering in your client code.

Supported modes:

- `input`
- `input_pullup`
- `input_pulldown`
- `output`
- `output_open_drain`, when supported by the platform

### List GPIOs

List supported GPIOs with current runtime parameters and capability metadata:

```powershell
Invoke-RestMethod -Uri http://<controller-ip>/api/gpio
```

Example response excerpt:

```json
{
  "ok": true,
  "pins": [
    {
      "pin": 13,
      "label": "safe_gpio",
      "available": true,
      "inputCapable": true,
      "outputCapable": true,
      "pullupCapable": true,
      "pulldownCapable": true,
      "analogCapable": false,
      "mode": "unconfigured",
      "pullup": false,
      "pulldown": false,
      "value": 0,
      "lastOutputValue": null
    }
  ],
  "count": 16
}
```

### Read GPIO

Read one supported GPIO:

```powershell
Invoke-RestMethod -Uri "http://<controller-ip>/api/gpio/read?pin=13"
```

If the pin has not been configured through REST yet, the controller automatically configures it as plain `input` without pull-up or pull-down before reading.

### Read Analog GPIO

Read one supported analog-capable GPIO:

```powershell
Invoke-RestMethod -Uri "http://<controller-ip>/api/gpio/analog?pin=34"
```

Example response:

```json
{
  "ok": true,
  "analog": {
    "pin": 34,
    "raw": 2048,
    "resolutionBits": 12,
    "maxRaw": 4095,
    "millivolts": 1650
  }
}
```

The `millivolts` field is included when the firmware platform supports calibrated millivolt reads. Use `GET /api/gpio` first and choose a pin with `analogCapable: true`; non-analog pins are rejected before hardware access.

### Configure GPIO

Configure one supported GPIO. For an input with internal pull-up:

```powershell
Invoke-RestMethod `
  -Uri http://<controller-ip>/api/gpio/configure `
  -Method Post `
  -ContentType "application/json" `
  -Body '{"pin":13,"mode":"input_pullup"}'
```

Configure an output and set its initial value:

```powershell
Invoke-RestMethod `
  -Uri http://<controller-ip>/api/gpio/configure `
  -Method Post `
  -ContentType "application/json" `
  -Body '{"pin":13,"mode":"output","value":0}'
```

Payload fields:

- `pin`: GPIO number from `GET /api/gpio`
- `mode`: one of the supported mode strings
- `value`: optional initial output value, `0` or `1`, used when configuring an output

### Write GPIO

Write a digital value to a pin that is already configured as output:

```powershell
Invoke-RestMethod `
  -Uri http://<controller-ip>/api/gpio/write `
  -Method Post `
  -ContentType "application/json" `
  -Body '{"pin":13,"value":1}'
```

Payload fields:

- `pin`: GPIO number from `GET /api/gpio`
- `value`: digital output value, `0` or `1`

If the pin has not been configured through REST yet, the controller automatically configures it as plain `output` without pull-up or pull-down before writing. If the pin was explicitly configured as an input, writes are rejected until it is configured as output.

## Smoke Test

1. Flash the firmware.
2. If the controller starts `wifi-i2c-setup`, connect to it and save WiFi credentials through the portal.
3. Reboot the controller.
4. Watch the serial monitor for the assigned station IP.
5. Call the I2C scan endpoint:

```powershell
Invoke-RestMethod -Uri http://<controller-ip>/api/i2c/scan
```

A successful response with `ok: true` confirms that the controller is in API mode and the I2C API is reachable. The returned device list depends on what is attached to the bus.

6. Call the GPIO list endpoint:

```powershell
Invoke-RestMethod -Uri http://<controller-ip>/api/gpio
```

A successful response with `ok: true` and a `pins` array confirms that the GPIO API is reachable.
