# GPIO over the REST API

This example shows how to list, configure, read, write, ADC-read, DAC-write, and PWM-drive selected controller GPIOs through the GPIO REST API.

The firmware exposes only a safe allowlist of GPIOs. Always list pins first and choose a pin that is marked `outputCapable` before connecting external hardware or writing values. For ADC reads, choose a pin marked `adcCapable`. For internal DAC writes, choose a pin marked `dacCapable`. For PWM output, choose a pin marked `pwmCapable`.

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
- `adcCapable`
- `dacCapable`
- `pwmCapable`

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

Add `--path-style` to use routes such as `/api/gpio/pin/13/read` instead of the compatibility endpoints:

```powershell
npm run read -- --host <controller-ip> --pin 13 --path-style
```

## Read An ADC Input

Read an ADC-capable pin:

```powershell
npm run adc -- --host <controller-ip> --pin 34
```

The path-style equivalent calls `/api/gpio/pin/34/adc`:

```powershell
npm run adc -- --host <controller-ip> --pin 34 --path-style
```

ADC reads return a raw value and resolution metadata. When supported by the firmware platform, the response also includes `millivolts`.

## Write An Internal DAC Output

Write an 8-bit raw DAC value to a DAC-capable pin:

```powershell
npm run dac -- --host <controller-ip> --pin 25 --value 128
```

Use path-style routing when you want the pin in the URL:

```powershell
npm run dac -- --host <controller-ip> --pin 25 --value 128 --path-style
```

The ESP32 internal DAC is available on GPIO25 and GPIO26. Values are raw 8-bit values from `0` through `255`; use an external DAC when you need higher resolution or better precision.

## Drive PWM Output

Start PWM on a PWM-capable output pin:

```powershell
npm run pwm -- --host <controller-ip> --pin 13 --frequency 1000 --resolution 10 --duty 512
```

Read the REST-managed PWM state:

```powershell
npm run pwm-read -- --host <controller-ip> --pin 13
```

Update PWM by running `pwm` again with new values:

```powershell
npm run pwm -- --host <controller-ip> --pin 13 --frequency 2000 --resolution 10 --duty 256
```

Stop PWM:

```powershell
npm run pwm-stop -- --host <controller-ip> --pin 13
```

Use path-style routing when you want the pin in the URL:

```powershell
npm run pwm -- --host <controller-ip> --pin 13 --frequency 1000 --resolution 10 --duty 512 --path-style
npm run pwm-read -- --host <controller-ip> --pin 13 --path-style
npm run pwm-stop -- --host <controller-ip> --pin 13 --path-style
```

PWM uses ESP32 LEDC hardware PWM channels. The `duty` value is raw and depends on `resolution`: with `--resolution 10`, valid duty values are `0` through `1023`.

## Drive A Servo

Most hobby servos expect a 50 Hz PWM signal with a pulse width around `500` to `2500` microseconds. The `servo` command maps an angle to that pulse range and sends the matching PWM request.

Move to the center position:

```powershell
npm run servo -- --host <controller-ip> --pin 13 --angle 90
```

Move by explicit pulse width:

```powershell
npm run servo -- --host <controller-ip> --pin 13 --pulse-us 1500
```

Use path-style routing:

```powershell
npm run servo -- --host <controller-ip> --pin 13 --angle 45 --path-style
```

Tune the pulse range for your servo if needed:

```powershell
npm run servo -- --host <controller-ip> --pin 13 --angle 180 --min-pulse 600 --max-pulse 2400
```

Power servos from a suitable external supply, connect grounds together, and avoid powering a servo directly from the ESP32 pin or regulator.

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

Path-style routing works for configure and write as well:

```powershell
npm run configure -- --host <controller-ip> --pin 13 --mode output --value 0 --path-style
npm run write -- --host <controller-ip> --pin 13 --value 1 --path-style
```

You can also call the script directly:

```powershell
node .\gpio.js list --host <controller-ip>
node .\gpio.js configure --host <controller-ip> --pin 13 --mode output --value 0
node .\gpio.js write --host <controller-ip> --pin 13 --value 1
node .\gpio.js read --host <controller-ip> --pin 13
node .\gpio.js adc --host <controller-ip> --pin 34 --path-style
node .\gpio.js dac --host <controller-ip> --pin 25 --value 128
node .\gpio.js pwm --host <controller-ip> --pin 13 --frequency 1000 --resolution 10 --duty 512
node .\gpio.js servo --host <controller-ip> --pin 13 --angle 90
node .\gpio.js pwm-stop --host <controller-ip> --pin 13
```

## Notes

- Do not assume pin `13` is safe for every board. It is used in examples only because it is commonly available on ESP32 dev boards.
- Do not assume pin `34` is wired to your input signal. It is used in examples because it is an ADC1-capable ESP32 input pin.
- Do not assume pin `25` is free on your board. It is used in examples because it is an ESP32 DAC-capable pin.
- PWM can immediately drive attached hardware. Use appropriate resistors, drivers, or level shifting, especially for motors, LEDs, fans, and servos.
- Servos usually need an external power supply. Connect the servo supply ground to the controller ground.
- Avoid connecting loads directly to GPIOs. Use appropriate resistors, drivers, or level shifting for your circuit.
- If a request fails, check the returned `error` and `detail` fields.
