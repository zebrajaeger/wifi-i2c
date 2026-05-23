---
name: platformio-device-workflow
description: use when working in a platformio project and the user wants to compile firmware, upload firmware to a connected device, or open the serial monitor. trigger for platformio, pio run, pio upload, pio monitor, embedded firmware build/upload/monitor workflows, and troubleshooting those commands. variables such as environments, upload port, monitor port, baud rate, and flags must be read from platformio.ini instead of guessed.
---

# PlatformIO Device Workflow

## Core rule

Always read `platformio.ini` before running PlatformIO commands.

Use values from `platformio.ini` for:
- environments
- `default_envs`
- `monitor_speed`
- `monitor_port`
- `upload_port`
- build or upload settings

Do not invent board, port, baud rate, or environment values.

## Supported commands

Only use these PlatformIO commands unless the user explicitly asks for something else:

```bash
pio run
pio run -t upload
pio device monitor
````

## Compile firmware

To compile the project:

```bash
pio run
```

If `platformio.ini` defines `default_envs`, rely on it.

If multiple environments exist and no `default_envs` is defined, ask which environment to use instead of guessing.

## Upload firmware

Before uploading, compile first:

```bash
pio run
```

Then upload:

```bash
pio run -t upload
```

Use `upload_port` from `platformio.ini` when present.

Do not change upload ports or board settings unless the user asks.

## Open serial monitor

To open the monitor:

```bash
pio device monitor
```

Use `monitor_speed` and `monitor_port` from `platformio.ini` when present.

If monitor settings are missing, run the plain command and let PlatformIO defaults apply.

## Error handling

When a command fails:

1. quote the relevant error lines
2. explain the likely cause briefly
3. suggest the smallest next change
4. avoid editing `platformio.ini` unless the fix is clear or the user asks

## Workflow

For “compile”:

1. read `platformio.ini`
2. run `pio run`
3. summarize result

For “upload”:

1. read `platformio.ini`
2. run `pio run`
3. if compile succeeds, run `pio run -t upload`
4. summarize result

For “monitor”:

1. read `platformio.ini`
2. run `pio device monitor`
3. report connection/output issues if any
