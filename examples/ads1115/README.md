# ADS1115 ADC over the I2C REST API

This example shows how to read an ADS1115 16-bit ADC through the controller's existing generic I2C REST API. It mirrors the style of the MCP4725 example, but focuses on register-style write-then-read transactions.

The ADS1115 uses four register pointers:

| Pointer | Register |
| --- | --- |
| `0x00` | Conversion register |
| `0x01` | Config register |
| `0x02` | Lo threshold |
| `0x03` | Hi threshold |

This example performs a single-shot conversion by writing the config register, waiting for the conversion to complete, then reading two bytes from the conversion register.

## Assumptions

- The controller is flashed, connected to WiFi, and running API mode.
- Replace `<controller-ip>` with the IP printed by the serial monitor.
- Common ADS1115 breakout boards use address `0x48`, but the ADDR pin can select `0x48` through `0x4B`.
- Default channel is `AIN0` measured single-ended against `GND`.
- Default gain is `±4.096 V`, so one bit is `4.096 / 32768 = 0.000125 V`.
- Default data rate is `128 SPS`, with a conservative `10 ms` wait before reading.
- Keep input voltages within the ADS1115 supply and selected full-scale range.

Typical wiring:

| ADS1115 | ESP32/controller |
| --- | --- |
| VDD | 3.3 V or board-supported supply |
| GND | GND |
| SDA | I2C SDA |
| SCL | I2C SCL |
| A0 | Analog signal to measure |

## 1. Scan the I2C bus

```powershell
Invoke-RestMethod -Uri http://<controller-ip>/api/i2c/scan
```

Look for the ADS1115 address in the returned device list. Common ADS1115 addresses are:

| ADDR pin | Address |
| --- | --- |
| GND | `0x48` |
| VDD | `0x49` |
| SDA | `0x4A` |
| SCL | `0x4B` |

The REST API accepts the address either as a number, such as `72`, or as a `0x`-prefixed string, such as `"0x48"`.

## 2. Configure and read AIN0

For default settings:

- single-shot conversion
- AIN0 single-ended
- gain `±4.096 V`
- single-shot mode
- `128 SPS`
- comparator disabled

the ADS1115 config register value is `0xC383`, sent as bytes `[1, 195, 131]` where `1` is the config register pointer.

```powershell
Invoke-RestMethod `
  -Uri http://<controller-ip>/api/i2c/write `
  -Method Post `
  -ContentType "application/json" `
  -Body '{"address":"0x48","bytes":[1,195,131]}'
```

Wait at least one conversion period, then read the conversion register with `write-read`. The prefix `[0]` selects the conversion register and `length: 2` reads the two conversion bytes.

```powershell
Invoke-RestMethod `
  -Uri http://<controller-ip>/api/i2c/write-read `
  -Method Post `
  -ContentType "application/json" `
  -Body '{"address":"0x48","prefix":[0],"length":2}'
```

Example response shape:

```json
{
  "ok": true,
  "address": 72,
  "hex": "0X48",
  "prefixWritten": 1,
  "requested": 2,
  "received": 2,
  "bytes": [25, 128]
}
```

## 3. Convert bytes to voltage

ADS1115 conversion bytes are a signed 16-bit big-endian value:

```text
raw = (msb << 8) | lsb
if raw >= 0x8000: raw -= 0x10000
volts = raw * fullScaleRange / 32768
```

With the default `±4.096 V` range, `[25, 128]` becomes:

```text
raw = 0x1980 = 6528
volts = 6528 * 4.096 / 32768 = 0.816 V
```

## 4. Node.js example

The directory contains a runnable Node.js example:

- `ads1115.js`: CLI for scan and single-shot reads
- `package.json`: npm scripts without third-party dependencies

Node.js 18 or newer is recommended because the script uses the built-in `fetch` API.

From this directory, scan the bus:

```powershell
npm run scan -- --host <controller-ip>
```

Read AIN0 with the default gain:

```powershell
npm run read -- --host <controller-ip>
```

Read another channel, address, gain, or data rate:

```powershell
npm run read -- --host <controller-ip> --address 0x49 --channel 2 --gain 2.048 --sps 250
```

You can also call the script directly:

```powershell
node .\ads1115.js --host <controller-ip> --scan --channel 0 --gain 4.096
```

## Notes

- These examples use single-ended channels `0` through `3`.
- Differential modes, comparator settings, and threshold registers are intentionally left out.
- If the response contains `ok: false`, check the `error`, `i2cStatus`, and `status` fields.
- If scan does not show the ADC, verify wiring, pull-ups, supply voltage, and ADDR pin wiring.
