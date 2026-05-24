#!/usr/bin/env node
'use strict';

function printUsage() {
  console.log(`Usage:
  node gpio.js list --host <controller-ip>
  node gpio.js read --host <controller-ip> --pin <n>
  node gpio.js adc --host <controller-ip> --pin <n>
  node gpio.js dac --host <controller-ip> --pin <n> --value 0..255
  node gpio.js configure --host <controller-ip> --pin <n> --mode <mode> [--value 0|1]
  node gpio.js write --host <controller-ip> --pin <n> --value 0|1

Options:
  --host <ip>    Controller IP or hostname. Can also use CONTROLLER_IP.
  --pin <n>      GPIO number from GET /api/gpio.
  --mode <mode>  input, input_pullup, input_pulldown, output, output_open_drain.
  --value <n>    Digital output value 0|1, or DAC value 0..255.
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
`);
}

function parseArgs(argv) {
  const options = {
    command: argv[0] || '',
    host: process.env.CONTROLLER_IP || '',
    pin: undefined,
    mode: '',
    value: undefined,
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
