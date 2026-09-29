# UnityDataDrivenActuators (Simple Baseline)

This is the recommended beginner baseline for Unity -> Arduino serial control.

## Data Protocol
Unity sends one line at a fixed interval:

CTRL,<servoAngle>,<lightState>

Examples:
- `CTRL,90,0` -> servo 90 deg, LED off
- `CTRL,135,1` -> servo 135 deg, LED on

Rules:
- Newline terminated (`\n`)
- `servoAngle` is clamped to 0-180 on Arduino
- `lightState` is treated as `0` (off) or non-zero (on)
- Unity sends `PING` after opening the port, and Arduino responds `READY`
- Arduino replies `OK` for each valid control command

## Unity Side
Use script: `Assets/Scripts/SendToArduinoScript.cs`

Assign in Inspector:
- `autoSelectPort` (recommended: on for class machines)
- `portName` and `baudRate` (used directly when `autoSelectPort` is off)
- `dataSource` (GameObject to read X position)
- `lightSource` (Light to read on/off state)
- `xMin` and `xMax` (scene X range mapped to servo 0-180)

## Arduino Side
Upload:
- `Firmware/UnityDataDrivenActuators/UnityDataDrivenActuators.ino`

Hardware defaults:
- Servo: pin 9
- LED: pin 8
- Serial: 115200 baud

On startup, the sketch runs a quick self-test (LED on, servo sweep 0 -> 180 -> 90).
If this self-test does not run, debug wiring/power before debugging Unity serial.

## Why this version is robust
- Integer-only payload (no float parsing on Arduino)
- One command format only
- Unity auto-reconnects if serial write/open fails
- Arduino ignores malformed lines safely
