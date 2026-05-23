#!/usr/bin/env node
'use strict';

const DEFAULT_ADDRESS = '0x40';

const REG_CONFIG = 0x00;
const CHANNEL_REGISTERS = {
  1: { shunt: 0x01, bus: 0x02 },
  2: { shunt: 0x03, bus: 0x04 },
  3: { shunt: 0x05, bus: 0x06 },
};

const SHUNT_LSB_VOLTS = 40e-6;
const BUS_LSB_VOLTS = 8e-3;

function printUsage() {
  console.log(`Usage:
  node ina3221.js --host <controller-ip> --scan
  node ina3221.js --host <controller-ip> --channel <1..3>
  node ina3221.js --host <controller-ip> --all
  node ina3221.js --host <controller-ip> --scan --all --shunt-ohms 0.1

Options:
  --host <ip>          Controller IP or hostname. Can also use CONTROLLER_IP.
  --address <addr>     INA3221 I2C address. Default: ${DEFAULT_ADDRESS}
  --channel <n>        Channel 1..3 to read. Default: 1.
  --all                Read channels 1, 2, and 3.
  --shunt-ohms <ohms>  Optional shunt resistor value for current estimates.
  --config             Read and print the config register before measurements.
  --scan               Scan the I2C bus before any read.
  --help               Show this help.

Examples:
  node ina3221.js --host 192.168.178.51 --scan
  node ina3221.js --host 192.168.178.51 --channel 1
  node ina3221.js --host 192.168.178.51 --all --shunt-ohms 0.1
  CONTROLLER_IP=192.168.178.51 npm run read:all
`);
}

function parseArgs(argv) {
  const options = {
    host: process.env.CONTROLLER_IP || '',
    address: process.env.INA3221_ADDRESS || DEFAULT_ADDRESS,
    channel: Number(process.env.INA3221_CHANNEL || 1),
    all: false,
    scan: false,
    config: false,
    shuntOhms: process.env.INA3221_SHUNT_OHMS
      ? Number(process.env.INA3221_SHUNT_OHMS)
      : undefined,
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
      case '--channel':
        options.channel = Number(next());
        break;
      case '--all':
        options.all = true;
        break;
      case '--shunt-ohms':
        options.shuntOhms = Number(next());
        break;
      case '--config':
        options.config = true;
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

function assertValidOptions(options) {
  if (!options.host) {
    throw new Error('Missing --host <controller-ip> or CONTROLLER_IP');
  }

  if (!options.all && !Number.isInteger(options.channel)) {
    throw new Error('channel must be an integer between 1 and 3');
  }

  if (!options.all && !(options.channel in CHANNEL_REGISTERS)) {
    throw new Error('channel must be 1, 2, or 3');
  }

  if (options.shuntOhms !== undefined && (!Number.isFinite(options.shuntOhms) || options.shuntOhms <= 0)) {
    throw new Error('shunt-ohms must be a positive number');
  }
}

function signed16(msb, lsb) {
  let value = ((msb & 0xff) << 8) | (lsb & 0xff);
  if (value & 0x8000) {
    value -= 0x10000;
  }
  return value;
}

function unsigned16(msb, lsb) {
  return ((msb & 0xff) << 8) | (lsb & 0xff);
}

function toHex(value, width = 4) {
  return `0x${value.toString(16).toUpperCase().padStart(width, '0')}`;
}

function decodeShuntVoltage(bytes) {
  const rawRegister = unsigned16(bytes[0], bytes[1]);
  const signedRegister = signed16(bytes[0], bytes[1]);
  const code = signedRegister >> 3;
  const volts = code * SHUNT_LSB_VOLTS;

  return {
    rawRegister,
    code,
    volts,
    millivolts: volts * 1000,
  };
}

function decodeBusVoltage(bytes) {
  const rawRegister = unsigned16(bytes[0], bytes[1]);
  const code = rawRegister >> 3;
  const volts = code * BUS_LSB_VOLTS;

  return {
    rawRegister,
    code,
    volts,
  };
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

async function readRegister(host, address, register) {
  const result = await requestJson(host, '/api/i2c/write-read', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify({
      address,
      prefix: [register],
      length: 2,
    }),
  });

  if (!Array.isArray(result.bytes) || result.bytes.length !== 2) {
    throw new Error(`Expected two register bytes, got: ${JSON.stringify(result)}`);
  }

  return result;
}

async function readConfig(host, address) {
  const result = await readRegister(host, address, REG_CONFIG);
  const value = unsigned16(result.bytes[0], result.bytes[1]);
  console.log(`Config register ${toHex(REG_CONFIG, 2)}: ${toHex(value)} (${JSON.stringify(result.bytes)})`);
  return { result, value };
}

async function readChannel(host, address, channel, shuntOhms) {
  const registers = CHANNEL_REGISTERS[channel];
  if (!registers) {
    throw new Error('channel must be 1, 2, or 3');
  }

  const shuntResult = await readRegister(host, address, registers.shunt);
  const busResult = await readRegister(host, address, registers.bus);
  const shunt = decodeShuntVoltage(shuntResult.bytes);
  const bus = decodeBusVoltage(busResult.bytes);
  const currentAmps = shuntOhms ? shunt.volts / shuntOhms : undefined;

  return {
    channel,
    shuntRegister: toHex(registers.shunt, 2),
    busRegister: toHex(registers.bus, 2),
    raw: {
      shunt: toHex(shunt.rawRegister),
      bus: toHex(bus.rawRegister),
    },
    shuntMillivolts: Number(shunt.millivolts.toFixed(6)),
    busVolts: Number(bus.volts.toFixed(6)),
    currentAmps: currentAmps === undefined ? undefined : Number(currentAmps.toFixed(6)),
    currentMilliamps: currentAmps === undefined ? undefined : Number((currentAmps * 1000).toFixed(3)),
    rest: {
      shunt: shuntResult,
      bus: busResult,
    },
  };
}

function printMeasurements(measurements, shuntOhms) {
  const rows = measurements.map((measurement) => ({
    channel: measurement.channel,
    busVolts: measurement.busVolts,
    shuntMillivolts: measurement.shuntMillivolts,
    currentMilliamps: measurement.currentMilliamps ?? 'n/a',
    rawShunt: measurement.raw.shunt,
    rawBus: measurement.raw.bus,
  }));

  console.table(rows);
  if (!shuntOhms) {
    console.log('Current is n/a because --shunt-ohms was not provided.');
  }
  console.log(JSON.stringify({ measurements }, null, 2));
}

async function main() {
  const options = parseArgs(process.argv.slice(2));

  if (options.help) {
    printUsage();
    return;
  }

  assertValidOptions(options);

  if (options.scan) {
    await scanBus(options.host);
  }

  const shouldOnlyScan = options.scan
    && !options.config
    && !options.all
    && !process.argv.slice(2).some((arg) => arg === '--channel');
  if (shouldOnlyScan) {
    return;
  }

  if (options.config) {
    await readConfig(options.host, options.address);
  }

  const channels = options.all ? [1, 2, 3] : [options.channel];
  const measurements = [];
  for (const channel of channels) {
    measurements.push(await readChannel(options.host, options.address, channel, options.shuntOhms));
  }

  printMeasurements(measurements, options.shuntOhms);
}

main().catch((error) => {
  console.error(error.message);
  process.exitCode = 1;
});
