#!/usr/bin/env node
'use strict';

const DEFAULT_ADDRESS = '0x40';
const DEFAULT_SHUNT_OHMS = 0.1;
const DEFAULT_CURRENT_LSB_AMPS = 100e-6;

const REG_CONFIG = 0x00;
const REG_SHUNT_VOLTAGE = 0x01;
const REG_BUS_VOLTAGE = 0x02;
const REG_POWER = 0x03;
const REG_CURRENT = 0x04;
const REG_CALIBRATION = 0x05;

const SHUNT_LSB_VOLTS = 10e-6;
const BUS_LSB_VOLTS = 4e-3;

function printUsage() {
  console.log(`Usage:
  node ina219.js --host <controller-ip> --scan
  node ina219.js --host <controller-ip>
  node ina219.js --host <controller-ip> --calibrate
  node ina219.js --host <controller-ip> --calibrate --shunt-ohms 0.1 --current-lsb 0.0001

Options:
  --host <ip>             Controller IP or hostname. Can also use CONTROLLER_IP.
  --address <addr>        INA219 I2C address. Default: ${DEFAULT_ADDRESS}
  --shunt-ohms <ohms>     Shunt resistor value used for calibration. Default: ${DEFAULT_SHUNT_OHMS}
  --current-lsb <amps>    Current LSB used for calibration. Default: ${DEFAULT_CURRENT_LSB_AMPS}
  --calibrate             Write the calibration register before reading current/power.
  --config                Read and print config and calibration registers before measurements.
  --scan                  Scan the I2C bus before any read.
  --help                  Show this help.

Examples:
  node ina219.js --host 192.168.178.51 --scan
  node ina219.js --host 192.168.178.51
  node ina219.js --host 192.168.178.51 --calibrate --shunt-ohms 0.1 --current-lsb 0.0001
  CONTROLLER_IP=192.168.178.51 npm run read:calibrated
`);
}

function parseArgs(argv) {
  const options = {
    host: process.env.CONTROLLER_IP || '',
    address: process.env.INA219_ADDRESS || DEFAULT_ADDRESS,
    shuntOhms: Number(process.env.INA219_SHUNT_OHMS || DEFAULT_SHUNT_OHMS),
    currentLsbAmps: Number(process.env.INA219_CURRENT_LSB || DEFAULT_CURRENT_LSB_AMPS),
    scan: false,
    config: false,
    calibrate: false,
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
      case '--shunt-ohms':
        options.shuntOhms = Number(next());
        break;
      case '--current-lsb':
        options.currentLsbAmps = Number(next());
        break;
      case '--calibrate':
        options.calibrate = true;
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

  if (!Number.isFinite(options.shuntOhms) || options.shuntOhms <= 0) {
    throw new Error('shunt-ohms must be a positive number');
  }

  if (!Number.isFinite(options.currentLsbAmps) || options.currentLsbAmps <= 0) {
    throw new Error('current-lsb must be a positive number in amps');
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

function registerBytes(register, value) {
  return [
    register,
    (value >> 8) & 0xff,
    value & 0xff,
  ];
}

function calculateCalibration(shuntOhms, currentLsbAmps) {
  const calibration = Math.trunc(0.04096 / (currentLsbAmps * shuntOhms));
  if (calibration < 1 || calibration > 0xffff) {
    throw new Error(`calibration value out of range: ${calibration}`);
  }
  return calibration;
}

function decodeShuntVoltage(bytes) {
  const rawRegister = unsigned16(bytes[0], bytes[1]);
  const code = signed16(bytes[0], bytes[1]);
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

function decodeCurrent(bytes, currentLsbAmps) {
  const rawRegister = unsigned16(bytes[0], bytes[1]);
  const code = signed16(bytes[0], bytes[1]);
  const amps = code * currentLsbAmps;

  return {
    rawRegister,
    code,
    amps,
    milliamps: amps * 1000,
  };
}

function decodePower(bytes, currentLsbAmps) {
  const rawRegister = unsigned16(bytes[0], bytes[1]);
  const code = rawRegister;
  const watts = code * 20 * currentLsbAmps;

  return {
    rawRegister,
    code,
    watts,
    milliwatts: watts * 1000,
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

async function writeRegister(host, address, register, value) {
  const bytes = registerBytes(register, value);
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

async function readRegisterValue(host, address, register) {
  const result = await readRegister(host, address, register);
  return {
    result,
    value: unsigned16(result.bytes[0], result.bytes[1]),
  };
}

async function readConfigRegisters(host, address) {
  const config = await readRegisterValue(host, address, REG_CONFIG);
  const calibration = await readRegisterValue(host, address, REG_CALIBRATION);
  console.log(`Config register ${toHex(REG_CONFIG, 2)}: ${toHex(config.value)} (${JSON.stringify(config.result.bytes)})`);
  console.log(`Calibration register ${toHex(REG_CALIBRATION, 2)}: ${toHex(calibration.value)} (${JSON.stringify(calibration.result.bytes)})`);
  return { config, calibration };
}

async function readMeasurements(host, address, currentLsbAmps, includeCalibrated) {
  const shuntResult = await readRegister(host, address, REG_SHUNT_VOLTAGE);
  const busResult = await readRegister(host, address, REG_BUS_VOLTAGE);
  const shunt = decodeShuntVoltage(shuntResult.bytes);
  const bus = decodeBusVoltage(busResult.bytes);

  const measurement = {
    shuntRegister: toHex(REG_SHUNT_VOLTAGE, 2),
    busRegister: toHex(REG_BUS_VOLTAGE, 2),
    raw: {
      shunt: toHex(shunt.rawRegister),
      bus: toHex(bus.rawRegister),
    },
    shuntMillivolts: Number(shunt.millivolts.toFixed(6)),
    busVolts: Number(bus.volts.toFixed(6)),
    currentMilliamps: 'n/a',
    powerMilliwatts: 'n/a',
    rest: {
      shunt: shuntResult,
      bus: busResult,
    },
  };

  if (includeCalibrated) {
    const currentResult = await readRegister(host, address, REG_CURRENT);
    const powerResult = await readRegister(host, address, REG_POWER);
    const current = decodeCurrent(currentResult.bytes, currentLsbAmps);
    const power = decodePower(powerResult.bytes, currentLsbAmps);

    measurement.currentRegister = toHex(REG_CURRENT, 2);
    measurement.powerRegister = toHex(REG_POWER, 2);
    measurement.raw.current = toHex(current.rawRegister);
    measurement.raw.power = toHex(power.rawRegister);
    measurement.currentAmps = Number(current.amps.toFixed(6));
    measurement.currentMilliamps = Number(current.milliamps.toFixed(3));
    measurement.powerWatts = Number(power.watts.toFixed(6));
    measurement.powerMilliwatts = Number(power.milliwatts.toFixed(3));
    measurement.rest.current = currentResult;
    measurement.rest.power = powerResult;
  }

  return measurement;
}

function printMeasurements(measurement, calibration) {
  console.table([
    {
      busVolts: measurement.busVolts,
      shuntMillivolts: measurement.shuntMillivolts,
      currentMilliamps: measurement.currentMilliamps,
      powerMilliwatts: measurement.powerMilliwatts,
      rawShunt: measurement.raw.shunt,
      rawBus: measurement.raw.bus,
      rawCurrent: measurement.raw.current ?? 'n/a',
      rawPower: measurement.raw.power ?? 'n/a',
    },
  ]);

  if (!calibration) {
    console.log('Current and power are n/a because --calibrate was not provided.');
  } else {
    console.log(
      `Calibration ${toHex(calibration.value)} written with shunt ${calibration.shuntOhms} ohm and current LSB ${calibration.currentLsbAmps} A/bit.`,
    );
  }

  console.log(JSON.stringify({ calibration, measurement }, null, 2));
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
    && !options.calibrate
    && process.argv.slice(2).every((arg) => arg !== '--read');
  if (shouldOnlyScan) {
    return;
  }

  if (options.config) {
    await readConfigRegisters(options.host, options.address);
  }

  let calibration;
  if (options.calibrate) {
    const value = calculateCalibration(options.shuntOhms, options.currentLsbAmps);
    const write = await writeRegister(options.host, options.address, REG_CALIBRATION, value);
    calibration = {
      register: toHex(REG_CALIBRATION, 2),
      value,
      hex: toHex(value),
      shuntOhms: options.shuntOhms,
      currentLsbAmps: options.currentLsbAmps,
      write,
    };
  }

  const measurement = await readMeasurements(
    options.host,
    options.address,
    options.currentLsbAmps,
    Boolean(calibration),
  );
  printMeasurements(measurement, calibration);
}

main().catch((error) => {
  console.error(error.message);
  process.exitCode = 1;
});
