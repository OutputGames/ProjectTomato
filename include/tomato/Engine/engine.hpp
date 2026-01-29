/**
 * @file engine.hpp
 * @brief Core engine initialization, update loop, and application management
 * 
 * This header defines the main engine structures and lifecycle functions.
 * The engine manages initialization of all subsystems (render, audio, input, etc.),
 * handles the main update loop, and provides clean shutdown functionality.
 */

#ifndef ENGINE_H
#define ENGINE_H

#include "utils.hpp"


namespace tmt::render
{
    struct RendererInfo;
}


namespace tmt::engine
{
    struct Application;

    struct EngineInfo;

    /**
     * @brief Contains references to core engine subsystems
     * 
     * This structure holds pointers to the renderer and application,
     * allowing easy access to the engine's primary components.
     */
    struct EngineInfo
    {
        render::RendererInfo* renderer;  ///< Pointer to the rendering subsystem
        Application* app;                ///< Pointer to the application instance
    };

    /**
     * @brief Initialize the game engine and all subsystems
     * 
     * Sets up the rendering system, input handlers, audio system, and object management.
     * This must be called before any other engine operations.
     * 
     * @param app Pointer to the Application instance
     * @param w Window size as a 2D vector (width, height)
     * @return EngineInfo* Pointer to the initialized engine information structure
     */
    EngineInfo* init(Application* app, glm::vec2 w);

    /**
     * @brief Main engine update loop
     * 
     * Updates all subsystems in the correct order:
     * 1. Debug UI (if in debug mode)
     * 2. Mouse position tracking
     * 3. Object system
     * 4. Rendering system
     * 5. Input handling
     * 6. Audio system
     * 7. Delta time calculation
     * 
     * Should be called once per frame in the main game loop.
     */
    void update();

    /**
     * @brief Cleanly shutdown the engine and free resources
     * 
     * Destroys the main scene and shuts down the rendering system.
     * This should be called before application exit to prevent memory leaks.
     */
    void shutdown();

    /**
     * @brief Represents a game application instance
     * 
     * The Application class encapsulates a game or application that uses the engine.
     * It handles the window creation and basic application properties.
     */
    struct Application
    {
        string name;           ///< Application/window title
        bool is2D;             ///< Whether this is a 2D or 3D application
        EngineInfo* info;      ///< Pointer to engine information

        /**
         * @brief Construct a new Application and initialize the engine
         * 
         * @param name Application title/name
         * @param width Window width in pixels
         * @param height Window height in pixels
         * @param is2D True for 2D mode, false for 3D mode
         */
        Application(string name, int width, int height, bool is2D);
    };

}

#endif
