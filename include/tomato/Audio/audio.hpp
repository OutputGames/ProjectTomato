#ifndef AUDIO_H
#define AUDIO_H

#include <tomato/utils.hpp>

#include <tomato/Obj/obj.hpp>

namespace tmt::audio
{
    struct SoundListener;

    struct AudioDevice 
    {
        static tmt::audio::AudioDevice* mInstance;

        AudioDevice();
        ~AudioDevice();
        void Update();

        void AddListener(SoundListener* listener);

    private:

        std::vector<SoundListener*> listeners;

    };

    struct Sound
    {
        int pId = -1;
        Sound(string path);
        ~Sound();

        void Play();
        void Stop();

    private:


    };

    struct SoundListener : tmt::obj::Object
    {
        int pId = -1;
        glm::vec3 velocity{0};

        SoundListener();
    };

    struct AudioPlayer : tmt::obj::Object
    {
        AudioPlayer();

        void play();
        void pause();
        void stop();
        void Update() override;

        void setSound(Sound* sound);
        void playOneShot(Sound* sound);

        float volume = 1.0f;

    private:

        void formatSound(Sound* sound);

        bool isPlaying = false;
        bool isLooping = false;
        Sound* sound;

    };


    void init();
    void update();
} // namespace tmt::audio


#endif // AUDIO_HPP
