#include "common.hpp"
#include "soundcache.hpp"



namespace soundcache
{
constexpr unsigned int EXPIRETIME = 10000;


std::map<int, SoundStruct> sound_cache;
static void CreateMove()
{
    if (CE_BAD(LOCAL_E))
        return;
    CUtlVector<SndInfo_t> sound_list;
    g_ISoundEngine->GetActiveSounds(sound_list);
    for (const auto &i : sound_list)
        cache_sound(i.m_pOrigin, i.m_nSoundSource);

    for (auto it = sound_cache.cbegin(); it != sound_cache.cend();)
    {
        if (it->second.last_update.check(EXPIRETIME) || (it->first <= g_IEngine->GetMaxClients() && !g_pPlayerResource->isAlive(it->first)))
            it = sound_cache.erase(it);
        else
            ++it;
    }
}

static InitRoutine init([]() {
    EC::Register(EC::CreateMove, CreateMove, "CM_SoundCache");
    EC::Register(
        EC::LevelInit, []() { sound_cache.clear(); }, "soundcache_levelinit");
});
} // namespace soundcache
