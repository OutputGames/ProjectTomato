

#include "audio.hpp"

#include "Time/time.hpp"


using namespace tmt::audio;

#define CHECK_RESULT(result)                                                                                           \
    if (result != MA_SUCCESS)                                                                                          \
    {                                                                                                                  \
        printf("Error: %s\n", ma_result_description(result));                                                          \
        return;                                                                                                   \
    }


tmt::audio::AudioDevice* mInstance;

ma_resource_manager resource_manager;

ma_sound_group defaultGroup;

void log_callback(void* pUserData, ma_uint32 level, const char* pMessage)
{
    std ::cout << "Level: " << level << " " << pMessage;
}

tmt::audio::AudioDevice* AudioDevice::GetInstance()
{ return mInstance; }

tmt::audio::AudioDevice::AudioDevice()
{
    mInstance = this;
    ma_result result;

    ma_engine_config pConfig = ma_engine_config_init();
    pConfig.listenerCount = 1;

    result = ma_engine_init(&pConfig, &engine);
    if (result != MA_SUCCESS)
    {
        std::cout << "Unable to initialize audio engine." << std::endl;
        return;
    }
}

AudioDevice::~AudioDevice()
{
    ma_engine_uninit(&mInstance->engine);
    //ma_resource_manager_uninit(&resource_manager);
}

void AudioDevice::Update()
{
    for (SoundListener* listener : listeners)
    {
        var p = listener->GetGlobalPosition();
        ma_engine_listener_set_position(&mInstance->engine, listener->pId, TO_ARGS(p));
        ma_engine_listener_set_velocity(&mInstance->engine, listener->pId, TO_ARGS(listener->velocity));

        var up = listener->GetUp();
        var fwd = listener->GetForward();

        ma_engine_listener_set_world_up(&mInstance->engine, listener->pId, TO_ARGS(up));
        ma_engine_listener_set_direction(&mInstance->engine, listener->pId, TO_ARGS(-fwd));
        ma_engine_listener_set_enabled(&mInstance->engine, listener->pId, true);
    }

}

void AudioDevice::AddListener(SoundListener* listener)
{
    listener->pId = listeners.size();
    listeners.push_back(listener);
}

tmt::audio::Sound::Sound(string path, SoundInitInfo info)
{
    var result = ma_sound_init_from_file(&mInstance->engine, path.c_str(), info.useSpatialization ? 0 : MA_SOUND_FLAG_NO_SPATIALIZATION,
        nullptr, nullptr, &sound);
    if (result != MA_SUCCESS)
    {
        printf("WARNING: Failed to load sound \"%s\"", path.c_str());
        return;
    }

    if (info.useSpatialization)
    {
    }
    //ma_sound_set_rolloff(&sound, 10.0f);
    ma_sound_set_volume(&sound, 0);

}

Sound::~Sound() { ma_sound_uninit(&sound); }

void Sound::Play()
{
    ma_sound_start(&sound);
}

void Sound::Stop()
{

    ma_sound_stop(&sound);
}

SoundListener::SoundListener() { mInstance->AddListener(this); }

AudioPlayer::AudioPlayer()
{

}

void AudioPlayer::play()
{
    if (sound)
    {
        isPlaying = true;
        formatSound(sound);
        sound->Play();
    }
}

void AudioPlayer::pause()
{
    isPlaying = false;
}

void AudioPlayer::stop()
{
    isPlaying = false;
    sound->Stop();
}

void AudioPlayer::formatSound(Sound* s)
{
    if (!s)
        return;
    var sound = s->sound;
    var fwd = GetForward();
    var pos = GetGlobalPosition();

    ma_sound_set_volume(&sound, volume);
    ma_sound_set_looping(&sound, isLooping);
    ma_sound_set_spatialization_enabled(&sound, use3dAudio);

    if (!use3dAudio)
    {
        ma_sound_set_attenuation_model(&sound, ma_attenuation_model_none);
    }
    else
    {
        ma_sound_set_spatialization_enabled(&sound, true);
        ma_sound_set_pinned_listener_index(&sound, 0);
        ma_sound_set_attenuation_model(&sound, ma_attenuation_model_linear);
        ma_sound_set_position(&sound, TO_ARGS(pos));
        ma_sound_set_direction(&sound, TO_ARGS(fwd));
        ma_sound_set_positioning(&sound, ma_positioning_absolute);
        ma_sound_set_volume(&sound, volume * 10);
    }
}

void AudioPlayer::Update()
{
    var t = tmt::time::getTime();
    //formatSound(sound);

    if (t == 0)
    {
        if (playOnStart)
        {
            play();
        }
    }

    Object::Update();
}


void AudioPlayer::setSound(Sound* s)
{
    this->sound = s;
}

void AudioPlayer::playOneShot(Sound* sound)
{
    formatSound(sound);
    sound->Play();
}

void tmt::audio::init()
{ var audioDevice = new AudioDevice(); }

void tmt::audio::update()
{mInstance->Update(); }
