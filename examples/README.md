# Tomato Engine Examples

This directory contains example applications demonstrating various features of the Tomato Engine.

## Building the Examples

All examples use CMake for building:

```bash
# From the project root
mkdir build && cd build
cmake ..
cmake --build .

# Run examples (from build directory)
./bin/BasicExample       # or BasicExample.exe on Windows
./bin/InputExample
./bin/PhysicsExample
```

## Example Overview

### 01_basic - Basic Application
**Difficulty**: Beginner  
**Topics**: Initialization, Rendering, Main Loop

The simplest example showing how to:
- Initialize the engine
- Create a window
- Render a 3D object (spinning cube)
- Implement the main game loop
- Shut down cleanly

**Perfect starting point for new users!**

### 02_input - Input Demo
**Difficulty**: Beginner  
**Topics**: Keyboard, Mouse, Gamepad, Camera Control

Demonstrates the input system:
- Keyboard input for movement
- Mouse clicks and dragging
- Mouse scroll for zooming
- Camera control and manipulation
- Cross-platform input handling

**Learn how to make your game interactive!**

### 03_physics - Physics Demo
**Difficulty**: Intermediate  
**Topics**: Rigid Bodies, Collision, Physics Simulation

Shows physics simulation capabilities:
- Creating physics bodies
- Static vs dynamic objects
- Gravity and collision
- Spawning objects at runtime
- Physics-render synchronization

**See realistic physics in action!**

## Learning Path

1. Start with **01_basic** to understand engine fundamentals
2. Move to **02_input** to learn user interaction
3. Try **03_physics** to add realistic simulation

## Example Code Pattern

All examples follow this pattern:

```cpp
#include "tomato/tomato.hpp"
#include "tomato/globals.hpp"

using namespace tmt;

int main(int argc, const char** argv)
{
    // 1. Initialize
    var app = new engine::Application("Title", width, height, is2D);
    
    // 2. Setup scene
    // Create objects, cameras, etc.
    
    // 3. Main loop
    while (!glfwWindowShouldClose(renderer->window))
    {
        glfwPollEvents();
        
        // Your game logic here
        
        engine::update();  // Update all systems
    }
    
    // 4. Cleanup
    engine::shutdown();
    
    return 0;
}
```

## Next Steps

After trying these examples, you can:
- Combine concepts from multiple examples
- Check out real projects:
  - [SplatoonTomato](https://github.com/OutputGames/SplatoonTomato)
  - [TomodachiRecreation](https://github.com/OutputGames/TomodachiRecreation)
- Read the engine documentation in the main README
- Explore the engine source code in `include/tomato/`

## Troubleshooting

**Build fails**: Make sure all dependencies are properly installed (see main README)  
**Window doesn't open**: Check graphics drivers are up to date  
**Physics acts weird**: Ensure proper mass values (0 = static, >0 = dynamic)  

## Contributing

Have an idea for a new example? Feel free to contribute! Good examples to add:
- Audio playback demo
- 2D rendering example
- Network multiplayer demo
- Particle effects showcase
- UI and ImGui integration
