#include "common.hpp"
#include "soundcache.hpp"

struct CSndInfo_t
{
    Vector m_pOrigin;
};

struct SoundStruct
{
    CSndInfo_t sound;
    Timer last_update;
};

static std::map<int, SoundStruct> sound_cache;

namespace soundcache
{
constexpr unsigned int EXPIRETIME = 10000;

void cache_sound(const Vector *Origin, int source)
{
    // Just in case
    if (!Origin)
        return;
    sound_cache[source].sound.m_pOrigin = *Origin;
    sound_cache[source].last_update.update();
}

static void CreateMove()
{
    if (CE_BAD(LOCAL_E))
        return;
    CUtlVector<SndInfo_t> sound_list;
    g_ISoundEngine->GetActiveSounds(sound_list);
    for (auto i : sound_list)
        cache_sound(i.m_pOrigin, i.m_nSoundSource);

    for (auto it = sound_cache.cbegin(); it != sound_cache.cend();)
    {
        if (it->second.last_update.check(EXPIRETIME) || (it->first <= g_IEngine->GetMaxClients() && !g_pPlayerResource->isAlive(it->first)))
            it = sound_cache.erase(it);
        else
            ++it;
    }
}

std::optional<Vector> GetSoundLocation(int entid)
{
    auto it = sound_cache.find(entid);
    if (it == sound_cache.end())
        return std::nullopt;
    return it->second.sound.m_pOrigin;
}

static InitRoutine init(
    []()
    {
        EC::Register(EC::CreateMove, CreateMove, "CM_SoundCache");
        EC::Register(
            EC::LevelInit, []() { sound_cache.clear(); }, "soundcache_levelinit");
    });
} // namespace soundcache
