#ifndef AUDIO_H
#define AUDIO_H

#include <miniaudio/extras/miniaudio_split/miniaudio.h>

#include <tomato/utils.hpp>

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
        ma_engine engine;

    private:
        std::vector<SoundListener*> listeners;

    };

    struct Sound
    {
        struct SoundInitInfo
        {
            string name = "NONEAUD";
            bool useSpatialization = true;
        };

        static Sound* CreateSound(string path, SoundInitInfo info = {});

        ~Sound();

        void Play();
        void Stop();

    private:
        friend struct AudioPlayer;
        Sound(string path, SoundInitInfo info = {});

        ma_sound sound;

    };

    struct SoundListener : obj::Object
    {
        int pId = -1;
        glm::vec3 velocity{0};

        SoundListener();
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
