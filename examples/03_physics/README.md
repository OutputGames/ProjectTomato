# Physics Demo Example

This example demonstrates the physics simulation capabilities of the Tomato Engine using Bullet3.

## What it shows:

1. **Physics Bodies**: Creating dynamic and static rigid bodies
2. **Collision Detection**: Objects colliding and bouncing
3. **Gravity**: Realistic falling motion
4. **Mass Properties**: Different masses affect behavior
5. **Dynamic Spawning**: Creating physics objects at runtime

## Controls:

- **SPACE**: Manually spawn a new cube
- **R**: Reset scene (note: simplified for demo)
- **ESC**: Exit application

## How to run:

```bash
mkdir build && cd build
cmake ..
cmake --build .
./bin/PhysicsExample  # or PhysicsExample.exe on Windows
```

## Code highlights:

- `physics::PhysicsBody` creates a physics-enabled object
- `physics::ColliderInitInfo` defines collision shape (Box, Sphere, Capsule, Mesh)
- Mass = 0 creates a static body (doesn't move)
- Mass > 0 creates a dynamic body (affected by gravity and forces)
- Physics bodies automatically sync with their parent mesh objects

## Physics concepts demonstrated:

- **Static Bodies**: The ground plane doesn't move (mass = 0)
- **Dynamic Bodies**: Cubes fall and bounce (mass > 0)
- **Collision Shapes**: Box colliders used for cubes
- **Gravity**: Objects naturally fall down
- **Collision Response**: Objects bounce off each other and the ground

## Expected result:

You'll see cubes spawning above a ground plane, falling due to gravity, bouncing, and settling. The physics simulation runs in real-time with proper collision detection.

## Notes:

- The physics simulation runs at a fixed timestep
- Collision detection is handled automatically by Bullet3
- Physics bodies are synchronized with render objects each frame
