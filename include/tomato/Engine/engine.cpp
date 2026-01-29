/**
 * @file engine.cpp
 * @brief Implementation of core engine initialization and update functionality
 */

#include "engine.hpp"
#include "globals.hpp"

/**
 * @brief Initialize the game engine and all subsystems
 * 
 * This function performs the following initialization steps in order:
 * 1. Sets the global application pointer
 * 2. Creates the resource manager for asset loading
 * 3. Initializes the rendering system with the specified window size
 * 4. Sets up input handling (keyboard, mouse, gamepad)
 * 5. Creates and populates the engine info structure
 * 6. Initializes audio system
 * 7. Initializes object/scene management system
 * 
 * @param app Pointer to the Application instance
 * @param ws Window size as a 2D vector (width, height)
 * @return EngineInfo* Pointer to the initialized engine information structure containing
 *         references to the renderer and application
 */
tmt::engine::EngineInfo* tmt::engine::init(Application* app, glm::vec2 ws)
{
    // Store the application instance globally for access across subsystems
    application = app;
    
    // Create resource manager for loading textures, models, audio, etc.
    var resourceManager = new fs::ResourceManager();

    // Initialize the rendering system (OpenGL/graphics context, shaders, etc.)
    var rendererInfo = render::init(ws.x, ws.y);

    // Set up input system for keyboard, mouse, and gamepad
    input::init();

    // Create the engine info structure that will be returned
    var engineInfo = new EngineInfo();
    engineInfo->renderer = rendererInfo;
    engineInfo->app = app;

    // Initialize audio system for sound playback
    audio::init();
    
    // Initialize object management and scene graph
    obj::init();

    return engineInfo;
}

/**
 * @brief Main engine update loop - called once per frame
 * 
 * This function updates all engine subsystems in the correct order to ensure
 * proper game state progression. The update order is important:
 * 
 * 1. Debug UI (debug builds only) - updated first to capture input
 * 2. Mouse tracking - calculates mouse position and delta for this frame
 * 3. Object system - updates game objects and components
 * 4. Render system - processes draw calls and renders the frame
 * 5. Input system - processes keyboard/mouse/gamepad state for next frame
 * 6. Audio system - updates sound playback
 * 7. Time tracking - calculates delta time for frame-rate independent movement
 * 
 * Should be called once per frame in the main game loop.
 */
void tmt::engine::update()
{
#ifdef DEBUG
    // Update debug UI overlay in debug builds
    debug::DebugUi::Update();
#endif

    // Track mouse position and calculate delta (movement since last frame)
    double xpos = 0;
    double ypos = 0;

    // Get current mouse cursor position from GLFW window
    glfwGetCursorPos(renderer->window, &xpos, &ypos);

    var p = glm::vec2{xpos, ypos};

    // Calculate mouse movement delta (previous position - current position)
    mousedelta = mousep - p;
    mousep = p;

    // Update all game objects and their components
    obj::update();
    
    // Process rendering: draw calls, shader updates, frame presentation
    render::update();
    
    // Update input state (key presses, releases, holds)
    input::Update();
    
    // Update audio system (streaming, 3D audio positioning, etc.)
    audio::update();

    // Calculate delta time - time elapsed since last frame
    // This is used for frame-rate independent movement and animations
    deltaTime = glfwGetTime() - lastTime;
    lastTime = glfwGetTime();
}

/**
 * @brief Cleanly shutdown the engine and free all resources
 * 
 * Performs cleanup in the following order:
 * 1. Destroys the main scene and all game objects
 * 2. Shuts down the rendering system (destroys shaders, textures, buffers)
 * 
 * This should be called before application exit to prevent memory leaks
 * and ensure proper cleanup of GPU resources.
 */
void tmt::engine::shutdown()
{
    // Clean up the main scene and all game objects
    delete mainScene;
    
    // Shutdown rendering system and free GPU resources
    render::shutdown();
}

/**
 * @brief Construct a new Application and initialize the engine
 * 
 * This constructor:
 * 1. Sets the application name (used as window title)
 * 2. Configures 2D vs 3D rendering mode
 * 3. Initializes the entire engine with the specified window size
 * 4. Stores a reference to itself in the engine info
 * 
 * @param name Application title/name (displayed in window title bar)
 * @param width Window width in pixels
 * @param height Window height in pixels
 * @param _2d True for 2D rendering mode, false for 3D mode
 */
tmt::engine::Application::Application(string name, int width, int height, bool _2d)
{
    // Set application properties
    this->name = name;
    this->is2D = _2d;

    // Initialize the engine with this application and specified window size
    info = init(this, glm::vec2(width, height));
    
    // Store reference to this application in the engine info
    info->app = this;
}
