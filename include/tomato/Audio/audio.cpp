#include "audio.hpp"

#include "AudioFile.h"
#include "Time/time.hpp"

using namespace tmt::audio;


AudioDevice* mInstance;


AudioDevice* AudioDevice::GetInstance()
{
    return mInstance;
}

AudioDevice::AudioDevice()
{
    mInstance = this;


    device = alcOpenDevice(nullptr);
    context = alcCreateContext(device, nullptr);
    alcMakeContextCurrent(context);
}

AudioDevice::~AudioDevice()
{
    alcDestroyContext(context);
    alcCloseDevice(device);
}

void AudioDevice::Update()
{
    for (SoundListener* listener : listeners)
    {
        var p = listener->GetGlobalPosition();

        var up = listener->GetUp();
        var fwd = listener->GetForward();

    }

}

void AudioDevice::AddListener(SoundListener* listener)
{
    listener->pId = listeners.size();
    listeners.push_back(listener);
}

void AudioDevice::RemoveListener(SoundListener* _listener)
{
    for (auto listener : listeners)
    {
        if (listener->pId >= _listener->pId)
            listener->pId--;
    }

    listeners.erase(VEC_FIND(listeners, _listener));
}

Sound::Sound(string path, SoundInitInfo info)
{
    AudioFile<s16> audioFile(path);

    var buffer = new s16[audioFile.getNumSamplesPerChannel() * 1];

    int channel = 0;
    int numSamples = audioFile.getNumSamplesPerChannel();

    for (int i = 0; i < numSamples; i++)
    {
        s16 currentSample = audioFile.samples[channel][i];

        buffer[i] = currentSample;
    }

    alGenBuffers(1, &this->buffer);
    alGenSources(1, &source);

    alBufferData(this->buffer, AL_FORMAT_MONO16, buffer, audioFile.getNumSamplesPerChannel() * sizeof(s16),
                 audioFile.getSampleRate());
    alSourcei(source, AL_BUFFER, this->buffer);

    fs::ResourceManager::pInstance->loaded_sounds[info.name] = this;
}

AudioBuffer::AudioBuffer(s16* buffer, u16 size, u16 frequency)
{
    alGenBuffers(1, &this->buffer);
    alGenSources(1, &source);

    alBufferData(this->buffer, AL_FORMAT_MONO16, buffer, size, frequency);
    alSourcei(source, AL_BUFFER, this->buffer);
}

void AudioBuffer::push()
{
    alSourcePlay(source);
}

Sound* Sound::CreateSound(string path, SoundInitInfo info)
{
    if (info.name == "NONEAUD")
    {
        info.name = std::generateRandomString(10) + (std::filesystem::path(path).filename().string());
    }
    if (fs::ResourceManager::pInstance->loaded_sounds.contains(info.name))
    {
        return fs::ResourceManager::pInstance->loaded_sounds[info.name];
    }

    var sound = new Sound(path, info);

    return sound;


}

Sound::~Sound()
{
}

void Sound::Play()
{
    alSourcePlay(source);
}

void Sound::Stop()
{
    alSourceStop(source);
}

void Sound::Pause()
{
    alSourcePause(source);
}

SoundListener::SoundListener() { mInstance->AddListener(this); }

SoundListener::~SoundListener()
{
    mInstance->RemoveListener(this);
}

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

    var fwd = GetForward();
    var pos = GetGlobalPosition();

    if (!use3dAudio)
    {

    }
    else
    {

    }
}

void AudioPlayer::Update()
{
    var t = time::getTime();
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
{
    var audioDevice = new AudioDevice();
}

void tmt::audio::update()
{
    mInstance->Update();
}
