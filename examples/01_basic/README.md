# Basic Example

This example demonstrates the most basic usage of the Tomato Engine.

## What it shows:

1. **Engine Initialization**: How to create an application and initialize the engine
2. **Window Creation**: Setting up a window with specified dimensions
3. **Camera Setup**: Positioning and orienting the main camera
4. **3D Rendering**: Creating and rendering a simple 3D cube
5. **Animation**: Rotating the cube over time
6. **Main Loop**: The basic game loop pattern
7. **Shutdown**: Proper cleanup when exiting

## How to run:

```bash
mkdir build && cd build
cmake ..
cmake --build .
./bin/BasicExample  # or BasicExample.exe on Windows
```

## Code walkthrough:

- `engine::Application` creates the window and initializes all engine subsystems
- `obj::CameraObject::GetMainCamera()` retrieves the default camera
- `obj::MeshObject::FromPrimitive()` creates a mesh from a primitive shape
- `engine::update()` updates all subsystems (input, physics, rendering, etc.)
- `engine::shutdown()` cleanly shuts down the engine

## Expected result:

You should see a window with a rotating colored cube in the center.
