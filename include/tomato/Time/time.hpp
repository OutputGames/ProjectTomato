/**
 * @file time.hpp
 * @brief Time and frame tracking utilities
 * 
 * Provides functions to access:
 * - Current time and frame count
 * - Delta time for frame-independent movement
 * - Trigonometric time functions for animations
 * - Blocking wait functions
 */

#ifndef TIME_H
#define TIME_H

#include "utils.hpp"


namespace tmt::time
{

    /**
     * @brief Get the current time counter
     * @return float Time value (increments each frame)
     */
    float getTime();

    /**
     * @brief Get sine of current time
     * @return float sin(time) - useful for smooth oscillating animations
     */
    float getSinTime();

    /**
     * @brief Get cosine of current time
     * @return float cos(time) - useful for smooth oscillating animations
     */
    float getCosTime();

    /**
     * @brief Get time elapsed since last frame
     * 
     * Use this to make movement frame-rate independent:
     * position += velocity * getDeltaTime();
     * 
     * @return float Delta time in seconds
     */
    float getDeltaTime();

    /**
     * @brief Get the current frame number
     * @return u32 Current frame count
     */
    u32 getFrameTime();

    /**
     * @brief Block execution for a specified time
     * 
     * Warning: This blocks the entire thread. Use sparingly.
     * 
     * @param seconds Time to wait in seconds
     */
    void waitForSeconds(float seconds);
    
    /**
     * @brief Block execution for a specified number of frames (assumes 60 FPS)
     * 
     * Warning: This blocks the entire thread. Use sparingly.
     * 
     * @param frames Number of frames to wait (at 60 FPS)
     */
    void waitForFrames(int frames);
}

#endif
