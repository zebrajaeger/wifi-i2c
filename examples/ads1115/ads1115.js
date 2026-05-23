#!/usr/bin/env node
'use strict';

const DEFAULT_ADDRESS = '0x48';
const DEFAULT_CHANNEL = 0;
const DEFAULT_GAIN = 4.096;
const DEFAULT_SPS = 128;
const DEFAULT_CONVERSION_WAIT_MS = 10;

const ADS1115_REG_CONVERSION = 0x00;
const ADS1115_REG_CONFIG = 0x01;

const SINGLE_ENDED_MUX_BITS = {
  0: 0x4000,
  1: 0x5000,
  2: 0x6000,
  3: 0x7000,
};

const PGA_BITS_BY_GAIN = {
  6.144: 0x0000,
  4.096: 0x0200,
  2.048: 0x0400,
  1.024: 0x0600,
  0.512: 0x0800,
  0.256: 0x0a00,
};

const DATA_RATE_BITS_BY_SPS = {
  8: 0x0000,
  16: 0x0020,
  32: 0x0040,
  64: 0x0060,
  128: 0x0080,
  250: 0x00a0,
  475: 0x00c0,
  860: 0x00e0,
};

function printUsage() {
  console.log(`Usage:
  node ads1115.js --host <controller-ip> --scan
  node ads1115.js --host <controller-ip> --channel <0..3>
  node ads1115.js --host <controller-ip> --scan --channel 0 --gain 4.096

Options:
  --host <ip>       Controller IP or hostname. Can also use CONTROLLER_IP.
  --address <addr>  ADS1115 I2C address. Default: ${DEFAULT_ADDRESS}
  --channel <n>     Single-ended channel 0..3. Default: ${DEFAULT_CHANNEL}
  --gain <volts>    Full-scale range: 6.144, 4.096, 2.048, 1.024, 0.512, 0.256. Default: ${DEFAULT_GAIN}
  --sps <rate>      Data rate: 8, 16, 32, 64, 128, 250, 475, 860. Default: ${DEFAULT_SPS}
  --wait-ms <ms>    Delay after config write before reading. Default: ${DEFAULT_CONVERSION_WAIT_MS}
  --scan            Scan the I2C bus before any read.
  --help            Show this help.

Examples:
  node ads1115.js --host 192.168.178.51 --scan
  node ads1115.js --host 192.168.178.51 --channel 0
  CONTROLLER_IP=192.168.178.51 npm run read:ain0
`);
}

function parseArgs(argv) {
  const options = {
    host: process.env.CONTROLLER_IP || '',
    address: process.env.ADS1115_ADDRESS || DEFAULT_ADDRESS,
    channel: Number(process.env.ADS1115_CHANNEL || DEFAULT_CHANNEL),
    gain: Number(process.env.ADS1115_GAIN || DEFAULT_GAIN),
    sps: Number(process.env.ADS1115_SPS || DEFAULT_SPS),
    waitMs: Number(process.env.ADS1115_WAIT_MS || DEFAULT_CONVERSION_WAIT_MS),
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
      case '--channel':
        options.channel = Number(next());
        break;
      case '--gain':
        options.gain = Number(next());
        break;
      case '--sps':
        options.sps = Number(next());
        break;
      case '--wait-ms':
        options.waitMs = Number(next());
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

function buildConfigWord({ channel, gain, sps }) {
  if (!Number.isInteger(channel) || !(channel in SINGLE_ENDED_MUX_BITS)) {
    throw new Error('channel must be an integer between 0 and 3');
  }

  if (!(gain in PGA_BITS_BY_GAIN)) {
    throw new Error(`gain must be one of: ${Object.keys(PGA_BITS_BY_GAIN).join(', ')}`);
  }

  if (!(sps in DATA_RATE_BITS_BY_SPS)) {
    throw new Error(`sps must be one of: ${Object.keys(DATA_RATE_BITS_BY_SPS).join(', ')}`);
  }

  return (
    0x8000 |
    SINGLE_ENDED_MUX_BITS[channel] |
    PGA_BITS_BY_GAIN[gain] |
    0x0100 |
    DATA_RATE_BITS_BY_SPS[sps] |
    0x0003
  );
}

function configBytes(configWord) {
  return [
    ADS1115_REG_CONFIG,
    (configWord >> 8) & 0xff,
    configWord & 0xff,
  ];
}

function signed16(msb, lsb) {
  let value = ((msb & 0xff) << 8) | (lsb & 0xff);
  if (value & 0x8000) {
    value -= 0x10000;
  }
  return value;
}

function rawToVolts(raw, gain) {
  return (raw * gain) / 32768;
}

function delay(ms) {
  return new Promise((resolve) => setTimeout(resolve, ms));
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

async function writeConfig(host, address, bytes) {
  return requestJson(host, '/api/i2c/write', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify({
      address,
      bytes,
    }),
  });
}

async function readConversion(host, address) {
  return requestJson(host, '/api/i2c/write-read', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json',
    },
    body: JSON.stringify({
      address,
      prefix: [ADS1115_REG_CONVERSION],
      length: 2,
    }),
  });
}

async function readSingleShot(host, options) {
  const configWord = buildConfigWord(options);
  const bytes = configBytes(configWord);

  console.log(
    `Configuring ADS1115 ${options.address} AIN${options.channel}, gain +/-${options.gain} V, ${options.sps} SPS`,
  );
  console.log(`Config word: 0x${configWord.toString(16).toUpperCase()}, bytes [${bytes.join(', ')}]`);

  await writeConfig(host, options.address, bytes);
  await delay(options.waitMs);

  const result = await readConversion(host, options.address);
  if (!Array.isArray(result.bytes) || result.bytes.length !== 2) {
    throw new Error(`Expected two conversion bytes, got: ${JSON.stringify(result)}`);
  }

  const raw = signed16(result.bytes[0], result.bytes[1]);
  const volts = rawToVolts(raw, options.gain);

  console.log(JSON.stringify(result, null, 2));
  console.log(`Raw signed value: ${raw}`);
  console.log(`Approximate voltage: ${volts.toFixed(6)} V`);
  return { result, raw, volts };
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

  if (!options.scan) {
    await readSingleShot(options.host, options);
    return;
  }

  await scanBus(options.host);
  const shouldOnlyScan = !process.argv.slice(2).some((arg) => arg === '--channel');
  if (!shouldOnlyScan) {
    await readSingleShot(options.host, options);
  }
}

main().catch((error) => {
  console.error(error.message);
  process.exitCode = 1;
});
