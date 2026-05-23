# INA219 current, voltage, and power monitor over the I2C REST API

This example shows how to read an INA219 single-channel current, voltage, and power monitor through the controller's existing generic I2C REST API. The request called the device `IA219`; this example uses the common `INA219` part name.

The INA219 exposes these commonly used registers:

| Register | Name |
| --- | --- |
| `0x00` | Config |
| `0x01` | Shunt voltage |
| `0x02` | Bus voltage |
| `0x03` | Power |
| `0x04` | Current |
| `0x05` | Calibration |

Bus voltage and shunt voltage can be read directly. Current and power depend on the calibration register, so the Node.js example only reads calibrated current and power after `--calibrate` writes a calibration value.

## Assumptions

- The controller is flashed, connected to WiFi, and running API mode.
- Replace `<controller-ip>` with the IP printed by the serial monitor.
- The INA219 is connected to the ESP32 I2C bus used by the firmware.
- Common INA219 breakout boards use address `0x40`, but address jumpers can select other addresses.
- The default shunt resistor is assumed to be `0.1 ohm`.
- The default current LSB is `0.0001 A` per bit, also written as `100 uA` per bit.
- Keep bus voltage, shunt voltage, and current within the limits of your INA219 board and measured circuit.

Typical wiring:

| INA219 | ESP32/controller |
| --- | --- |
| VCC | 3.3 V or board-supported supply |
| GND | GND |
| SDA | I2C SDA |
| SCL | I2C SCL |
| VIN+ | Supply side of measured high-side path |
| VIN- | Load side of measured high-side path |

## 1. Scan the I2C bus

```powershell
Invoke-RestMethod -Uri http://<controller-ip>/api/i2c/scan
```

Look for the INA219 address in the returned device list. A common result is `0x40`:

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

Common INA219 breakout addresses include `0x40`, `0x41`, `0x44`, and `0x45`, depending on address jumpers.

The REST API accepts the address either as a number, such as `64`, or as a `0x`-prefixed string, such as `"0x40"`.

## 2. Read voltage registers

Read the shunt-voltage register `0x01`:

```powershell
Invoke-RestMethod `
  -Uri http://<controller-ip>/api/i2c/write-read `
  -Method Post `
  -ContentType "application/json" `
  -Body '{"address":"0x40","prefix":[1],"length":2}'
```

Read the bus-voltage register `0x02`:

```powershell
Invoke-RestMethod `
  -Uri http://<controller-ip>/api/i2c/write-read `
  -Method Post `
  -ContentType "application/json" `
  -Body '{"address":"0x40","prefix":[2],"length":2}'
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
  "bytes": [31, 160]
}
```

## 3. Optional calibration for current and power

Current register `0x04` and power register `0x03` depend on the calibration register `0x05`.

The calibration value is:

```text
calibration = trunc(0.04096 / (current_lsb_amps * shunt_ohms))
```

With `current_lsb_amps = 0.0001` and `shunt_ohms = 0.1`:

```text
calibration = trunc(0.04096 / (0.0001 * 0.1)) = 4096 = 0x1000
```

Write that to register `0x05` as `[5, 16, 0]`:

```powershell
Invoke-RestMethod `
  -Uri http://<controller-ip>/api/i2c/write `
  -Method Post `
  -ContentType "application/json" `
  -Body '{"address":"0x40","bytes":[5,16,0]}'
```

Then read current and power with `write-read`:

```powershell
Invoke-RestMethod `
  -Uri http://<controller-ip>/api/i2c/write-read `
  -Method Post `
  -ContentType "application/json" `
  -Body '{"address":"0x40","prefix":[4],"length":2}'
```

```powershell
Invoke-RestMethod `
  -Uri http://<controller-ip>/api/i2c/write-read `
  -Method Post `
  -ContentType "application/json" `
  -Body '{"address":"0x40","prefix":[3],"length":2}'
```

## 4. Convert bytes to values

Combine the two bytes as a 16-bit big-endian register value:

```text
register = (msb << 8) | lsb
```

For shunt voltage, interpret the register as signed 16-bit and multiply by `10 uV`:

```text
shunt_volts = signed16(register) * 0.000010
```

For bus voltage, shift away the three status/reserved low bits and multiply by `4 mV`:

```text
bus_code = register >> 3
bus_volts = bus_code * 0.004
```

For calibrated current:

```text
current_amps = signed16(current_register) * current_lsb_amps
```

For calibrated power:

```text
power_watts = power_register * 20 * current_lsb_amps
```

## 5. Node.js example

The directory contains a runnable Node.js example:

- `ina219.js`: CLI for scan, config reads, voltage reads, and optional calibrated current/power reads
- `package.json`: npm scripts without third-party dependencies

Node.js 18 or newer is recommended because the script uses the built-in `fetch` API.

From this directory, scan the bus:

```powershell
npm run scan -- --host <controller-ip>
```

Read bus and shunt voltage:

```powershell
npm run read -- --host <controller-ip>
```

Read config and calibration registers:

```powershell
npm run config -- --host <controller-ip>
```

Write calibration and read voltage, current, and power:

```powershell
npm run read:calibrated -- --host <controller-ip> --shunt-ohms 0.1 --current-lsb 0.0001
```

Use another I2C address if your board differs:

```powershell
npm run read -- --host <controller-ip> --address 0x41
```

You can also call the script directly:

```powershell
node .\ina219.js --host <controller-ip> --scan --config --calibrate --shunt-ohms 0.1
```

## Notes

- `--calibrate` writes only the INA219 calibration register `0x05`.
- Current and power are shown as `n/a` unless `--calibrate` is supplied.
- If the response contains `ok: false`, check the `error`, `i2cStatus`, and `status` fields.
- If scan does not show the monitor, verify wiring, pull-ups, supply voltage, and address pin configuration.
