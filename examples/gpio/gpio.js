#!/usr/bin/env node
'use strict';

function printUsage() {
  console.log(`Usage:
  node gpio.js list --host <controller-ip>
  node gpio.js read --host <controller-ip> --pin <n>
  node gpio.js adc --host <controller-ip> --pin <n>
  node gpio.js dac --host <controller-ip> --pin <n> --value 0..255
  node gpio.js pwm --host <controller-ip> --pin <n> --frequency <hz> --resolution <bits> --duty <raw>
  node gpio.js pwm-read --host <controller-ip> --pin <n>
  node gpio.js pwm-stop --host <controller-ip> --pin <n>
  node gpio.js servo --host <controller-ip> --pin <n> (--angle 0..180 | --pulse-us <us>)
  node gpio.js configure --host <controller-ip> --pin <n> --mode <mode> [--value 0|1]
  node gpio.js write --host <controller-ip> --pin <n> --value 0|1

Options:
  --host <ip>    Controller IP or hostname. Can also use CONTROLLER_IP.
  --pin <n>      GPIO number from GET /api/gpio.
  --mode <mode>  input, input_pullup, input_pulldown, output, output_open_drain.
  --value <n>    Digital output value 0|1, or DAC value 0..255.
  --frequency <hz> PWM frequency in hertz.
  --resolution <n> PWM duty resolution bits.
  --duty <n>     Raw PWM duty value from 0 to (2^resolution)-1.
  --angle <n>    Servo angle 0..180, mapped to min/max pulse width.
  --pulse-us <n> Servo pulse width in microseconds.
  --min-pulse <n> Servo pulse for 0 degrees. Default: 500.
  --max-pulse <n> Servo pulse for 180 degrees. Default: 2500.
  --path-style   Use /api/gpio/pin/<pin>/<operation> for pin-specific requests.
  --help         Show this help.

Examples:
  node gpio.js list --host 192.168.178.51
  node gpio.js configure --host 192.168.178.51 --pin 13 --mode output --value 0
  node gpio.js write --host 192.168.178.51 --pin 13 --value 1
  node gpio.js read --host 192.168.178.51 --pin 13
  node gpio.js adc --host 192.168.178.51 --pin 34
  node gpio.js adc --host 192.168.178.51 --pin 34 --path-style
  node gpio.js dac --host 192.168.178.51 --pin 25 --value 128
  node gpio.js pwm --host 192.168.178.51 --pin 13 --frequency 1000 --resolution 10 --duty 512
  node gpio.js pwm-read --host 192.168.178.51 --pin 13 --path-style
  node gpio.js pwm-stop --host 192.168.178.51 --pin 13
  node gpio.js servo --host 192.168.178.51 --pin 13 --angle 90 --path-style
`);
}

function parseArgs(argv) {
  const options = {
    command: argv[0] || '',
    host: process.env.CONTROLLER_IP || '',
    pin: undefined,
    mode: '',
    value: undefined,
    frequency: undefined,
    resolution: undefined,
    duty: undefined,
    angle: undefined,
    pulseUs: undefined,
    minPulse: 500,
    maxPulse: 2500,
    pathStyle: false,
    help: false,
  };

  if (options.command === '--help' || options.command === '-h') {
    options.command = '';
    options.help = true;
    return options;
  }

  for (let i = 1; i < argv.length; i += 1) {
    const arg = argv[i];
    const next = () => {
      i += 1;
      if (i >= argv.length) {
        throw new Error(`${arg} requires a value`);
      }
      return argv[i];
    };

    switch (arg) {
      case '--host':
        options.host = next();
        break;
      case '--pin':
        options.pin = Number(next());
        break;
      case '--mode':
        options.mode = next();
        break;
      case '--value':
        options.value = Number(next());
        break;
      case '--frequency':
        options.frequency = Number(next());
        break;
      case '--resolution':
        options.resolution = Number(next());
        break;
      case '--duty':
        options.duty = Number(next());
        break;
      case '--angle':
        options.angle = Number(next());
        break;
      case '--pulse-us':
        options.pulseUs = Number(next());
        break;
      case '--min-pulse':
        options.minPulse = Number(next());
        break;
      case '--max-pulse':
        options.maxPulse = Number(next());
        break;
      case '--path-style':
        options.pathStyle = true;
        break;
      case '--help':
      case '-h':
        options.help = true;
        break;
      default:
        throw new Error(`Unknown argument: ${arg}`);
    }
  }

  return options;
}

function requireHost(options) {
  if (!options.host) {
    throw new Error('Missing --host <controller-ip> or CONTROLLER_IP');
  }
}

function requirePin(options) {
  if (!Number.isInteger(options.pin) || options.pin < 0) {
    throw new Error('Missing or invalid --pin <n>');
  }
}

function requireValue(options) {
  if (options.value !== 0 && options.value !== 1) {
    throw new Error('Missing or invalid --value 0|1');
  }
}

function requireDacValue(options) {
  if (!Number.isInteger(options.value) || options.value < 0 || options.value > 255) {
    throw new Error('Missing or invalid --value 0..255');
  }
}

function requirePwmOptions(options) {
  if (!Number.isInteger(options.frequency) || options.frequency < 1) {
    throw new Error('Missing or invalid --frequency <hz>');
  }
  if (!Number.isInteger(options.resolution) || options.resolution < 1 || options.resolution > 16) {
    throw new Error('Missing or invalid --resolution 1..16');
  }

  const maxDuty = (2 ** options.resolution) - 1;
  if (!Number.isInteger(options.duty) || options.duty < 0 || options.duty > maxDuty) {
    throw new Error(`Missing or invalid --duty 0..${maxDuty}`);
  }
}

function requireServoOptions(options) {
  if (!Number.isInteger(options.minPulse) || options.minPulse < 1) {
    throw new Error('Missing or invalid --min-pulse <us>');
  }
  if (!Number.isInteger(options.maxPulse) || options.maxPulse <= options.minPulse) {
    throw new Error('Missing or invalid --max-pulse <us>');
  }

  const hasAngle = options.angle !== undefined;
  const hasPulse = options.pulseUs !== undefined;
  if (hasAngle === hasPulse) {
    throw new Error('Use exactly one of --angle 0..180 or --pulse-us <us>');
  }

  if (hasAngle && (!Number.isFinite(options.angle) || options.angle < 0 || options.angle > 180)) {
    throw new Error('Missing or invalid --angle 0..180');
  }

  if (hasPulse && (!Number.isInteger(options.pulseUs) ||
      options.pulseUs < options.minPulse ||
      options.pulseUs > options.maxPulse)) {
    throw new Error(`Missing or invalid --pulse-us ${options.minPulse}..${options.maxPulse}`);
  }
}

function servoPulseUs(options) {
  if (options.pulseUs !== undefined) {
    return options.pulseUs;
  }

  return Math.round(
    options.minPulse + ((options.maxPulse - options.minPulse) * options.angle) / 180,
  );
}

function dutyFromPulseUs(pulseUs, frequencyHz, resolutionBits) {
  const maxDuty = (2 ** resolutionBits) - 1;
  return Math.round((pulseUs * frequencyHz * maxDuty) / 1000000);
}

async function requestJson(host, path, options = {}) {
  const response = await fetch(`http://${host}${path}`, options);
  const body = await response.json();

  if (!response.ok || body.ok === false) {
    throw new Error(`REST request failed: ${JSON.stringify(body)}`);
  }

  return body;
}

async function postJson(host, path, body) {
  return requestJson(host, path, {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify(body),
  });
}

function gpioOperationPath(options, operation, queryPath) {
  if (!options.pathStyle) {
    return queryPath;
  }

  return `/api/gpio/pin/${options.pin}/${operation}`;
}

function printPins(response) {
  if (!Array.isArray(response.pins)) {
    console.log(JSON.stringify(response, null, 2));
    return;
  }

  console.table(response.pins.map((pin) => ({
    pin: pin.pin,
    mode: pin.mode,
    value: pin.value,
    input: pin.inputCapable,
    output: pin.outputCapable,
    pullup: pin.pullupCapable,
    pulldown: pin.pulldownCapable,
    adc: pin.adcCapable,
    dac: pin.dacCapable,
    pwm: pin.pwmCapable,
    pwmActive: pin.pwm && pin.pwm.active,
    lastDac: pin.lastDacValue,
    label: pin.label,
  })));
  console.log(JSON.stringify(response, null, 2));
}

async function main() {
  const options = parseArgs(process.argv.slice(2));

  if (options.help || !options.command) {
    printUsage();
    return;
  }

  requireHost(options);

  switch (options.command) {
    case 'list': {
      const response = await requestJson(options.host, '/api/gpio');
      printPins(response);
      return;
    }
    case 'read': {
      requirePin(options);
      const response = await requestJson(
        options.host,
        gpioOperationPath(options, 'read', `/api/gpio/read?pin=${options.pin}`),
      );
      console.log(JSON.stringify(response, null, 2));
      return;
    }
    case 'adc': {
      requirePin(options);
      const response = await requestJson(
        options.host,
        gpioOperationPath(options, 'adc', `/api/gpio/adc?pin=${options.pin}`),
      );
      console.log(JSON.stringify(response, null, 2));
      return;
    }
    case 'dac': {
      requirePin(options);
      requireDacValue(options);
      const body = {
        value: options.value,
      };
      if (!options.pathStyle) {
        body.pin = options.pin;
      }
      const response = await postJson(
        options.host,
        gpioOperationPath(options, 'dac', '/api/gpio/dac'),
        body,
      );
      console.log(JSON.stringify(response, null, 2));
      return;
    }
    case 'pwm': {
      requirePin(options);
      requirePwmOptions(options);
      const body = {
        frequencyHz: options.frequency,
        resolutionBits: options.resolution,
        duty: options.duty,
      };
      if (!options.pathStyle) {
        body.pin = options.pin;
      }
      const response = await postJson(
        options.host,
        gpioOperationPath(options, 'pwm', '/api/gpio/pwm'),
        body,
      );
      console.log(JSON.stringify(response, null, 2));
      return;
    }
    case 'pwm-read': {
      requirePin(options);
      const response = await requestJson(
        options.host,
        gpioOperationPath(options, 'pwm', `/api/gpio/pwm?pin=${options.pin}`),
      );
      console.log(JSON.stringify(response, null, 2));
      return;
    }
    case 'pwm-stop': {
      requirePin(options);
      const body = {};
      if (!options.pathStyle) {
        body.pin = options.pin;
      }
      const response = await postJson(
        options.host,
        gpioOperationPath(options, 'pwm/stop', '/api/gpio/pwm/stop'),
        body,
      );
      console.log(JSON.stringify(response, null, 2));
      return;
    }
    case 'servo': {
      requirePin(options);
      requireServoOptions(options);
      const frequencyHz = 50;
      const resolutionBits = 16;
      const pulseUs = servoPulseUs(options);
      const duty = dutyFromPulseUs(pulseUs, frequencyHz, resolutionBits);
      const body = {
        frequencyHz,
        resolutionBits,
        duty,
      };
      if (!options.pathStyle) {
        body.pin = options.pin;
      }
      const response = await postJson(
        options.host,
        gpioOperationPath(options, 'pwm', '/api/gpio/pwm'),
        body,
      );
      console.log(JSON.stringify({
        servo: {
          pin: options.pin,
          angle: options.angle ?? null,
          pulseUs,
          frequencyHz,
          resolutionBits,
          duty,
        },
        response,
      }, null, 2));
      return;
    }
    case 'configure': {
      requirePin(options);
      if (!options.mode) {
        throw new Error('Missing --mode <mode>');
      }
      const body = {
        mode: options.mode,
      };
      if (!options.pathStyle) {
        body.pin = options.pin;
      }
      if (options.value !== undefined) {
        requireValue(options);
        body.value = options.value;
      }
      const response = await postJson(
        options.host,
        gpioOperationPath(options, 'configure', '/api/gpio/configure'),
        body,
      );
      console.log(JSON.stringify(response, null, 2));
      return;
    }
    case 'write': {
      requirePin(options);
      requireValue(options);
      const body = {
        value: options.value,
      };
      if (!options.pathStyle) {
        body.pin = options.pin;
      }
      const response = await postJson(
        options.host,
        gpioOperationPath(options, 'write', '/api/gpio/write'),
        body,
      );
      console.log(JSON.stringify(response, null, 2));
      return;
    }
    default:
      printUsage();
      throw new Error(`Unknown command: ${options.command}`);
  }
}

main().catch((error) => {
  console.error(error.message);
  process.exitCode = 1;
});
