#include "SDLSoundSystem.h"

// ── Platform-specific implementation ─────────────────────────────────────────

#ifndef __EMSCRIPTEN__

#include <SDL3_mixer/SDL_mixer.h>
#include <unordered_map>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iostream>

namespace dae
{
	struct SDLSoundSystem::Impl
	{
		enum class SoundRequestType { PlaySound, PlayMusic, StopMusic, StopAll, Quit };

		struct SoundRequest
		{
			SoundRequestType type{};
			SoundId			 id{};
			float			 volume{ 1.f };
			bool			 loop{ false };
		};

		MIX_Mixer*								m_pMixer{ nullptr };
		std::unordered_map<SoundId, MIX_Audio*>	m_sounds{};
		MIX_Track*								m_pMusicTrack{ nullptr };

		std::queue<SoundRequest>	m_requestQueue{};
		std::mutex					m_queueMutex{};
		std::condition_variable		m_cv{};
		std::thread					m_audioThread{};

		Impl()
		{
			if (!MIX_Init())
			{
				std::cerr << "[SDLSoundSystem] MIX_Init failed: " << SDL_GetError() << "\n";
				return;
			}

			m_pMixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
			if (!m_pMixer)
			{
				std::cerr << "[SDLSoundSystem] MIX_CreateMixerDevice failed: " << SDL_GetError() << "\n";
				return;
			}

			m_pMusicTrack = MIX_CreateTrack(m_pMixer);
			if (!m_pMusicTrack)
				std::cerr << "[SDLSoundSystem] MIX_CreateTrack failed: " << SDL_GetError() << "\n";

			m_audioThread = std::thread(&Impl::ProcessQueue, this);
		}

		~Impl()
		{
			{
				std::scoped_lock lock{ m_queueMutex };
				m_requestQueue.push({ SoundRequestType::Quit });
			}
			m_cv.notify_one();

			if (m_audioThread.joinable())
				m_audioThread.join();

			for (auto& [id, audio] : m_sounds)
				MIX_DestroyAudio(audio);

			MIX_Quit();
		}

		void LoadAudio(SoundId id, const std::string& filePath)
		{
			MIX_Audio* audio = MIX_LoadAudio(m_pMixer, filePath.c_str(), false);
			if (!audio)
			{
				std::cerr << "[SDLSoundSystem] Failed to load audio: " << filePath << " - " << SDL_GetError() << "\n";
				return;
			}
			m_sounds[id] = audio;
		}

		void PlaySound(SoundId id, float volume)       { Enqueue({ SoundRequestType::PlaySound, id, volume, false }); }
		void PlayMusic(SoundId id, float volume, bool loop) { Enqueue({ SoundRequestType::PlayMusic, id, volume, loop }); }
		void StopMusic()                               { Enqueue({ SoundRequestType::StopMusic }); }
		void StopAll()                                 { Enqueue({ SoundRequestType::StopAll   }); }

	private:
		void Enqueue(SoundRequest request)
		{
			{
				std::scoped_lock lock{ m_queueMutex };
				m_requestQueue.push(request);
			}
			m_cv.notify_one();
		}

		void ProcessQueue()
		{
			while (true)
			{
				std::unique_lock lock{ m_queueMutex };
				m_cv.wait(lock, [this] { return !m_requestQueue.empty(); });

				SoundRequest req = m_requestQueue.front();
				m_requestQueue.pop();
				lock.unlock();

				switch (req.type)
				{
				case SoundRequestType::PlaySound:
				{
					auto it = m_sounds.find(req.id);
					if (it == m_sounds.end()) { std::cerr << "[SDLSoundSystem] Sound id " << req.id << " not loaded.\n"; break; }
					MIX_Track* track = MIX_CreateTrack(m_pMixer);
					if (track)
					{
						MIX_SetTrackAudio(track, it->second);
						MIX_SetTrackGain(track, req.volume);
						MIX_PlayTrack(track, 0);
					}
					break;
				}
				case SoundRequestType::PlayMusic:
				{
					auto it = m_sounds.find(req.id);
					if (it == m_sounds.end()) { std::cerr << "[SDLSoundSystem] Music id " << req.id << " not loaded.\n"; break; }
					MIX_SetTrackAudio(m_pMusicTrack, it->second);
					MIX_SetTrackGain(m_pMusicTrack, req.volume);
					MIX_PlayTrack(m_pMusicTrack, req.loop ? -1 : 0);
					break;
				}
				case SoundRequestType::StopMusic:
				case SoundRequestType::StopAll:
					if (m_pMusicTrack) MIX_StopTrack(m_pMusicTrack, 0);
					break;

				case SoundRequestType::Quit:
					return;
				}
			}
		}
	};
}

#elif defined(__EMSCRIPTEN__)

#include <SDL_mixer.h>
#include <unordered_map>
#include <iostream>

namespace dae
{
	struct SDLSoundSystem::Impl
	{
		std::unordered_map<SoundId, Mix_Chunk*> m_chunks{};
		std::unordered_map<SoundId, Mix_Music*> m_music{};

		Impl()
		{
			if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
				std::cerr << "[SDLSoundSystem] Mix_OpenAudio failed: " << Mix_GetError() << "\n";
		}

		~Impl()
		{
			for (auto& [id, chunk] : m_chunks) Mix_FreeChunk(chunk);
			for (auto& [id, music] : m_music)  Mix_FreeMusic(music);
			Mix_CloseAudio();
		}

		// Sound (SFX) — loaded as Mix_Chunk
		void LoadAudio(SoundId id, const std::string& filePath)
		{
			Mix_Chunk* chunk = Mix_LoadWAV(filePath.c_str());
			if (chunk) { m_chunks[id] = chunk; return; }

			// Fallback: try as music (e.g. mp3)
			Mix_Music* music = Mix_LoadMUS(filePath.c_str());
			if (music) { m_music[id] = music; return; }

			std::cerr << "[SDLSoundSystem] Failed to load audio: " << filePath << " - " << Mix_GetError() << "\n";
		}

		void PlaySound(SoundId id, float volume)
		{
			auto it = m_chunks.find(id);
			if (it == m_chunks.end()) { std::cerr << "[SDLSoundSystem] Sound id " << id << " not loaded.\n"; return; }
			Mix_VolumeChunk(it->second, static_cast<int>(volume * MIX_MAX_VOLUME));
			Mix_PlayChannel(-1, it->second, 0);
		}

		void PlayMusic(SoundId id, float volume, bool loop)
		{
			auto it = m_music.find(id);
			if (it == m_music.end()) { std::cerr << "[SDLSoundSystem] Music id " << id << " not loaded.\n"; return; }
			Mix_VolumeMusic(static_cast<int>(volume * MIX_MAX_VOLUME));
			Mix_PlayMusic(it->second, loop ? -1 : 1);
		}

		void StopMusic() { Mix_HaltMusic(); }
		void StopAll()   { Mix_HaltChannel(-1); Mix_HaltMusic(); }
	};
}

#endif

// ── Platform-independent forwarding ──────────────────────────────────────────

namespace dae
{
	SDLSoundSystem::SDLSoundSystem()  : m_pImpl{ std::make_unique<Impl>() } {}
	SDLSoundSystem::~SDLSoundSystem() = default;

	void SDLSoundSystem::LoadSound(SoundId id, const std::string& filePath) { m_pImpl->LoadAudio(id, filePath); }
	void SDLSoundSystem::LoadMusic(SoundId id, const std::string& filePath) { m_pImpl->LoadAudio(id, filePath); }
	void SDLSoundSystem::PlaySound(SoundId id, float volume)                { m_pImpl->PlaySound(id, volume); }
	void SDLSoundSystem::PlayMusic(SoundId id, float volume, bool loop)     { m_pImpl->PlayMusic(id, volume, loop); }
	void SDLSoundSystem::StopMusic()                                        { m_pImpl->StopMusic(); }
	void SDLSoundSystem::StopAllSounds()                                    { m_pImpl->StopAll(); }
}