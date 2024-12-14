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
