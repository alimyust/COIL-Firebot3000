

# COIL FireBot3000

This project is a joint collaboration between Hochschule Ruhr West and Wayne State University as a part of the COIL project. A robot and controller module was constructed to be used by firefighters to explore a scene before going in themselves. The project utilizes the Featherboard M0 RFM69HCW module to communicate between the robot and the controller. 

This project utilizes an event-driven communication flow which describes what each side should be doing at any time. The radio stack and application logic are connected by an `EventScheduler`, which routes incoming packets and periodic tasks to the relevant handlers and drivers.

## Code Architecture

The active flow is:

- `RadioComm` handles the low-level RF packet transport.
- `EventScheduler` listens for incoming radio packets and periodic tasks, then dispatches them by command ID and priority.
- `ControllerHandler` and `RobotHandler` implement the node-specific behavior, describing the contract between the radio and the robot/controller.
- Driver libraries in `lib/` provide the actual hardware interfaces (motors, sensors, display, radio, audio, etc.) which are decoupled and usable by themselves.

## Build targets

PlatformIO environments are configured in `platformio.ini`:

- `env:controller` builds `src/controller.cpp`
- `env:robot` builds `src/robot.cpp`

## How to build

Upload the robot firmware:

- `pio run -e robot -t upload`

Upload the controller firmware:

- `pio run -e controller -t upload`

Run a test in the `test/` folder:

- `pio test -e [robot|controller] -f [test_name]`

Example:

- `pio test -e robot -f test_motor_quad`

## Current project structure

- `include/`
  - Shared public definitions for the project, including `ProtocolCommands.h` for radio command IDs and payload layouts.

- `src/`
  - Main firmware entry points: `controller.cpp` and `robot.cpp`.
  - Scheduler and handler logic under `handlers/`, including `scheduler.h`, `ControllerHandler.*`, and `RobotHandler.*`.

- `lib/`
  - Hardware and subsystem implementations:
    - `audio/` — microphone and speaker drivers.
    - `co_sensor/` — CO sensor interface.
    - `display/` — OLED display driver.
    - `joystick/` — joystick input handling.
    - `motor/` — motor driver and PWM support.
    - `radio/` — RF communications layer.
    - `Sensor/` — environmental sensor support.
    - `tagtronics/` — RF and turret code that was used for COIL 2025
    - `turret/` — turret mapping definitions.

- `test/`
  - PlatformIO test targets for audio, schedulers, handler payloads, and motor behavior.

- `arduino_code/`
  - Legacy Arduino sketches and reference code.

- `platformio.ini`
  - PlatformIO configuration for the controller and robot targets.

- `README.md`
  - Project overview and build instructions.

## Runtime flow

The current runtime pattern is:

1. `radio.update()` receives packets from the RF module and enqueues them as system events.
2. `scheduler.update()` evaluates system tasks continously until the queue is empty by dispatching callbacks.
3. Matching handlers receive the command payload/callback and execute the relevant logic.
4. Handlers then call driver code or send outbound packets back through the scheduler.

This keeps the behavior centralized and event-driven while still separating low-level hardware code from the control logic.
