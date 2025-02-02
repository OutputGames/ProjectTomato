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

void tmt::time::waitForSeconds(float seconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(seconds * 1000)));
}

void tmt::time::waitForFrames(int frames)
{
    float seconds = (1 / 60.0f) * frames;

    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(seconds * 1000)));
}
