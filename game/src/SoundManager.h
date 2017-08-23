#ifndef _SoundManagerClass_
#define _SoundManagerClass_

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/Music.hpp>

#include <unordered_map>

namespace o2d {
    struct Sound
    {
        uint8_t id;
        sf::Sound* sound;
        bool loop;
    };

    struct Music
    {
        uint8_t id;
        sf::Music* music;
        bool loop;
    };
}

class SoundManager
{
private:

    static std::unordered_map<std::string, sf::SoundBuffer*> soundBuffers;

    static std::unordered_map<uint8_t, o2d::Sound*> sounds;
    static uint8_t soundId;

    static std::unordered_map<uint8_t, o2d::Music*> musics;
    static uint8_t musicId;

protected:

    SoundManager () {}
    ~SoundManager() {}

public:

    static void cleanUp()
    {
        for (auto it = soundBuffers.begin(); it != soundBuffers.end(); soundBuffers.erase(it++)) {
            delete it->second;
        }

        for (auto it = sounds.begin(); it != sounds.end(); sounds.erase(it++)) {
            delete it->second->sound;
            delete it->second;
        }

        for (auto it = musics.begin(); it != musics.end(); musics.erase(it++)) {
            delete it->second->music;
            delete it->second;
        }
    }

    static uint8_t playSound(const std::string& path, bool loop = false)
    {
        bool loaded = soundBuffers.count(path) > 0;

        if (!loaded)
        {
            sf::SoundBuffer* buffer = new sf::SoundBuffer();
            if (!buffer->loadFromFile(path))
            {
                delete buffer;
                return 0;
            }

            soundBuffers.insert({path, buffer});
        }

        sf::Sound* sound = new sf::Sound();
        sound->setBuffer(*soundBuffers[path]);
        sound->setLoop(loop);
        sound->play();

        uint8_t id = soundId++;

        o2d::Sound* s = new o2d::Sound;
        s->id = id;
        s->loop = loop;
        s->sound = sound;

        sounds.insert({id, s});

        return id;
    }

    static bool stopSound(const uint8_t& id)
    {
        if (sounds.count(id) > 0)
        {
            sounds[id]->sound->stop();
            return true;
        }

        return false;
    }

    static bool releaseSound(const uint8_t& id)
    {
        if (sounds.count(id) > 0)
        {
            sounds[id]->sound->play();
            return true;
        }

        return false;
    }

    static uint8_t playMusic(const std::string& path, bool loop = false)
    {
        sf::Music* music = new sf::Music();

        if (!music->openFromFile(path))
        {
            delete music;
            return 0;
        }

        music->setLoop(loop);
        music->play();

        uint8_t id = musicId++;

        o2d::Music* m = new o2d::Music;
        m->id = id;
        m->loop = loop;
        m->music = music;

        musics.insert({id, m});

        return id;
    }

    static bool stopMusic(const uint8_t& id)
    {
        if (musics.count(id) > 0)
        {
            musics[id]->music->stop();
            return true;
        }

        return false;
    }

    static bool releaseMusic(const uint8_t& id)
    {
        if (musics.count(id) > 0)
        {
            musics[id]->music->play();
            return true;
        }

        return false;
    }
};

std::unordered_map<std::string, sf::SoundBuffer*> SoundManager::soundBuffers;
std::unordered_map<uint8_t, o2d::Sound*> SoundManager::sounds;
std::unordered_map<uint8_t, o2d::Music*> SoundManager::musics;

uint8_t SoundManager::soundId = 1;
uint8_t SoundManager::musicId = 1;

#endif //_SoundManagerClass_
