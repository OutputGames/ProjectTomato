# Input Demo Example

This example demonstrates various input methods supported by the Tomato Engine.

## What it shows:

1. **Keyboard Input**: Using WASD/Arrow keys for movement
2. **Mouse Input**: 
   - Click to interact
   - Mouse movement for camera control
   - Scroll wheel for zooming
3. **Virtual Axes**: Using the engine's cross-platform input system
4. **Camera Control**: Interactive camera manipulation
5. **Input States**: Detecting press, hold, and release states

## Controls:

- **WASD / Arrow Keys**: Move the cube around
- **Right Mouse Button + Drag**: Rotate camera
- **Left Click**: Change cube color randomly
- **Mouse Scroll**: Zoom camera in/out
- **Space**: Reset camera position
- **ESC**: Exit application

## How to run:

```bash
mkdir build && cd build
cmake ..
cmake --build .
./bin/InputExample  # or InputExample.exe on Windows
```

## Code highlights:

- `input::GetAxis()` provides cross-platform input (works with keyboard and gamepad)
- `input::Mouse::GetMouseButton()` detects mouse clicks with Press/Hold/Release states
- `input::Mouse::GetMouseDelta()` tracks mouse movement
- `input::Mouse::GetMouseScroll()` handles scroll wheel input
- Camera orbiting implemented using spherical coordinates

## Expected result:

An interactive 3D scene where you can:
- Move a colored cube with keyboard
- Rotate the camera around the cube
- Click to change colors
- Zoom in and out
