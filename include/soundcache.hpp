#pragma once
#include "timer.hpp"
#include <map>
#include "public/mathlib/vector.h"
#include <optional>

namespace soundcache
{
    struct CSndInfo_t
{
    Vector m_pOrigin;
};

struct SoundStruct
{
    CSndInfo_t sound;
    Timer last_update;
};

extern boost::unordered_flat_map<int, SoundStruct> sound_cache;
inline void cache_sound(const Vector *Origin, int source)
{
    // Just in case
    if (!Origin)
        return;
    sound_cache[source].sound.m_pOrigin = *Origin;
    sound_cache[source].last_update.update();
}    
inline std::optional<Vector> GetSoundLocation(int entid)
{
    auto it = sound_cache.find(entid);
    if (it == sound_cache.end())
        return std::nullopt;
    return it->second.sound.m_pOrigin;
}

} // namespace soundcache
