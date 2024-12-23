

#include "audio.hpp"


#include "miniaudio.h"

using namespace tmt::audio;

#define CHECK_RESULT(result)                                                                                           \
    if (result != MA_SUCCESS)                                                                                          \
    {                                                                                                                  \
        printf("Error: %s\n", ma_result_description(result));                                                          \
        return;                                                                                                   \
    }


tmt::audio::AudioDevice* AudioDevice::mInstance;

ma_resource_manager resource_manager;

ma_sound_group defaultGroup;

std::vector<ma_sound> sounds;

ma_sound* GetSound(Sound* sound) { return &sounds[sound->pId]; }

void log_callback(void* pUserData, ma_uint32 level, const char* pMessage)
{
    std ::cout << "Level: " << level << " " << pMessage;
}

ma_engine* GetEngine() { return (ma_engine*)AudioDevice::mInstance->engine; }

tmt::audio::AudioDevice::AudioDevice()
{
    mInstance = this;
    ma_result result;

    ma_engine e;

    result = ma_engine_init(nullptr, &e);
    if (result != MA_SUCCESS)
    {
        std::cout << "Unable to initialize audio engine." << std::endl;
        return;
    }

    engine = &e;
}

AudioDevice::~AudioDevice()
{
    ma_engine_uninit(static_cast<ma_engine*>(engine));
    //ma_resource_manager_uninit(&resource_manager);
}

void AudioDevice::Update()
{
    var eng = GetEngine();
    for (SoundListener* listener : listeners)
    {
        ma_engine_listener_set_position(eng, listener->pId, TO_ARGS(listener->GetGlobalPosition()));
        ma_engine_listener_set_velocity(eng, listener->pId, TO_ARGS(listener->velocity));

        var up = listener->GetUp();
        var fwd = listener->GetForward();

        ma_engine_listener_set_world_up(eng, listener->pId, TO_ARGS(up));
        ma_engine_listener_set_direction(eng, listener->pId, TO_ARGS(fwd));
    }

}

void AudioDevice::AddListener(SoundListener* listener)
{
    listener->pId = listeners.size();
    listeners.push_back(listener);
}

tmt::audio::Sound::Sound(string path)
{
    ma_sound sound;
    var result = ma_sound_init_from_file(GetEngine(), path.c_str(), 0,
        nullptr, nullptr, &sound);
    if (result != MA_SUCCESS)
    {
        printf("WARNING: Failed to load sound \"%s\"", path.c_str());
        return;
    }
    ma_sound_start(&sound);

    ma_sound_set_spatialization_enabled(&sound, true);

    pId = sounds.size();
    sounds.push_back(sound);

}

Sound::~Sound() { ma_sound_uninit(&sounds[pId]); }

void Sound::Play()
{
    var sound = GetSound(this);

    ma_sound_start(sound);
}

void Sound::Stop()
{
    var sound = GetSound(this);

    ma_sound_stop(sound);
}

SoundListener::SoundListener()
{ AudioDevice::mInstance->AddListener(this); }

AudioPlayer::AudioPlayer()
{

}

void AudioPlayer::play()
{ isPlaying = true; }

void AudioPlayer::pause()
{ isPlaying = false; }

void AudioPlayer::stop()
{ isPlaying = false; }

void AudioPlayer::formatSound(Sound* s)
{
    var sound = GetSound(s);
    var fwd = GetForward();

    ma_sound_set_position(sound, TO_ARGS(GetGlobalPosition()));
    ma_sound_set_direction(sound, TO_ARGS(fwd));
    ma_sound_set_volume(sound, volume);
    ma_sound_set_positioning(sound, ma_positioning_absolute);
    ma_sound_set_looping(sound, true);
}

void AudioPlayer::Update()
{


    Object::Update();
}


void AudioPlayer::setSound(Sound* sound)
{
    this->sound = sound;
}

void AudioPlayer::playOneShot(Sound* sound)
{
    formatSound(sound);
    sound->Play();
}

void tmt::audio::init()
{ var audioDevice = new AudioDevice(); }

void tmt::audio::update()
{ AudioDevice::mInstance->Update(); }
