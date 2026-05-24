# GPIO over the REST API

This example shows how to list, configure, read, write, and analog-read selected controller GPIOs through the GPIO REST API.

The firmware exposes only a safe allowlist of GPIOs. Always list pins first and choose a pin that is marked `outputCapable` before connecting external hardware or writing values. For analog reads, choose a pin marked `analogCapable`.

## Requirements

- The controller is flashed, connected to WiFi, and running API mode.
- Replace `<controller-ip>` with the IP printed by the serial monitor.
- Node.js 18 or newer is recommended because the script uses the built-in `fetch` API.

## List GPIOs

```powershell
npm run list -- --host <controller-ip>
```

Look for a pin with the capabilities needed for your use case:

- `inputCapable`
- `outputCapable`
- `pullupCapable`
- `pulldownCapable`
- `analogCapable`

## Configure An Input

Configure a pin as input with an internal pull-up:

```powershell
npm run configure -- --host <controller-ip> --pin 13 --mode input_pullup
```

Then read it:

```powershell
npm run read -- --host <controller-ip> --pin 13
```

If you read a supported pin before configuring it, the controller automatically configures it as plain `input` without pull-up or pull-down.

## Read An Analog Input

Read an analog-capable pin:

```powershell
npm run analog -- --host <controller-ip> --pin 34
```

Analog reads return a raw ADC value and resolution metadata. When supported by the firmware platform, the response also includes `millivolts`.

## Configure And Write An Output

Configure a pin as output with an initial low value:

```powershell
npm run configure -- --host <controller-ip> --pin 13 --mode output --value 0
```

Set it high:

```powershell
npm run write -- --host <controller-ip> --pin 13 --value 1
```

Set it low:

```powershell
npm run write -- --host <controller-ip> --pin 13 --value 0
```

If you write a supported output-capable pin before configuring it, the controller automatically configures it as plain `output` without pull-up or pull-down. If you already configured the pin as an input, configure it as output before writing.

You can also call the script directly:

```powershell
node .\gpio.js list --host <controller-ip>
node .\gpio.js configure --host <controller-ip> --pin 13 --mode output --value 0
node .\gpio.js write --host <controller-ip> --pin 13 --value 1
node .\gpio.js read --host <controller-ip> --pin 13
node .\gpio.js analog --host <controller-ip> --pin 34
```

## Notes

- Do not assume pin `13` is safe for every board. It is used in examples only because it is commonly available on ESP32 dev boards.
- Do not assume pin `34` is wired to your analog signal. It is used in examples because it is an ADC1-capable ESP32 input pin.
- Avoid connecting loads directly to GPIOs. Use appropriate resistors, drivers, or level shifting for your circuit.
- If a request fails, check the returned `error` and `detail` fields.
