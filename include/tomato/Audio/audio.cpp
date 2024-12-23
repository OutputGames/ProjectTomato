#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio/miniaudio.h"

#include "audio.hpp"


tmt::audio::AudioDevice* audioDevice;
ma_engine engine;

tmt::audio::AudioDevice::AudioDevice()
{
    audioDevice = this;

    ma_result result;


    result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS)
    {
        printf("Failed to initialize audio engine.");
        return;
    }

    ma_engine_play_sound(&engine, "resources/sound/test.wav", NULL);

}

void tmt::audio::init()
{
    audioDevice = new AudioDevice;
}
