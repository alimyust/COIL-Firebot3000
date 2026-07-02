

# COIL FireBot3000

This project is built around two main PlatformIO upload targets in `src/`:

- `src/controller.cpp` — controller-side entrypoint
- `src/robot.cpp` — robot-side entrypoint

The rest of the project is organized into library modules under `lib/`, with each folder containing a specific subsystem.

## Build targets

PlatformIO environments are configured in `platformio.ini`:

- `env:controller` builds `src/controller.cpp`
- `env:robot` builds `src/robot.cpp`

## How To Build

To upload the robot code (`src/robot.cpp`) to the robot side featherboard:

- `pio run -e robot -t upload`

To upload the controller code (`src/controller.cpp`) to the controller side featherboard:

- `pio run -e controller -t upload`

To run any of the tests, they must be in the `test/` folder with the prefix `test_[...]/`.

- `pio test -e [robot/controller] -f [test_name]`

For example, to run the quad_pwm test on the robot environment...

- `pio test -e robot -f test_motor_quad`

## Current project structure

- `src/`
  - `controller.cpp`
  - `robot.cpp`

- `lib/`
  - `radio/`
    - `radio.cpp`
    - `radio.h`
  - `ProtocolLayer/`
    - `ProtocolLayer.cpp`
    - `ProtocolLayer.hpp`
  - `ControllerHandler/`
    - `ControllerHandler.cpp`
    - `ControllerHandler.hpp`
  - `RobotHandler/`
    - `RobotHandler.cpp`
    - `RobotHandler.hpp`
  - `joystick/`
    - `Joystick.cpp`
    - `Joystick.hpp`
  - `motor/`
    - `DualHWPwm.cpp`
    - `DualHWPwm.hpp`
    - `QuadHWPwm.cpp`
    - `QuadHWPwm.hpp`
    - `MotorDriver.h`
  - `DebugLog/`
    - `DebugLog.cpp`
    - `DebugLog.hpp`
  - `tagtronics/`
    - `RFComm.cpp`
    - `RFComm.hpp`
    - `TripleHWPwm.cpp`
    - `TripleHWPwm.hpp`

- `test/`
  - unit and integration test support directories

- root sketches and examples:
  - `8pwm_receiver.ino`
  - `Sensiron_receiver.ino`
  - `Sensiron_Transmitter.ino`
  - `Turret_empfangen_2.0/`
  - `Turret_senden/`

## How to use

Hardware-specific implementations should be kept inside the appropriate `lib/` subsystem folder and exposed through the handler interfaces used by `controller.cpp` and `robot.cpp`.

## Radio architecture

The wireless stack is layered into three responsibilities:

1. **Radio layer** (`lib/radio/`)
   - Provides low-level RF69 packet send/receive functionality.
   - Handles node addressing, frequency setup, encryption key configuration, and RSSI reporting.
   - Contains two main classes:
     - `RF69_Comm` — direct packet transmit/receive with callback support.
     - `EventRadioComm` — event-driven wrapper that enqueues received packets and telemetry tick events for later processing.
   - Purpose: keep the physical radio interface separate from message semantics.

2. **Protocol layer** (`lib/ProtocolLayer/`)
   - Builds on `EventRadioComm` to define protocol commands and message handling.
   - Defines command IDs such as `THROTTLE`, `STEERING_DUTY`, `BATTERY`, and `HEARTBEAT`.
   - Exposes outgoing methods like `sendThrottle()`, `sendSteering()`, `sendBatteryLevel()`, and `sendHeartbeat()`.
   - Processes events from `EventRadioComm` and routes packets to a registered `ProtocolHandler`.
   - Purpose: translate raw packets into application-level commands and telemetry, decoupling communication details from control logic.

3. **Handlers**
   - Implement `ProtocolLayer::ProtocolHandler` to receive protocol events and react accordingly.
   - `ControllerHandler`:
     - Runs on the controller side.
     - Reads joystick inputs, sends throttle and steering commands to the robot.
     - Receives battery level updates and heartbeat/timing events.
   - `RobotHandler`:
     - Runs on the robot side.
     - Receives throttle and steering commands from the controller.
     - Drives the motor subsystem and logs incoming messages.
   - Purpose: contain application-specific behavior for each node, while the protocol layer stays generic.

## Notes

- `src/controller.cpp` and `src/robot.cpp` are the only upload targets for the main system.
- The radio/protocol/handler split is intentionally layered so the low-level RF transport, protocol semantics, and application actions remain separate.
