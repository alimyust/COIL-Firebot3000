

# COIL FireBot3000

This project is organized around two standalone uploadable applications in `src/`:

- `src/controller.cpp` — controller-side entrypoint
- `src/robot.cpp` — robot-side entrypoint

Shared implementation code has been moved into `lib/project/`.
Headers remain in `include/`.

## Build targets

PlatformIO environments are configured in `platformio.ini`:

- `env:controller` builds `src/controller.cpp`
- `env:robot` bui

## How to use

Any code written for specific hardware should be contained in `lib/project` as a .cpp file, and 
'include/' as a .h file. The source file can then be included in either `controller.cpp` or `robot.cpp`, 
which is where the lifecycle of the controller/robot will be present. 

## Folder layout

- `src/`
  - `controller.cpp`
  - `robot.cpp`
- `include/`
  - `Controller.hpp`
  - `Display.hpp`
  - `DisplaySensorProtocol.hpp`
  - `ProtocolLayer.hpp`
  - `Sensor.hpp`
- `lib/project/`
  - `Display.cpp`
  - `ProtocolLayer.cpp`
  - `Sensor.cpp`

## Notes

- Only `controller.cpp` and `robot.cpp` are upload targets.
- `lib/project/` contains support modules that are compiled as part of the project.
- `include/README.md` describes the include folder and header conventions.
