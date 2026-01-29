# Tomato Engine

[![wakatime](https://wakatime.com/badge/user/c29784a7-7b2b-4770-8b4c-f0de1a86246c/project/ba004dac-e0e1-43cf-90ff-ac5a94091ec7.svg)](https://wakatime.com/badge/user/c29784a7-7b2b-4770-8b4c-f0de1a86246c/project/ba004dac-e0e1-43cf-90ff-ac5a94091ec7)

A 3D game engine built in C++ with support for both 2D and 3D rendering, physics simulation, audio, and cross-platform input handling.

## Quick Start

### Building with CMake (Recommended)

```bash
# Clone the repository
git clone https://github.com/OutputGames/ProjectTomato.git
cd ProjectTomato

# Create build directory
mkdir build && cd build

# Configure (automatically builds dependencies)
cmake ..

# Build
cmake --build .

# Run examples
./bin/BasicExample  # or BasicExample.exe on Windows
```

### Building with Premake5

```bash
# Generate project files
premake5 vs2022  # or vs2019, gmake2, etc.

# Open generated solution and build
```

## Examples

The `examples/` directory contains several example applications demonstrating engine features:

- **01_basic**: Basic engine initialization and 3D rendering
- **02_input**: Keyboard, mouse, and gamepad input handling
- **03_physics**: Physics simulation with Bullet3

See [examples/README.md](examples/README.md) for details.

## Architecture Overview

The Tomato Engine is organized into modular subsystems, each handling a specific aspect of game functionality:

### Core Systems

- **Engine** (`include/tomato/Engine/`) - Core initialization, update loop, and application management
  - Initializes all subsystems in the correct order
  - Manages the main game loop and frame timing
  - Handles clean shutdown and resource cleanup

- **Globals** (`include/tomato/globals.cpp/hpp`) - Global state and cross-system utilities
  - Shared variables accessed by multiple subsystems
  - Type conversion functions between different libraries (GLM, Bullet3, Assimp)
  - Mesh loading and geometric utilities

### Graphics and Rendering

- **Render** (`include/tomato/Render/`) - Graphics rendering system
  - Shader management and compilation
  - Texture loading and management
  - Draw call batching and execution
  - Camera system

- **Light** (`include/tomato/Light/`) - Lighting system
  - Point lights, directional lights, spot lights
  - Shadow mapping
  - Light uniform management

- **2D** (`include/tomato/2D/`) - 2D rendering utilities
  - Sprite rendering
  - 2D transforms and cameras

- **UI** (`include/tomato/Ui/`) - User interface rendering
  - ImGui integration for debug UI
  - In-game UI rendering

### Physics and Collision

- **Physics** (`include/tomato/Physics/`) - Physics simulation using Bullet3
  - Rigid body dynamics
  - Collision detection and response
  - Ray casting
  - Support for box, sphere, capsule, and mesh colliders

- **Particle** (`include/tomato/Particle/`) - Particle system
  - Particle emitters and simulation
  - Particle physics and collision

### Input and Interaction

- **Input** (`include/tomato/Input/`) - Cross-platform input handling
  - Mouse input (position, buttons, scroll)
  - Keyboard input (press, hold, release states)
  - Gamepad support (buttons and analog sticks)
  - Virtual axis system for consistent input across devices
  - Automatic switching between keyboard/mouse and gamepad

### Utilities and Math

- **Math** (`include/tomato/Math/`) - Mathematical utilities
  - Vector and matrix conversions between libraries
  - Quaternion operations (look rotation, from-to rotation)
  - Interpolation functions (lerp, slerp)
  - Geometric calculations

- **Time** (`include/tomato/Time/`) - Time and frame tracking
  - Delta time for frame-rate independent movement
  - Current time and frame counters
  - Trigonometric time functions for animations

- **Utils** (`include/tomato/utils.hpp`) - Common utilities and macros
  - Type definitions (u8, u16, s32, etc.)
  - Common macros (`var`, `randval`, etc.)
  - Random number generation
  - String utilities

### Asset Management

- **Fs** (`include/tomato/Fs/`) - File system and resource management
  - Loading textures, models, and audio files
  - Resource caching
  - Asset path management

### Audio

- **Audio** (`include/tomato/Audio/`) - Sound playback and management
  - 2D and 3D audio
  - Sound effect playback
  - Music streaming

### Other Systems

- **Debug** (`include/tomato/Debug/`) - Debugging utilities
  - Debug drawing (lines, spheres, boxes)
  - Debug UI (ImGui integration)
  - Performance profiling

- **Net** (`include/tomato/Net/`) - Network utilities
  - Network communication
  - Multiplayer support

- **Ai** (`include/tomato/Ai/`) - AI utilities (currently minimal)

- **Prim** (`include/tomato/Prim/`) - Primitive mesh generation
  - Procedural generation of basic shapes (cubes, spheres, etc.)

## Key Design Patterns

### Initialization Order
The engine initializes subsystems in a specific order to handle dependencies:
1. Resource Manager
2. Rendering System (creates window and graphics context)
3. Input System
4. Audio System
5. Object/Scene Management

### Update Loop
Each frame, subsystems are updated in this order:
1. Debug UI (if in debug mode)
2. Mouse position tracking
3. Object/scene updates (game logic, transforms)
4. Rendering (draw calls, shader updates)
5. Input state updates (key/button press detection)
6. Audio updates (3D positioning, streaming)
7. Delta time calculation

### Global State
Many core systems use global variables (defined in `globals.cpp/hpp`) for:
- Performance (avoid pointer indirection)
- Convenience (easy access from anywhere)
- Shared state (renderer info, input state, physics world)

### Type Conversions
The engine bridges multiple libraries (GLM for math, Bullet3 for physics, Assimp for loading).
Conversion functions in `globals.cpp` and `math.cpp` handle translation between these types.

## Building

The engine uses premake5 for build configuration:

```bash
premake5.exe vs2022  # Generate Visual Studio 2022 solution
```

Then open the generated solution and build.

## Usage Example

```cpp
// Create an application
tmt::engine::Application app("My Game", 1920, 1080, false);

// Main game loop
while (!glfwWindowShouldClose(renderer->window))
{
    glfwPollEvents();
    
    // Your game logic here
    
    tmt::engine::update();  // Update all engine systems
}

// Clean shutdown
tmt::engine::shutdown();
```

## Dependencies

- **GLFW** - Window and input management
- **GLM** - Mathematics library
- **Bullet3** - Physics simulation
- **Assimp** - 3D model loading
- **stb_image** - Image loading
- **par_shapes** - Procedural mesh generation
- **ImGui** - Debug UI
- **bgfx** - Graphics rendering backend
- **FreeType** - Font rendering
- **Box2D** - 2D physics (optional)
- **ENet** - Networking (optional)

Most dependencies are included in the `vendor/` directory and will be built automatically with CMake.

## Building

### CMake Build (Cross-Platform)

CMake is the recommended build system as it works across Windows, Linux, and macOS:

```bash
# Clone repository with submodules
git clone --recursive https://github.com/OutputGames/ProjectTomato.git
cd ProjectTomato

# Create build directory
mkdir build && cd build

# Configure (use -G to specify generator)
cmake ..                          # Default generator
cmake -G "Visual Studio 17 2022" ..  # VS 2022
cmake -G "Unix Makefiles" ..      # Makefiles

# Build
cmake --build .                   # Default configuration
cmake --build . --config Release  # Release build

# Install (optional)
cmake --install . --prefix /path/to/install

# Build options
cmake -DTOMATO_BUILD_EXAMPLES=ON ..   # Build examples (default: ON)
cmake -DTOMATO_BUILD_SHARED=ON ..     # Build shared library (default: OFF)
```

### Premake5 Build (Windows)

Premake5 is also supported for Visual Studio on Windows:

```bash
# Generate Visual Studio solution
premake5.exe vs2022  # or vs2019, vs2017

# Open generated .sln file in Visual Studio and build
```

## Usage Example

```cpp
#include "tomato/tomato.hpp"
#include "tomato/globals.hpp"

using namespace tmt;

int main()
{
    // Create application window
    var app = new engine::Application("My Game", 1920, 1080, false);

    // Get main camera
    var cam = obj::CameraObject::GetMainCamera();
    cam->position = glm::vec3(5, 5, 5);
    cam->LookAt(glm::vec3(0, 0, 0));

    // Create a cube
    var cube = obj::MeshObject::FromPrimitive(prim::Cube);
    
    // Main game loop
    while (!glfwWindowShouldClose(renderer->window))
    {
        glfwPollEvents();
        
        // Game logic
        cube->rotation.y += time::getDeltaTime() * 50.0f;
        
        engine::update();  // Update all systems
    }
    
    // Cleanup
    engine::shutdown();
    return 0;
}
```

## Projects Using Tomato Engine

- [SplatoonTomato](https://github.com/OutputGames/SplatoonTomato) - Splatoon-inspired game
- [TomodachiRecreation](https://github.com/OutputGames/TomodachiRecreation) - Tomodachi Life recreation

## Dependencies

- **GLFW** - Window and input management
- **GLM** - Mathematics library
- **Bullet3** - Physics simulation
- **Assimp** - 3D model loading
- **stb_image** - Image loading
- **par_shapes** - Procedural mesh generation
- **ImGui** - Debug UI
- **bgfx** - Graphics rendering backend

## Notes for Future Development

When returning to this project after time away, key areas to understand:

1. **Start with `engine.cpp`** - Understand initialization and update order
2. **Check `globals.hpp`** - See what global state exists
3. **Review `input.cpp`** - Understand how player input is processed
4. **Look at `math.cpp`** - Review type conversions between libraries
5. **Examine physics integration** - See how Bullet3 integrates with the engine in `globals.cpp` and `physics.cpp`

The engine follows a straightforward pattern: initialize systems, run update loop, clean up. Each subsystem is relatively self-contained, making it easier to modify individual components without affecting others.
