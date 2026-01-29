cmake_minimum_required(VERSION 3.15)
project(TomatoEngine VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(TOMATO_BUILD_EXAMPLES "Build example applications" ON)
option(TOMATO_BUILD_SHARED "Build shared library" OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Platform-specific settings
if(WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DNOMINMAX)
endif()

# Vendor libraries
set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor)

# GLM (header-only)
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE ${VENDOR_DIR}/glm)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${VENDOR_DIR}/glfw)

# Assimp
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)
add_subdirectory(${VENDOR_DIR}/assimp)

# Bullet Physics
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_CPU_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(${VENDOR_DIR}/bullet3)

# bx, bimg, bgfx (graphics libraries)
# Note: These need special handling as they use a different build system
# For now, we'll include them as header-only or link pre-built libraries
add_library(bx INTERFACE)
target_include_directories(bx INTERFACE 
    ${VENDOR_DIR}/bx/include
)
# Add platform-specific compatibility includes for bx
if(MSVC)
    target_include_directories(bx INTERFACE ${VENDOR_DIR}/bx/include/compat/msvc)
elseif(MINGW)
    target_include_directories(bx INTERFACE ${VENDOR_DIR}/bx/include/compat/mingw)
elseif(APPLE)
    target_include_directories(bx INTERFACE ${VENDOR_DIR}/bx/include/compat/osx)
endif()
# Add BX_CONFIG_DEBUG definition as INTERFACE so it propagates to users of bx
target_compile_definitions(bx INTERFACE
    $<$<CONFIG:Debug>:BX_CONFIG_DEBUG=1>
    $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>:BX_CONFIG_DEBUG=0>
    # Fallback for when build type is not set (default to Debug behavior)
    $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>>:BX_CONFIG_DEBUG=1>
)
# Add MSVC-specific compiler options for bx (required by bx/platform.h)
if(MSVC)
    target_compile_options(bx INTERFACE /Zc:preprocessor)
endif()

add_library(bimg INTERFACE)
target_include_directories(bimg INTERFACE ${VENDOR_DIR}/bimg/include)

add_library(bgfx INTERFACE)
target_include_directories(bgfx INTERFACE ${VENDOR_DIR}/bgfx/include)

# STB (header-only)
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${VENDOR_DIR}/stb)

# Dear ImGui
file(GLOB IMGUI_SOURCES 
    ${VENDOR_DIR}/dear-imgui/*.cpp
    ${VENDOR_DIR}/dear-imgui/*.h
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC 
    ${VENDOR_DIR}/dear-imgui
    ${VENDOR_DIR}  # Add vendor dir so imgui can find stb/stb_*.h
)
target_link_libraries(imgui PUBLIC glfw stb bx)

# Add BX_CONFIG_DEBUG definition for imgui
target_compile_definitions(imgui PRIVATE
    $<$<CONFIG:Debug>:BX_CONFIG_DEBUG=1>
    $<$<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>:BX_CONFIG_DEBUG=0>
    # Fallback for when build type is not set (default to Debug behavior)
    $<$<NOT:$<OR:$<CONFIG:Debug>,$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>>:BX_CONFIG_DEBUG=1>
)

# Add MSVC-specific compiler options for imgui
if(MSVC)
    target_compile_options(imgui PRIVATE /Zc:preprocessor)
endif()

# FreeType
add_subdirectory(${VENDOR_DIR}/freetype)

# Box2D
add_subdirectory(${VENDOR_DIR}/box2d)

# ENet
file(GLOB ENET_SOURCES ${VENDOR_DIR}/enet/*.c)
add_library(enet STATIC ${ENET_SOURCES})
target_include_directories(enet PUBLIC ${VENDOR_DIR}/enet/include)
if(WIN32)
    target_link_libraries(enet PRIVATE ws2_32 winmm)
endif()

# TMGL (Tomato Graphics Library)
add_subdirectory(${VENDOR_DIR}/tmgl)

# Add bgfx/bimg/bx include directories to tmgl target after it's created
target_include_directories(tmgl PUBLIC 
    ${VENDOR_DIR}/bgfx/include
    ${VENDOR_DIR}/bimg/include
    ${VENDOR_DIR}/bx/include
)

# Add platform-specific bx compatibility includes
if(MSVC)
    target_include_directories(tmgl PUBLIC ${VENDOR_DIR}/bx/include/compat/msvc)
elseif(MINGW)
    target_include_directories(tmgl PUBLIC ${VENDOR_DIR}/bx/include/compat/mingw)
elseif(APPLE)
    target_include_directories(tmgl PUBLIC ${VENDOR_DIR}/bx/include/compat/osx)
endif()

# Tomato Engine Library
file(GLOB_RECURSE TOMATO_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tomato/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tomato/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tomato_generated/*.h
)

if(TOMATO_BUILD_SHARED)
    add_library(TomatoEngine SHARED ${TOMATO_SOURCES})
    target_compile_definitions(TomatoEngine PRIVATE TOMATO_DLLBUILD)
else()
    add_library(TomatoEngine STATIC ${TOMATO_SOURCES})
endif()

# Include directories
target_include_directories(TomatoEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tomato
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor
    ${VENDOR_DIR}/glm
    ${VENDOR_DIR}/assimp/include
    ${VENDOR_DIR}/bullet3/src
    ${VENDOR_DIR}/stb
    ${VENDOR_DIR}/dear-imgui
)

# Link libraries
target_link_libraries(TomatoEngine PUBLIC
    glm
    glfw
    assimp
    BulletDynamics
    BulletCollision
    LinearMath
    bx
    bimg
    bgfx
    imgui
    freetype
    box2d
    enet
    tmgl
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(TomatoEngine PUBLIC
        opengl32
        gdi32
        user32
        kernel32
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(TomatoEngine PUBLIC
        GL
        X11
        pthread
        dl
    )
elseif(APPLE)
    target_link_libraries(TomatoEngine PUBLIC
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()

# Set compiler warnings
if(MSVC)
    target_compile_options(TomatoEngine PRIVATE /W3 /Zc:__cplusplus /Zc:preprocessor)
else()
    target_compile_options(TomatoEngine PRIVATE -Wall -Wextra)
endif()

# Build examples
if(TOMATO_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Install targets
install(TARGETS TomatoEngine
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/tomato
    DESTINATION include
    FILES_MATCHING PATTERN "*.hpp" PATTERN "*.h"
)
