# Include folder

This directory contains header files used by the main application sources.

Headers in `include/` are meant to expose interfaces and shared declarations
for implementation files in `src/` and `lib/project/`.

## What belongs here

- `*.hpp`/`*.h` files for project classes such as `Controller`, `Sensor`,
  `Display`, and `ProtocolLayer`
- public type definitions and constants used across multiple source files
- declarations for classes whose definitions are implemented in `lib/project/`

## What does not belong here

- `.cpp` implementation files
- Arduino `setup()` / `loop()` entrypoints
- private helper code that is only used in a single translation unit

## This project layout

- `src/controller.cpp` — controller upload entrypoint
- `src/robot.cpp` — robot upload entrypoint
- `lib/project/` — shared implementation files
- `include/` — public header interfaces
