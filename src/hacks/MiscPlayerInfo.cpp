#include "common.hpp"
#include "playerlist.hpp"
#include <boost/format.hpp>

namespace hacks::tf2::miscplayerinfo
{
static settings::Boolean draw_kda{ "misc.playerinfo.draw-kda", "false" };
static settings::Boolean mafia_city{ "misc.playerinfo.draw-level", "false" };
struct LevelInfo
{
    int min;
    int max;
    std::string content;
    LevelInfo(int _min, int _max, std::string _content)
    {
        min     = _min;
        max     = _max;
        content = _content;
    }
};
// Source: https://www.youtube.com/watch?v=Yke9BhP1uks
static std::array<LevelInfo, 10> mafia_levels{ LevelInfo(0, 9, "Crook"), LevelInfo(50, 50, "Crook"), LevelInfo(10, 10, "Bad Cop"), LevelInfo(0, 10, "Hoody"), LevelInfo(0, 5, "Gangster"), LevelInfo(1, 1, "Poor Man"), LevelInfo(10, 10, "Rich Man"), LevelInfo(10, 34, "Hitman"), LevelInfo(15, 99, "Boss"), LevelInfo(60, 100, "God Father") };

#if ENABLE_VISUALS
boost::unordered_flat_map<unsigned, std::pair<std::string, int>> choosen_entry{};
boost::unordered_flat_map<unsigned, int> previous_entry_amount{};
std::string random_mafia_entry(int level, unsigned steamid)
{
    std::vector<std::string> store;
    if (choosen_entry[steamid].first != "")
    {
        int entry_amt = 0;
        for (auto &i : mafia_levels)
            if (i.min <= level && i.max >= level)
                entry_amt++;
        if (entry_amt == previous_entry_amount[steamid])
            for (auto &i : mafia_levels)
                if (i.min <= level && i.max >= level && i.content == choosen_entry[steamid].first)
                    return i.content;
        previous_entry_amount[steamid] = entry_amt;
    }
    for (auto &i : mafia_levels)
        if (i.min <= level && i.max >= level)
            store.push_back(i.content);
    if (store.empty())
        return "Crook";
    else
        return store.at(rand() % store.size());
}
static std::array<float, PLAYER_ARRAY_SIZE> death_timer;
void Paint()
{
    if (!*draw_kda && !*mafia_city)
        return;
    for (auto const &ent: entity_cache::player_cache)
    {
        int i = ent->m_IDX;
        if (CE_BAD(ent))
            continue;
        if (ent->m_Type() != ENTITY_PLAYER || !ent->player_info->friendsID)
            continue;
        // Update alive state
        if (g_pPlayerResource->isAlive(i))
            death_timer[i] = g_GlobalVars->curtime;
        // No more processing after 3 Seconds of Death
        if (g_GlobalVars->curtime - death_timer[i] > 3.0f)
            continue;
        auto collidable = RAW_ENT(ent)->GetCollideable();
        if (draw_kda)
        {
            // Choose point over player's Head
            Vector draw_at_kd = ent->m_vecOrigin();
            draw_at_kd.x += (collidable->OBBMaxs().x + collidable->OBBMins().x) / 2;
            draw_at_kd.y += (collidable->OBBMaxs().y + collidable->OBBMins().y) / 2;
            draw_at_kd.z += collidable->OBBMaxs().z + 30.0f;
            Vector out_kd;

            // Base Color
            rgba_t color = colors::white;
            int kills    = g_pPlayerResource->GetKills(i);
            int deaths   = g_pPlayerResource->GetDeaths(i);
            // Calculate KD
            float KDA = deaths ? (float) kills / (float) deaths : kills;
            // Green == 1+ KD
            if (KDA > 1.0f)
                color = colors::green;
            // Orange == KD below 1
            else if (KDA < 1.0f && deaths)
                color = colors::orange;
            // Make color slowly fade
            color.g -= (float) (g_GlobalVars->curtime - death_timer[i]) / (3.0f);
            color.b -= (float) (g_GlobalVars->curtime - death_timer[i]) / (3.0f);
            color.r += (float) (g_GlobalVars->curtime - death_timer[i]) / (3.0f);
            // Don't go out of bounds
            color.g = fmaxf(0.0f, color.g);
            color.b = fmaxf(0.0f, color.b);
            color.r = fminf(1.0f, color.r);
            // If blue/green not empty yet
            if (color.b > 0.0f || color.g > 0.0f)
                if (draw::WorldToScreen(draw_at_kd, out_kd))
                {
                    std::string to_use = format("KD: ", boost::format("%.2f") % KDA, " (", kills, "/", deaths, ")");
                    float w, h;
                    fonts::center_screen->stringSize(to_use, &w, &h);
                    // Center the string
                    draw::String(out_kd.x - w / 2, out_kd.y, color, to_use.c_str(), *fonts::center_screen);
                }
        }
        if (mafia_city)
        {
            // Get Position to draw at
            Vector draw_at_mafia = ent->m_vecOrigin();
            draw_at_mafia.x += (collidable->OBBMaxs().x + collidable->OBBMins().x) / 2;
            draw_at_mafia.y += (collidable->OBBMaxs().y + collidable->OBBMins().y) / 2;
            draw_at_mafia.z += collidable->OBBMaxs().z + 30.0f;
            Vector out_mafia;

            // Base Color
            rgba_t color = colors::white;
            // tint LOCAL_E name slightly
            if (i == g_IEngine->GetLocalPlayer())
                color.b += 0.5f;
            // tint CAT status people's names too
            if (playerlist::AccessData(ent->player_info->friendsID).state == playerlist::k_EState::CAT)
                color.g = 0.8f;

            // Calculate Player Level
            int death_score  = g_pPlayerResource->GetDeaths(i) * 7;
            int kill_score   = g_pPlayerResource->GetKills(i) * 3;
            int damage_score = g_pPlayerResource->GetDamage(i) / 100;
            int level        = min(kill_score + damage_score - death_score, 100);
            level            = max(level, 1);

            // String to draw, {Level} Cat for cathook users, else gotten from std::vector at random.
            if (choosen_entry[ent->player_info->friendsID].first == "" || choosen_entry[ent->player_info->friendsID].second != level)
                choosen_entry[ent->player_info->friendsID] = { random_mafia_entry(level, ent->player_info->friendsID), level };
            std::string to_display = (playerlist::AccessData(ent->player_info->friendsID).state == playerlist::k_EState::CAT ? format("Lv.", level, " Cat") : format("Lv.", level, " ", choosen_entry[ent->player_info->friendsID].first));

            // Clamp to prevent oob
            color.g -= (float) (g_GlobalVars->curtime - death_timer[i]) / (3.0f);
            color.b -= (float) (g_GlobalVars->curtime - death_timer[i]) / (3.0f);
            color.r += (float) (g_GlobalVars->curtime - death_timer[i]) / (3.0f);
            color.g = fmaxf(0.0f, color.g);
            color.b = fmaxf(0.0f, color.b);
            color.r = fminf(1.0f, color.r);

            // Draw as long as blue/green still exist
            if (color.b > 0.0f || color.g > 0.0f)
                if (draw::WorldToScreen(draw_at_mafia, out_mafia))
                {
                    float w, h;
                    fonts::center_screen->stringSize(to_display, &w, &h);
                    // Don't draw ontop of eachother!
                    if (draw_kda)
                        out_mafia.y += h;
                    // Center the string
                    draw::String(out_mafia.x - w / 2, out_mafia.y, color, to_display.c_str(), *fonts::center_screen);
                }
        }
    }
}
static InitRoutine init([]() { EC::Register(EC::Draw, Paint, "DRAW_Miscplayerinfo", EC::average); });
#endif
}; // namespace hacks::tf2::miscplayerinfo
