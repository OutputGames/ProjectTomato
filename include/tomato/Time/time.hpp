#ifndef TIME_H
#define TIME_H

#include "utils.hpp"


namespace tmt::time
{


    float getTime();

    float getSinTime();

    float getCosTime();

    float getDeltaTime();

    u32 getFrameTime();


    void waitForSeconds(float seconds);
    void waitForFrames(int frames);
}

#endif
