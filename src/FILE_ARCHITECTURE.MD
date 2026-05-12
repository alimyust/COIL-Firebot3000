

# FireBot3000 File Architecture

This project is split into 2 main parts: **Robot Code** and **Controller Code**.

## Robot Code

### Robot.cpp
The main handler for all logic on the robot side. This is the decision-making file where the core robot behavior is orchestrated. It serves as the central hub that coordinates between the CommandHandler and ProtocolLayer.

### CommandHandler
A class that encapsulates varying robot behavior from multiple subsystems. This abstraction layer provides a clean interface for executing commands across different hardware components.

**Example Commands:**
- `cmd_drive_motor()` - Drive main motor
- `cmd_steer_motor()` - Control steering mechanism
- `cmd_steer_laser()` - Adjust laser direction
- `cmd_laser()` - Fire laser
- `play_speaker()` - Audio output
- `flash_led()` - LED control

### ProtocolLayer
A class that encapsulates all communication logic between the controller and the robot. It acts as a wrapper around RFComm.cpp with enhanced features specific to this use case, including potential audio/video transmission capabilities.

**Key Functions:**
- `send_msg()` - Transmit messages to controller
- `receive_msg()` - Receive commands from controller
- Audio/video transmission support (planned)

## Controller Code

**Status:** Work in Progress (WIP)