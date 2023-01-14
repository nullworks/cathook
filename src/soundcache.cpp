#include "common.hpp"
#include "soundcache.hpp"



namespace soundcache
{
constexpr unsigned int EXPIRETIME = 10000;


boost::unordered_flat_map<int, SoundStruct> sound_cache;
static void CreateMove()
{
    if (CE_BAD(LOCAL_E))
        return;
    CUtlVector<SndInfo_t> sound_list;
    g_ISoundEngine->GetActiveSounds(sound_list);
    for (const auto &i : sound_list)
        cache_sound(i.m_pOrigin, i.m_nSoundSource);

    for (auto const &[key,val]: sound_cache)
        if (val.last_update.check(EXPIRETIME) || (key <= g_IEngine->GetMaxClients() && !g_pPlayerResource->isAlive(key)))
            sound_cache.erase(key);
}

static InitRoutine init([]() {
    EC::Register(EC::CreateMove, CreateMove, "CM_SoundCache");
    EC::Register(
        EC::LevelInit, []() { sound_cache.clear(); }, "soundcache_levelinit");
});
} // namespace soundcache
