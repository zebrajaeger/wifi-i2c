# INA3221 current and voltage monitor over the I2C REST API

This example shows how to read an INA3221 three-channel current and bus-voltage monitor through the controller's existing generic I2C REST API. It mirrors the MCP4725 and ADS1115 examples, but focuses on register-style measurement reads.

The INA3221 exposes each channel as a shunt-voltage register plus a bus-voltage register:

| Channel | Shunt register | Bus register |
| --- | --- | --- |
| 1 | `0x01` | `0x02` |
| 2 | `0x03` | `0x04` |
| 3 | `0x05` | `0x06` |

The config register is at `0x00`. This example reads measurements only; it does not write configuration registers.

## Assumptions

- The controller is flashed, connected to WiFi, and running API mode.
- Replace `<controller-ip>` with the IP printed by the serial monitor.
- The INA3221 is connected to the ESP32 I2C bus used by the firmware.
- Common INA3221 breakout boards use address `0x40`, but board address pins can select other addresses.
- Channels are named `1`, `2`, and `3`, matching the INA3221 datasheet register groups.
- Current calculation requires your external shunt resistor value in ohms.
- Keep bus voltages and shunt voltages within the limits of your INA3221 board and connected circuit.

Typical wiring:

| INA3221 | ESP32/controller |
| --- | --- |
| VCC | 3.3 V or board-supported supply |
| GND | GND |
| SDA | I2C SDA |
| SCL | I2C SCL |
| CHx+ / CHx- | High-side shunt sense path for the measured channel |

## 1. Scan the I2C bus

```powershell
Invoke-RestMethod -Uri http://<controller-ip>/api/i2c/scan
```

Look for the INA3221 address in the returned device list. A common result is `0x40`:

```json
{
  "ok": true,
  "devices": [
    {
      "address": 64,
      "hex": "0X40"
    }
  ],
  "count": 1
}
```

The REST API accepts the address either as a number, such as `64`, or as a `0x`-prefixed string, such as `"0x40"`.

## 2. Read the config register

The INA3221 config register is register pointer `0x00`.

```powershell
Invoke-RestMethod `
  -Uri http://<controller-ip>/api/i2c/write-read `
  -Method Post `
  -ContentType "application/json" `
  -Body '{"address":"0x40","prefix":[0],"length":2}'
```

Example response shape:

```json
{
  "ok": true,
  "address": 64,
  "hex": "0X40",
  "prefixWritten": 1,
  "requested": 2,
  "received": 2,
  "bytes": [113, 39]
}
```

## 3. Read channel measurements

Each measurement is a two-byte big-endian register read. For channel 1, read the shunt-voltage register `0x01` and the bus-voltage register `0x02`.

```powershell
Invoke-RestMethod `
  -Uri http://<controller-ip>/api/i2c/write-read `
  -Method Post `
  -ContentType "application/json" `
  -Body '{"address":"0x40","prefix":[1],"length":2}'
```

```powershell
Invoke-RestMethod `
  -Uri http://<controller-ip>/api/i2c/write-read `
  -Method Post `
  -ContentType "application/json" `
  -Body '{"address":"0x40","prefix":[2],"length":2}'
```

Use register pairs `0x03`/`0x04` for channel 2 and `0x05`/`0x06` for channel 3.

## 4. Convert bytes to values

Combine the two bytes as a 16-bit big-endian register value:

```text
register = (msb << 8) | lsb
```

For shunt voltage, sign-extend the 16-bit value, shift away the three reserved low bits, and multiply by `40 uV`:

```text
signed = register
if signed >= 0x8000: signed -= 0x10000
shunt_code = signed >> 3
shunt_volts = shunt_code * 0.000040
```

For bus voltage, shift away the three reserved low bits and multiply by `8 mV`:

```text
bus_code = register >> 3
bus_volts = bus_code * 0.008
```

If you know the shunt resistor value, calculate current with Ohm's law:

```text
current_amps = shunt_volts / shunt_ohms
```

For example, with `shunt_volts = 0.004 V` and a `0.1 ohm` shunt:

```text
0.004 / 0.1 = 0.04 A
```

## 5. Node.js example

The directory contains a runnable Node.js example:

- `ina3221.js`: CLI for scan, config reads, single-channel reads, and all-channel reads
- `package.json`: npm scripts without third-party dependencies

Node.js 18 or newer is recommended because the script uses the built-in `fetch` API.

From this directory, scan the bus:

```powershell
npm run scan -- --host <controller-ip>
```

Read channel 1:

```powershell
npm run read -- --host <controller-ip> --channel 1
```

Read all three channels:

```powershell
npm run read:all -- --host <controller-ip>
```

Read all channels and estimate current with a `0.1 ohm` shunt:

```powershell
npm run read:all -- --host <controller-ip> --shunt-ohms 0.1
```

Use another I2C address if your board differs:

```powershell
npm run read -- --host <controller-ip> --address 0x41 --channel 2
```

You can also call the script directly:

```powershell
node .\ina3221.js --host <controller-ip> --scan --config --all --shunt-ohms 0.1
```

## Notes

- The example is observational by default: it reads registers and does not write INA3221 configuration.
- Current values are shown as `n/a` unless `--shunt-ohms` is supplied.
- If the response contains `ok: false`, check the `error`, `i2cStatus`, and `status` fields.
- If scan does not show the monitor, verify wiring, pull-ups, supply voltage, and address pin configuration.
