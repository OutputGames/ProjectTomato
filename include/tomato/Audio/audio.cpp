
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio/miniaudio.h"

#include "audio.hpp"


using namespace tmt::audio;

tmt::audio::AudioDevice* AudioDevice::mInstance;

ma_engine engine;
ma_device device;
ma_context context;
ma_resource_manager resource_manager;

ma_sound_group defaultGroup;

std::vector<ma_sound> sounds;

ma_sound* GetSound(Sound* sound) { return &sounds[sound->pId]; }

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    (void)pInput;

    /*
    Since we're managing the underlying device ourselves, we need to read from the engine directly.
    To do this we need access to the ma_engine object which we passed in to the user data. One
    advantage of this is that you could do your own audio processing in addition to the engine's
    standard processing.
    */
    ma_engine_read_pcm_frames((ma_engine*)pDevice->pUserData, pOutput, frameCount, NULL);
}

void log_callback(void* pUserData, ma_uint32 level, const char* pMessage)
{
    std ::cout << "Level: " << level << " " << pMessage;
}

tmt::audio::AudioDevice::AudioDevice()
{
    mInstance = this;
    ma_result result;
    ma_device_info* pPlaybackDeviceInfos;
    u32 playbackDeviceCount;

    var resource_mgr_cfg = ma_resource_manager_config_init();
    resource_mgr_cfg.decodedFormat = ma_format_f32;
    resource_mgr_cfg.decodedChannels = 0;
    resource_mgr_cfg.decodedSampleRate = 48000;

    result = ma_resource_manager_init(&resource_mgr_cfg, &resource_manager);
    if (result != MA_SUCCESS)
    {
        return;
    }
    
    /* We're going to want a context so we can enumerate our playback devices. */
    result = ma_context_init(NULL, 0, NULL, &context);
    if (result != MA_SUCCESS)
    {
        return;
    }

    result = ma_context_get_devices(&context, &pPlaybackDeviceInfos, &playbackDeviceCount, NULL, NULL);
    if (result != MA_SUCCESS)
    {
        ma_context_uninit(&context);
        return;
    }
    int deviceId = 0;
    for (int i = 0; i < playbackDeviceCount; ++i)
    {
        printf("%d: %s\n", i, pPlaybackDeviceInfos[i].name);
        if (pPlaybackDeviceInfos[i].isDefault)
        {
            deviceId = i;
            break;
        }
    }

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format = resource_manager.config.decodedFormat;
    deviceConfig.playback.channels = 0;
    deviceConfig.playback.pDeviceID = &pPlaybackDeviceInfos[deviceId].id;
    deviceConfig.sampleRate = resource_manager.config.decodedSampleRate;
    deviceConfig.pUserData = &engine;
    deviceConfig.dataCallback = data_callback;

    result = ma_device_init(&context, &deviceConfig, &device);
    if (result != MA_SUCCESS)
    {
        return;
    }

    var pConfig = ma_engine_config_init();
    pConfig.pDevice = &device;
    pConfig.pResourceManager = &resource_manager;
    //pConfig.noAutoStart = MA_TRUE;
    pConfig.listenerCount = 1;

    ma_log log;
    result = ma_log_init(NULL, &log);
    ma_log_register_callback(&log, ma_log_callback_init(log_callback, nullptr));

    pConfig.pLog = &log;

    result = ma_engine_init(&pConfig, &engine);
    if (result != MA_SUCCESS)
    {
        return;
    }

    ma_engine_play_sound(&engine, "resources/sound/call.wav", NULL);

}

AudioDevice::~AudioDevice()
{
    ma_engine_uninit(&engine);
    ma_device_uninit(&device);
    ma_context_uninit(&context);
    ma_resource_manager_uninit(&resource_manager);
}

void AudioDevice::Update()
{
    for (SoundListener* listener : listeners)
    {
        ma_engine_listener_set_position(&engine, listener->pId, TO_ARGS(listener->GetGlobalPosition()));
        ma_engine_listener_set_velocity(&engine, listener->pId, TO_ARGS(listener->velocity));

        var up = listener->GetUp();
        var fwd = listener->GetForward();

        ma_engine_listener_set_world_up(&engine, listener->pId, TO_ARGS(up));
        ma_engine_listener_set_direction(&engine, listener->pId, TO_ARGS(fwd));
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
    var result = ma_sound_init_from_file(
        &engine, path.c_str(), MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_DECODE | MA_RESOURCE_MANAGER_DATA_SOURCE_FLAG_ASYNC,
        NULL, NULL, &sound);
    if (result != MA_SUCCESS)
    {
        printf("WARNING: Failed to load sound \"%s\"", path);
        return;
    }

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
