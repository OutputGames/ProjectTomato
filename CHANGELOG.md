# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog],
and this project adheres to [Semantic Versioning].

## 0.0.3 - 2024-6-25
### Added

* **Script systen using Mono**
* **Separation of engine and runtime**
* Application compiler using MSBuild (needs reviewing)
* Editor UI using ImGUI
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
* **UNLOADING & DELETING**


## 0.0.2 - 2024-6-8
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


## 0.0.1 - 2024-6-8

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
