### 9.11.2026
Created an initial project structure and made a simple program compile. The project now uses CMake as its build system.
Implemented a simple websocket communication prototype. Chose libraries to use. Can be tested by building with CMake.
Instructions for testing:
```
cmake -B build
cmake --build build
IN TERMINAL 1:
./build/SensorDisSys 5000
IN TERMINAL 2:
./build/SensorDisSys 5001 ws://localhost:5000
```