#ifndef AUDIO_H
#define AUDIO_H


#include <tomato/utils.hpp>

#include <al.h>
#include <alc.h>
#include "Obj/obj.hpp"

namespace tmt::audio
{
    struct SoundListener;


    struct AudioDevice
    {
        static AudioDevice* GetInstance();

        AudioDevice();
        ~AudioDevice();
        void Update();

        void AddListener(SoundListener* listener);
        void RemoveListener(SoundListener* listener);

    private:
        std::vector<SoundListener*> listeners;

        ALCdevice* device;
        ALCcontext* context;

    };

    struct Sound
    {
        struct SoundInitInfo
        {
            string name = "NONEAUD";
            bool useSpatialization = true;
        };

        static Sound* CreateSound(string path, SoundInitInfo info);

        ~Sound();

        void Play();
        void Stop();
        void Pause();

    private:
        friend struct AudioPlayer;
        Sound(string path, SoundInitInfo info);

        ALuint buffer, source;

    };

    struct AudioBuffer
    {
        AudioBuffer(s16* buffer, u16 size, u16 frequency);

        void push();

    private:
        friend struct AudioPLayer;

        ALuint buffer, source;
    };

    struct SoundListener : obj::Object
    {
        int pId = -1;
        glm::vec3 velocity{0};

        SoundListener();
        ~SoundListener() override;
    };

    struct AudioPlayer : obj::Object
    {
        AudioPlayer();

        void play();
        void pause();
        void stop();
        void Update() override;

        void setSound(Sound* sound);
        void playOneShot(Sound* sound);

        float volume = 1.0f;
        bool isLooping = false;
        bool playOnStart = true;
        bool use3dAudio = true;

    private:
        void formatSound(Sound* sound);

        bool isPlaying = false;
        Sound* sound = nullptr;

    };


    void init();
    void update();
} // namespace tmt::audio


#endif // AUDIO_HPP
