#!/usr/bin/env node
'use strict';

const DEFAULT_ADDRESS = '0x60';
const DEFAULT_VCC = 3.3;

function printUsage() {
  console.log(`Usage:
  node mcp4725.js --host <controller-ip> --scan
  node mcp4725.js --host <controller-ip> --value <0..4095>
  node mcp4725.js --host <controller-ip> --scan --value 2048

Options:
  --host <ip>       Controller IP or hostname. Can also use CONTROLLER_IP.
  --address <addr>  MCP4725 I2C address. Default: ${DEFAULT_ADDRESS}
  --value <n>       12-bit DAC value, 0..4095.
  --vcc <volts>     DAC supply/reference voltage for output estimate. Default: ${DEFAULT_VCC}
  --scan            Scan the I2C bus before any write.
  --help            Show this help.

Examples:
  node mcp4725.js --host 192.168.178.51 --scan
  node mcp4725.js --host 192.168.178.51 --value 2048
  CONTROLLER_IP=192.168.178.51 npm run midscale
`);
}

function parseArgs(argv) {
  const options = {
    host: process.env.CONTROLLER_IP || '',
    address: process.env.MCP4725_ADDRESS || DEFAULT_ADDRESS,
    value: undefined,
    vcc: Number(process.env.MCP4725_VCC || DEFAULT_VCC),
    scan: false,
  };

  for (let i = 0; i < argv.length; i += 1) {
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
      case '--address':
        options.address = next();
        break;
      case '--value':
        options.value = Number(next());
        break;
      case '--vcc':
        options.vcc = Number(next());
        break;
      case '--scan':
        options.scan = true;
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

function mcp4725FastModeBytes(value) {
  if (!Number.isInteger(value) || value < 0 || value > 4095) {
    throw new Error('value must be an integer between 0 and 4095');
  }

  return [
    (value >> 8) & 0x0f,
    value & 0xff,
  ];
}

function estimateVoltage(value, vcc) {
  if (!Number.isFinite(vcc) || vcc <= 0) {
    throw new Error('vcc must be a positive number');
  }

  return (vcc * value) / 4095;
}

async function requestJson(host, path, options = {}) {
  const response = await fetch(`http://${host}${path}`, options);
  const body = await response.json();

  if (!response.ok || body.ok === false) {
    throw new Error(`REST request failed: ${JSON.stringify(body)}`);
  }

  return body;
}

async function scanBus(host) {
  const scan = await requestJson(host, '/api/i2c/scan');
  console.log(JSON.stringify(scan, null, 2));
  return scan;
}

async function writeValue(host, address, value, vcc) {
  const bytes = mcp4725FastModeBytes(value);
  const voltage = estimateVoltage(value, vcc);

  console.log(
    `Writing MCP4725 value ${value} to ${address} as bytes [${bytes.join(', ')}]`,
  );
  console.log(`Approximate output at VCC=${vcc} V: ${voltage.toFixed(3)} V`);

  const result = await requestJson(host, '/api/i2c/write', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify({
      address,
      bytes,
    }),
  });

  console.log(JSON.stringify(result, null, 2));
  return result;
}

async function main() {
  const options = parseArgs(process.argv.slice(2));

  if (options.help) {
    printUsage();
    return;
  }

  if (!options.host) {
    throw new Error('Missing --host <controller-ip> or CONTROLLER_IP');
  }

  if (!options.scan && options.value === undefined) {
    printUsage();
    throw new Error('Choose --scan and/or --value <0..4095>');
  }

  if (options.scan) {
    await scanBus(options.host);
  }

  if (options.value !== undefined) {
    await writeValue(options.host, options.address, options.value, options.vcc);
  }
}

main().catch((error) => {
  console.error(error.message);
  process.exitCode = 1;
});
