#pragma once
#include <SFML/Audio.hpp>
#include <unordered_map>

namespace DungeonGame
{
	enum class MusicTrack 
	{
		None,
		Menu,
		Game,
		GameOver
	};

	enum class SFX 
	{
		Attack,
		TakeDamage,
		Death,
		OpenChest,
		CloseChest,
		BuyItem,
		SellItem,
		Select
	};


	class AudioManager
	{
	public:
		static AudioManager& get();

		void playMusic(MusicTrack track);
		void stopMusic();
		void playSFX(SFX sfx);
		void setMusicVolume(float volume);
		void setSFXVolume(float volume);
	
	private:
		AudioManager();

		sf::Music    m_music;
		MusicTrack   m_currentTrack = MusicTrack::None;
		float        m_musicVolume = 100.f;
		float        m_sfxVolume = 100.f;

		std::unordered_map<int, sf::SoundBuffer> m_buffers;
		sf::Sound    m_sfxPlayer;
	};
}


