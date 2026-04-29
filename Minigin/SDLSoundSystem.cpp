#include "SDLSoundSystem.h"

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