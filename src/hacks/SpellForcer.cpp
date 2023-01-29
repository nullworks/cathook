#include "common.hpp"
#include "AntiCheatBypass.hpp"

namespace hacks::tf2::spellforcer
{
static settings::Boolean enabled("spellforce.enabled", "false");

bool isEnabled()
{
    return enabled && g_pGameRules->halloweenScenario != 0;
}

static settings::Int default_spell("spellforce.default_spell", "-1");
static settings::Int default_spell_rare("spellforce.default_spell.rare", "-1");
static settings::Int default_spell_enemies("spellforce.default_spell.enemies", "-1");

static settings::Int doomsday_spell("spellforce.doomsday_spell", "-1");
static settings::Int doomsday_spell_rare("spellforce.doomsday_spell.rare", "-1");
static settings::Int doomsday_spell_enemies("spellforce.doomsday_spell.enemies", "-1");

static settings::Int helltower_spell("spellforce.helltower_spell", "-1");
static settings::Int helltower_spell_enemies("spellforce.helltower_spell.enemies", "-1");

static settings::Int bumper_spell("spellforce.bumper_spell", "-1");
static settings::Int bumper_spell_enemies("spellforce.bumper_spell.enemies", "-1");

enum spelltypes
{
    BUMPER_CARS = 0,
    DOOMSDAY    = 1,
    HELLTOWER   = 2,
    NORMAL      = 3
};

enum halloween_scenario
{
    HALLOWEEN_SCENARIO_NONE = 0,
    HALLOWEEN_SCENARIO_MANN_MANOR,
    HALLOWEEN_SCENARIO_VIADUCT,
    HALLOWEEN_SCENARIO_LAKESIDE,
    HALLOWEEN_SCENARIO_HIGHTOWER,
    HALLOWEEN_SCENARIO_DOOMSDAY
};

spelltypes getSpellMode()
{
    if (HasCondition<TFCond_HalloweenKart>(LOCAL_E))
        return BUMPER_CARS;

    int scenario = g_pGameRules->halloweenScenario;
    switch (scenario)
    {
    case HALLOWEEN_SCENARIO_DOOMSDAY:
        return DOOMSDAY;
    case HALLOWEEN_SCENARIO_HIGHTOWER:
        return HELLTOWER;
    default:
        return NORMAL;
    }
}

// Too much random noise on these maps to force spells
// static std::array<std::string, 1> noisy_maps{ "" };

static boost::unordered_flat_map<int, int> spellmap_normal({ { 0, 4206970 }, { 1, 4206969 }, { 2, 4206972 }, { 3, 4206997 }, { 4, 4206971 }, { 5, 4206977 }, { 7, 4206973 }, { 8, 4206982 }, { 9, 4206976 } });
static boost::unordered_flat_map<int, int> spellmap_normal_rare({ { 7, 4206987 }, { 8, 4206982 }, { 9, 4207006 }, { 10, 4206983 }, { 11, 4206984 } });

static boost::unordered_flat_map<int, int> spellmap_helltower({ { 0, 4206998 }, { 1, 4206984 }, { 2, 4206985 }, { 3, 4206986 }, { 4, 4206987 }, { 5, 4206982 }, { 6, 4206983 } });

static boost::unordered_flat_map<int, int> spellmap_doomsday_normal({ { 0, 4206982 }, { 2, 4207006 }, { 4, 4206983 }, { 5, 4206984 } });
static boost::unordered_flat_map<int, int> spellmap_doomsday_rare({ { 7, 4207009 }, { 8, 4207014 }, { 9, 4207008 }, { 10, 4207007 } });

static boost::unordered_flat_map<int, int> spellmap_bumpercar({ { 12, 4206982 }, { 13, 4206983 }, { 14, 4206987 }, { 15, 4206988 } });
// static boost::unordered_flat_map<int, int> hasslecastle_normal({ { 0, 4206990 }, { 1, 4206989 }, { 2, 4206986 }, { 3, 4206997 }, { 4, 4206984 }, { 5, 4206983 }, { 7, 4207000 }, { 8, 4206982 }, { 9, 4207027 } });

int getCommandForSpellID(int spellidx, int spelltype)
{
    boost::unordered_flat_map<int, int> spellmap;

    // if (levelname.find("koth_slasher") != levelname.npos || levelname.find("pl_bloodwater") != levelname.npos || levelname.find("pl_rumble_event") != levelname.npos)

    auto spellmode = getSpellMode();

    switch (spellmode)
    {
    case HELLTOWER:
        spellmap = spellmap_helltower;
        break;
    case BUMPER_CARS:
        spellmap = spellmap_bumpercar;
        break;
    case DOOMSDAY:
        spellmap = spelltype ? spellmap_doomsday_rare : spellmap_doomsday_normal;
        break;
    default:
        spellmap = spelltype ? spellmap_normal_rare : spellmap_normal;
    }

    int value = spellmap[spellidx];

    // "No entry" should be returned as -1
    return value ? value : -1;
}

// Doomsday spell list

static const int g_doomsdayNormalSpellIndexList[] = {
    0, // Fireball
    0, // Fireball x2
    2, // overheal
    4, // Jump
    5, // Stealth
};

// Side note: 6 is the Teleport spell

static const int g_doomsdayRareSpellIndexList[] = {
    7, // Lightning
    8, // Mini
    9, // Meteor
    7, // Lightning
    8, // Mini
    9, // Meteor
    10 // Boss / Monoculus.  Smaller chance
};

// "Normal" Spell list
static const int g_generalSpellIndexList[] = { 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 7, 8, 9 };

CachedEntity *getClosestSpell()
{
    CachedEntity *ent = nullptr;
    float best_dist   = FLT_MAX;
    bool is_dormant   = false;

    if (CE_BAD(LOCAL_E) || !LOCAL_E->m_bAlivePlayer())
        return ent;
    for (auto &spell : entity_cache::valid_ents)
    {
        if (!spell->m_vecDormantOrigin() || (spell->m_ItemType() != ITEM_SPELL && spell->m_ItemType() != ITEM_SPELL_RARE))
            continue;
        float dist = spell->m_flDistance();
        if (dist < best_dist || (is_dormant && !RAW_ENT(spell)->IsDormant() && dist <= 300.0f))
        {
            ent        = spell;
            best_dist  = dist;
            is_dormant = RAW_ENT(spell)->IsDormant();
        }
    }
    return ent;
}

static void CreateMoveLate()
{
    if (!isEnabled() || !g_pGameRules->isUsingSpells_fn())
        return;
    auto cmd = 0;

    auto spell = getClosestSpell();

    // Apply bad effects for enemies
    if (CE_BAD(spell) || spell->m_flDistance() > 300.0f)
    {
        int spellindex;

        auto spellmode = getSpellMode();
        switch (spellmode)
        {
        case BUMPER_CARS:
            spellindex = *bumper_spell_enemies;
            break;
        case DOOMSDAY:
            spellindex = *doomsday_spell_enemies;
            break;
        case HELLTOWER:
            spellindex = *helltower_spell_enemies;
            break;
        case NORMAL:
            spellindex = *default_spell_enemies;
            break;
        }

        cmd = getCommandForSpellID(spellindex, 0);
    }
    else if (spell->m_ItemType() == ITEM_SPELL)
    {
        int spellindex;

        auto spellmode = getSpellMode();
        switch (spellmode)
        {
        case BUMPER_CARS:
            spellindex = *bumper_spell;
            break;
        case DOOMSDAY:
            spellindex = *doomsday_spell;
            break;
        case HELLTOWER:
            spellindex = *helltower_spell;
            break;
        case NORMAL:
            spellindex = *default_spell;
            break;
        }

        cmd = getCommandForSpellID(spellindex, 0);
    }
    else
    {
        int spellindex;

        auto spellmode = getSpellMode();
        switch (spellmode)
        {
        case DOOMSDAY:
            spellindex = *doomsday_spell_rare;
            break;
        case HELLTOWER:
        case BUMPER_CARS:
        case NORMAL:
            spellindex = *default_spell_rare;
            break;
        }
        cmd = getCommandForSpellID(spellindex, 1);
    }

    if (cmd != -1)
    {
        current_late_user_cmd->command_number = cmd;
        current_late_user_cmd->random_seed    = MD5_PseudoRandom(cmd) & 0x7FFFFFFF;
    }
}

#if ENABLE_VISUALS
static Timer draw_timer;
void Draw()
{
    if (!isEnabled() || !g_IEngine->IsInGame())
    {
        draw_timer.update();
        return;
    }
    static bool previously_highest = true;
    // Is the player the highest IDX of all players?
    bool is_player_highest = true;

    int players_above = 0;

    // Check if there is a valid player exceeding our IDX
    for (int i = g_pLocalPlayer->entity_idx + 1; i <= g_IEngine->GetMaxClients(); ++i)
    {
        if (g_pPlayerResource->isValid(i))
        {
            is_player_highest = false;
            players_above++;
        }
    }
    if (is_player_highest != previously_highest)
        draw_timer.update();

    previously_highest = is_player_highest;

    if (!draw_timer.check(60000))
    {
        if (is_player_highest)
            AddCenterString("Forcing Spells.", colors::green);
        else
            AddCenterString(format("Cannot Force Spells, entity idx is ", players_above, " too low :("), colors::red);
    }
}
#endif

static InitRoutine init(
    []()
    {
#if ENABLE_VISUALS
        EC::Register(EC::Draw, Draw, "spellforcer_draw", EC::early);
        EC::Register(
            EC::LevelInit, []() { draw_timer.update(); }, "spellforcer_lvlinit");
#endif
        EC::Register(EC::CreateMoveLate, CreateMoveLate, "spellforcer_cml", EC::early);
    });

} // namespace hacks::tf2::spellforcer
