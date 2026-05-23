# MCP4725 DAC over the I2C REST API

This example shows how to drive an MCP4725 12-bit DAC through the controller's existing generic I2C REST API. The folder name is `mcp4425` to match the requested path, but the device used here is the MCP4725.

The example uses MCP4725 fast-mode DAC writes. In fast mode, the MCP4725 accepts two data bytes after the I2C address:

- byte 1: `0b00PPDDDD`, where `PP` are power-down bits and `DDDD` are DAC bits `D11..D8`
- byte 2: DAC bits `D7..D0`

For normal operation, use power-down bits `00`, so the bytes for a 12-bit value are:

```text
high = (value >> 8) & 0x0F
low  = value & 0xFF
```

## Assumptions

- The controller is flashed, connected to WiFi, and running API mode.
- Replace `<controller-ip>` with the IP printed by the serial monitor.
- The MCP4725 is connected to the ESP32 I2C bus used by the firmware.
- Common MCP4725 breakout boards use address `0x60`, but you should confirm with a scan.
- The DAC output range is approximately `0 V` to `VCC`.
- Start with safe low or mid-scale values and verify wiring before writing full-scale output.

Typical wiring:

| MCP4725 | ESP32/controller |
| --- | --- |
| VCC | 3.3 V or board-supported supply |
| GND | GND |
| SDA | I2C SDA |
| SCL | I2C SCL |

## 1. Scan the I2C bus

```powershell
Invoke-RestMethod -Uri http://<controller-ip>/api/i2c/scan
```

Look for the MCP4725 address in the returned device list. A common result is `0x60`:

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

The REST API accepts the address either as a number, such as `96`, or as a `0x`-prefixed string, such as `"0x60"`.

## 2. Write DAC output values

The MCP4725 has a 12-bit input code from `0` to `4095`.

Approximate output voltage:

```text
Vout = VCC * value / 4095
```

With `VCC = 3.3 V`, the examples below are approximately:

| Target | 12-bit value | Bytes | Approx. output |
| --- | ---: | --- | ---: |
| Zero scale | `0` | `[0, 0]` | `0.00 V` |
| Mid scale | `2048` | `[8, 0]` | `1.65 V` |
| Full scale | `4095` | `[15, 255]` | `3.30 V` |

### Zero scale

```powershell
Invoke-RestMethod `
  -Uri http://<controller-ip>/api/i2c/write `
  -Method Post `
  -ContentType "application/json" `
  -Body '{"address":"0x60","bytes":[0,0]}'
```

### Mid scale

```powershell
Invoke-RestMethod `
  -Uri http://<controller-ip>/api/i2c/write `
  -Method Post `
  -ContentType "application/json" `
  -Body '{"address":"0x60","bytes":[8,0]}'
```

### Full scale

```powershell
Invoke-RestMethod `
  -Uri http://<controller-ip>/api/i2c/write `
  -Method Post `
  -ContentType "application/json" `
  -Body '{"address":"0x60","bytes":[15,255]}'
```

Example successful response:

```json
{
  "ok": true,
  "address": 96,
  "hex": "0X60",
  "bytesWritten": 2,
  "i2cStatus": 0,
  "status": "ok"
}
```

## 3. Generate bytes for another value

Use this PowerShell snippet to calculate the two MCP4725 fast-mode bytes for any 12-bit value:

```powershell
$value = 1234
if ($value -lt 0 -or $value -gt 4095) {
  throw "value must be between 0 and 4095"
}

$high = ($value -shr 8) -band 0x0F
$low = $value -band 0xFF
@($high, $low)
```

Then place the generated values in the `bytes` array:

```powershell
Invoke-RestMethod `
  -Uri http://<controller-ip>/api/i2c/write `
  -Method Post `
  -ContentType "application/json" `
  -Body '{"address":"0x60","bytes":[4,210]}'
```

For `value = 1234`, the approximate output at `3.3 V` is:

```text
3.3 * 1234 / 4095 = 0.99 V
```

## 4. Node.js example

The directory also contains a runnable Node.js example:

- `mcp4725.js`: CLI for scan and write operations
- `package.json`: npm scripts without third-party dependencies

Node.js 18 or newer is recommended because the script uses the built-in `fetch` API.

From this directory, scan the bus:

```powershell
npm run scan -- --host <controller-ip>
```

Set a mid-scale output:

```powershell
npm run write -- --host <controller-ip> --value 2048
```

Use another I2C address or supply voltage if your hardware differs:

```powershell
npm run write -- --host <controller-ip> --address 0x61 --value 1234 --vcc 5
```

You can also call the script directly:

```powershell
node .\mcp4725.js --host <controller-ip> --scan --value 2048
```

## Notes

- These examples write the volatile DAC register. They do not intentionally write MCP4725 EEPROM.
- If the response contains `ok: false`, check the `error`, `i2cStatus`, and `status` fields.
- If scan does not show the DAC, verify wiring, pull-ups, supply voltage, and the board's address configuration.
