#pragma once
#include <string>
#include <functional>
#include <filesystem>

namespace dae
{
	class Minigin final
	{
		bool doContinue{ true };
		float fixed_time_step{ 0.016f };
		int ms_per_frame{ 16 };
		std::chrono::high_resolution_clock::time_point last_time{ std::chrono::high_resolution_clock::now() };
		float lag{};
	public:
		explicit Minigin(const std::filesystem::path& dataPath);
		~Minigin();
		void Run(const std::function<void()>& load);
		void RunOneFrame();

		Minigin(const Minigin& other) = delete;
		Minigin(Minigin&& other) = delete;
		Minigin& operator=(const Minigin& other) = delete;
		Minigin& operator=(Minigin&& other) = delete;
	};
}