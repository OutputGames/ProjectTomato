/**
 * @file time.cpp
 * @brief Implementation of time utilities
 */

#include "time.hpp"
#include "globals.hpp"

float tmt::time::getTime()
{
    return counterTime;
}

float tmt::time::getSinTime()
{
    return glm::sin(counterTime);
}

float tmt::time::getCosTime()
{
    return glm::cos(counterTime);
}

float tmt::time::getDeltaTime()
{
    return deltaTime;
}

u32 tmt::time::getFrameTime()
{
    return frameTime;
}

/**
 * @brief Block thread execution for specified time
 * Converts seconds to milliseconds and uses std::this_thread::sleep_for
 */
void tmt::time::waitForSeconds(float seconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(seconds * 1000)));
}

/**
 * @brief Block thread execution for specified frames (assuming 60 FPS)
 * Calculates equivalent time and sleeps
 */
void tmt::time::waitForFrames(int frames)
{
    // Assume 60 FPS: each frame is 1/60 seconds
    float seconds = (1 / 60.0f) * frames;

    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(seconds * 1000)));
}
