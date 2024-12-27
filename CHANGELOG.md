# Changelog & Info

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog],
and this project adheres to [Semantic Versioning].

# Sources Used
* 

# FAQ

* Why didn't you use the blinn-phong lighting model when moving between Phong and PBR?
    * The Blinn-Phong lighting model isn't physically accurate because it returns more "light" than it was given, and there are specific rules to having realistic lighting such as **returning less than what was given.** I would recommend watching [this video](https://www.youtube.com/watch?v=KkOkx0FiHDA) which explains how lighting models work.

## 0.0.1a - 

## 0.05p - 15-7-2024

### Added

* Debug drawing using *glampert*'s `debug-draw` system

### Changed

* Render system now has a callback system that will be used in sync with the draw-call system

#### Optimizations

### In Progress (at commit time)

* Frustum cullng system
* ShaderFactory implementation
* Physics system (engine not chosen yet)
* Audio system using OpenAL

## 0.04p - 14-7-2024

### Added

* ### Lighting system
    * Light objects (Point, Directional, Spot)
    * Skybox/Cubemap loading (with [irradiance generation](https://learnopengl.com/PBR/IBL/Diffuse-irradiance) and [prefiltering](https://learnopengl.com/PBR/IBL/Specular-IBL))
    * [BRDF (Bidirectional reflectance distribution function)](https://en.wikipedia.org/wiki/Bidirectional_reflectance_distribution_function) & [IBL (Image Based Lighting)](https://en.wikipedia.org/wiki/Image-based_lighting) integration using the [Disney BRDF](https://disneyanimation.com/publications/physically-based-shading-at-disney/)
    * #### Shadow mapping
        * PCM used for antialiasing
        * CSM (cascaded shadow mapping) planned

* ### Text Rendering
    * Using freetype/ttf fonts
    * SDF (signed distance field) based fonts planned

* ### Skeletal Animation
    * 2 new components: `tmAnimator` and `tmSkeleton`

* Input mangement
* Revised material system and shader reflection
* Entity Updates
    * Deleting, Duplicating
* Unloading and Garbage Collection
    * Most components still need implementation
* RenderDoc can now be injected directly from the application
* #### Editor Updates
    * User script loading & assembly/runtime reloading
        * `MonoComponent` now can take an assembly index in it's constructor
    * Asset Browser
        * Many new types: `Asset`, `ResourceMgr`
    * Entity picking
    * ImGUI extensions added such as a file picker and a texture picker


### Changed

* Default deferred shader now uses [PBR](https://en.wikipedia.org/wiki/Physically_based_rendering) instead of [Phong](https://en.wikipedia.org/wiki/Phong_reflection_model)
* Editor camera now has input and a fly-cam
* Texture class can now be initialized using a shader
* Logger class now actually logs entries in a list
* Editor UI now has Explorer and **Properties** windows
    * Added `EngineRender` and `GetIcon` functions to component classes
* Materials now have shaders' fields and properties allowing them to be changed through the editor
* Actors can now be selected through the editor by just clicking on them using the Red channel of the `Shader` pass in the deferred pipeline 
* Actors can now be found from their name recursively
* Textures are now loaded using linear filtering instead of point filtering
* Metadata is now stored on system through the editor
* Default shaders are now stored in the code
* Draw calls can now be overriden using sub data insertion and material field replacement
* Framebuffers can now be created using depth only
* New position based pass added for shadow position
* Single-channel textures use `GL_RED` instead of `GL_R`

#### Optimizations

* `tmShader::Use` now caches the currently used shader (???%)
* Dictionary class can now be indexed using an integer (???%)
* `fs::canonical` replaced with another algo used to check if asset is in current directory (2.36%)
* replaced in-effiecient loop-based matrix array insertion with a new function, `setMat4Array` which sends an array of matrices in one call (11.36%)
* changed bone map in the `tmAnimator` class from a vector of bones to a unordered map of string and bones (15.49%)
* added move semantics, map capacity, and repeated access to `tmSkinnedMeshRenderer::Update` (30.18%)

#### Engine now runs at 120+ FPS with 4 skeletal animation calculations simultaneously and resource calculation upgrades.

### In Progress (at commit time)

* Frustum cullng system
* ShaderFactory implementation
* Physics system (engine not chosen yet)
* Audio system using OpenAL

## 0.0.3p - 25-6-2024
### Added

* **Script systen using [Mono](https://en.wikipedia.org/wiki/Mono_(software))**
* **Separation of engine and runtime**
* Application compiler using [MSBuild](https://en.wikipedia.org/wiki/MSBuild) (needs reviewing)
* Editor UI using [Dear ImGui](https://en.wikipedia.org/wiki/Immediate_mode_GUI)
* [Trello progress board](https://trello.com/invite/b/ZtBMbEzQ/ATTIb7b5fd194ee4a20ca691a740d63d338f22675A7D/main)
* Project/Game serialization system

* `RuntimeStart` & `RuntimeUpdate` now work

* Logger class
    * Replaced all instances of `std::cout` with `Logger::logger`

* Main new components:
    * tmCamera
        * Main camera class
    * tmRenderer
        * Base renderer class
    * tmMeshRenderer
        * Mesh renderer class
    * MonoComponent
        * Script handler
* New Functions:
    * `RunCommand` for running batch commands (thread unsafe)
    * `ReplaceAll` for replacing all occurences of certain characters in a string.
    * `glm::to_json` for copying `glm::vec2/vec3` to `nlohmann::json`

    * `glm::from_json` for getting `glm::vec2/vec3` from `nlohmann:json`

    * `tmeFullUpdate` mostly used from the compiled games to call loop functions without calling multiple functions between C# and C++

    * `tmeStartLoop` used to start render loop from compiled game

    * `tmeLoad` used to load full game from a `.tmg` file

    * `tmEngine::SerializeGame` & `tmEngine::DeserializeGame` used to save and load games

### Changed

* `tmFramebuffer` no longer uses the screen size as a default size

* `tmfs` now has `writeFileString`, `copyFile`, `fileExists`, and `copyDirectory`
    * `loadFileBytes` now uses a `uint32_t` instead of an `int` for data size

### Deprecated

### Removed

### Fixed

* `RenderMgr::Clear` has been changed/fixed to prevent a memory leak

* Framebuffer system now works without deferred rendering (forward rendering)

### In Progress (at commit time)

* User script loading
* Resource packing using RRes
* UI system with text rendering
* Lighting system
    * Shadow mapping
    * Skyboxes
* Skeletal animation
* Post processing system
* Unloading & Deleting


## 0.0.2p - 8-6-2024
### Added

* Framebuffer system w/ **deferred rendering**
* Model Loading using [Assimp](https://github.com/assimp/assimp)
* Vertex Buffer and Draw Call System

### Changed

* Camera now uses framebuffer system
* `tmeEngine` now provides the screen dimensions


### Deprecated

### Removed

### Fixed

### In Progress (at commit time)


## 0.0.1p - 8-6-2024

### Added

- initial commit

<!--
### Changed

### Deprecated

### Removed

### Fixed

-->

### In Progress (at commit time)

* vertex buffer system
* framebuffer/render textures
* model loading

### Planned

* x86 support
* Mono Runtime

<!-- Links -->
[keep a changelog]: https://keepachangelog.com/en/1.0.0/
[semantic versioning]: https://semver.org/spec/v2.0.0.html
[Deko3d]: https://github.com/devkitPro/deko3d
[Metal]: https://developer.apple.com/metal/
[Citro3d]: https://github.com/devkitPro/citro3d/tree/master

<!-- Versions -->
[unreleased]: https://github.com/Author/Repository/compare/v0.0.2...HEAD
