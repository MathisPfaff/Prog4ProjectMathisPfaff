#pragma once

#if USE_STEAMWORKS
#pragma warning(push)
#pragma warning(disable:4996)
#include <steam_api.h>
#pragma warning(pop)

#include <iostream>

namespace dae
{
    // Achievement API names — must exactly match the Steamworks partner portal.
    namespace AchievementID
    {
        inline constexpr const char* WinOneGame = "ACH_WIN_ONE_GAME";
    }

    class SteamAchievements final
    {
    public:
        static SteamAchievements& GetInstance()
        {
            static SteamAchievements instance;
            return instance;
        }

        SteamAchievements(const SteamAchievements&)            = delete;
        SteamAchievements& operator=(const SteamAchievements&) = delete;

        void Init() { /* singleton constructed on first GetInstance() call */ }

        // Unlocks an achievement by its API name.
        void Unlock(const char* achievementID)
        {
            if (!SteamUserStats())
            {
                std::cerr << "[Steam] SteamUserStats() returned null — is Steam running?\n";
                return;
            }

            const bool setOk    = SteamUserStats()->SetAchievement(achievementID);
            const bool storeOk  = SteamUserStats()->StoreStats();

            std::cout << "[Steam] Unlock(\"" << achievementID << "\")"
                      << " SetAchievement=" << setOk
                      << " StoreStats="     << storeOk << "\n";

            if (!setOk)
                std::cerr << "[Steam] SetAchievement failed — does \"" << achievementID
                          << "\" match the API name in the Steamworks partner portal exactly?\n";
        }

    private:
        SteamAchievements() = default;
    };
}
#endif // USE_STEAMWORKS