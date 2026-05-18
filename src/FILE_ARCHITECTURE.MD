

# FireBot3000 File Architecture

This project is split into 2 main parts: **Robot Code** and **Controller Code**.

## Robot Code

### Robot.cpp
The main handler for all logic on the robot side. It now wires the robot directly to the shared `tagtronics` libraries, handling radio packet reception through `RF69_Comm` and applying motor/steering PWM through `DualHardwarePWM` without extra robot-only wrapper classes.

### ProtocolLayer
The higher-level radio wrapper remains available for code that still benefits from it, such as the controller side. It builds on top of `RF69_Comm` for command-oriented messaging.

## Controller Code

**Status:** Work in Progress (WIP)
