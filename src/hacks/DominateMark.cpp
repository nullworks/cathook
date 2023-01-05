/* Made by BenCat07
 *
 * On 30th of March
 * 2019
 *
 */
#include "common.hpp"
#include "soundcache.hpp"

namespace hacks::tf2::dominatemark
{
static settings::Boolean draw_dominate{ "dominatemark.enable", "false" };
static settings::Float min_size{ "dominatemark.min-size", "15.0f" };
static settings::Float max_size{ "dominatemark.max-size", "40.0f" };

static InitRoutine init([]() {
    EC::Register(
        EC::Draw,
        []() {
            if (!*draw_dominate)
                return;
            if (CE_BAD(LOCAL_E))
                return;
            re::CTFPlayerShared *shared_player = &re::C_BasePlayer::shared_(RAW_ENT(LOCAL_E));
            for (auto const &ent: entity_cache::player_cache)
            {
                if (CE_VALID(ent) && ent->m_bAlivePlayer() && re::CTFPlayerShared::IsDominatingPlayer(shared_player, ent->m_IDX))
                {
                    float size;
                    if (!ent->hitboxes.GetHitbox(0))
                        continue;
                    // Calculate draw pos
                    auto c        = ent->InternalEntity()->GetCollideable();
                    auto draw_pos = ent->m_vecDormantOrigin();
                    if (!draw_pos)
                        continue;
                    draw_pos->z += c->OBBMaxs().z;
                    // Calculate draw size
                    size = *max_size * 1.5f - ent->m_flDistance() / 20.0f;
                    size = fminf(*max_size, size);
                    size = fmaxf(*min_size, size);

                    Vector out;
                    if (draw::WorldToScreen(*draw_pos, out))
                    {
                        static textures::sprite sprite = textures::atlas().create_sprite(448, 257, 64, 64);
                        sprite.draw(int(out.x - size / 2.0f), int(out.y - size), int(size), int(size), colors::white);
                    }
                }
            }
        },
        "dominatemark_draw");
});
} // namespace hacks::tf2::dominatemark
