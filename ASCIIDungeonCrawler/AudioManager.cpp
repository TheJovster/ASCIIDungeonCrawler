#include "AudioManager.h"
#include "AudioManager.h"
#include <stdexcept>

namespace DungeonGame {

    AudioManager& AudioManager::get() {
        static AudioManager instance;
        return instance;
    }

    AudioManager::AudioManager() {
        
        auto load = [&](SFX sfx, const std::string& path) {
            sf::SoundBuffer buf;
            if (buf.loadFromFile(path))
                m_buffers[(int)sfx] = std::move(buf);
            };

        load(SFX::Attack, "assets/sfx_attack.ogg");
        load(SFX::TakeDamagePlayer, "assets/sfx_takedamageplayer.ogg");
        load(SFX::TakeDamage, "assets/sfx_takedamage.ogg");
        load(SFX::OpenChest, "assets/sfx_openchest.ogg");
        load(SFX::Death, "assets/sfx_enemyDeath.ogg");
        load(SFX::CloseChest, "assets/sfx_closechest.ogg");
        load(SFX::BuyItem, "assets/sfx_buyitem.ogg");
        load(SFX::SellItem, "assets/sfx_sellItem.ogg");
        load(SFX::Select, "assets/sfx_select.ogg");
    }

    void AudioManager::playMusic(MusicTrack track) {
        if (track == m_currentTrack) return;
        m_music.stop();
        m_currentTrack = track;

        std::string path;
        switch (track) {
        case MusicTrack::Menu:    path = "assets/MenuMusic.ogg";    break;
        case MusicTrack::Game:    path = "assets/GameplayMusic.ogg";    break;
        case MusicTrack::GameOver:path = "assets/GameOverMusic.ogg"; break;
        case MusicTrack::None:    return;
        }

        if (m_music.openFromFile(path)) {
            m_music.setLoop(track != MusicTrack::GameOver);
            m_music.setVolume(m_musicVolume);
            m_music.play();
        }
    }

    void AudioManager::stopMusic() {
        m_music.stop();
        m_currentTrack = MusicTrack::None;
    }

    void AudioManager::playSFX(SFX sfx) {
        auto it = m_buffers.find((int)sfx);
        if (it == m_buffers.end()) return; // file wasn't loaded, skip silently
        m_sfxPlayer.setBuffer(it->second);
        m_sfxPlayer.setVolume(m_sfxVolume);
        m_sfxPlayer.play();
    }

    void AudioManager::setMusicVolume(float volume) {
        m_musicVolume = volume;
        m_music.setVolume(volume);
    }

    void AudioManager::setSFXVolume(float volume) {
        m_sfxVolume = volume;
    }

}