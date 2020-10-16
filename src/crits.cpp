#include "common.hpp"
#include "crits.hpp"
#include "Backtrack.hpp"
#include "WeaponData.hpp"
#include "netadr.h"

std::unordered_map<int, int> command_number_mod{};

namespace criticals
{
settings::Boolean enabled{ "crit.enabled", "false" };
settings::Boolean melee{ "crit.melee", "false" };
static settings::Button crit_key{ "crit.key", "<null>" };
static settings::Boolean force_no_crit{ "crit.anti-crit", "true" };

static settings::Boolean draw{ "crit.info", "false" };
static settings::Boolean draw_meter{ "crit.draw-meter", "false" };
// Draw control
static settings::Int draw_string_x{ "crit.draw-info.x", "8" };
static settings::Int draw_string_y{ "crit.draw-info.y", "800" };
static settings::Int size{ "crit.bar-size", "100" };
static settings::Int bar_x{ "crit.bar-x", "50" };
static settings::Int bar_y{ "crit.bar-y", "500" };

// Debug rvar
static settings::Boolean debug_desync{ "crit.desync-debug", "false" };

// How much is added to bucket per shot?
static float added_per_shot = 0.0f;
// How much is taken in the case of a fastfire weapon?
static float taken_per_crit = 0.0f;
// Needed to calculate observed crit chance properly
static int cached_damage   = 0;
static int crit_damage     = 0;
static int melee_damage    = 0;
static int round_damage    = 0;
static bool is_out_of_sync = false;
// Optimization
static int shots_to_fill_bucket = 0;

static float getBucketCap()
{
    static ConVar *tf_weapon_criticals_bucket_cap = g_ICvar->FindVar("tf_weapon_criticals_bucket_cap");
    return tf_weapon_criticals_bucket_cap->GetFloat();
}

static float getWithdrawMult(IClientEntity *wep)
{
    weapon_info info(wep);
    // Increment call count
    int call_count = info.crit_count + 1;
    // How many times there was a check for crits
    int crit_checks  = info.crit_attempts + 1;
    float flMultiply = 0.5;

    if (g_pLocalPlayer->weapon_mode != weapon_melee)
        flMultiply = RemapValClamped(((float) call_count / (float) crit_checks), 0.1f, 1.f, 1.f, 3.f);

    float flToRemove = flMultiply * 3.0f;
    return flToRemove;
}

// How much is taken per critical shot
float getWithdrawAmount(IClientEntity *wep)
{
    float amount = added_per_shot * getWithdrawMult(wep);

    if (isRapidFire(wep))
    {
        // Hey Mantissa, could you like, COOPERATE?
        // For real though, on minigun the last bit of mantissa is falsely 1 sometimes. So we remove that last bit
        amount = taken_per_crit * getWithdrawMult(wep);
        reinterpret_cast<int &>(amount) &= ~1;
    }
    return amount;
}

// This simply checks if we can withdraw the specified damage from the bucket or not.
// add_damage parameter simply specifies if we also kind of simulate the damage that gets added before
// the function gets ran in the game.
static bool isAllowedToWithdrawFromBucket(IClientEntity *wep, bool add_damage = true)
{
    weapon_info info(wep);
    if (add_damage)
    {
        // Regulate crit frequency to reduce client-side seed hacking
        if (info.crit_bucket < getBucketCap())
        {
            // Treat raw damage as the resource by which we add or subtract from the bucket
            info.crit_bucket += added_per_shot;
            info.crit_bucket = std::min(info.crit_bucket, getBucketCap());
        }
    }
    // Can't remove
    if (getWithdrawAmount(wep) > info.crit_bucket)
        return false;

    return true;
}

// This simulates a shot in all the important aspects, like increating crit attempts, bucket, etc
static void simulateNormalShot(IClientEntity *wep, float flDamage)
{
    weapon_info info(wep);
    // Regulate crit frequency to reduce client-side seed hacking
    if (info.crit_bucket < getBucketCap())
    {
        // Treat raw damage as the resource by which we add or subtract from the bucket
        info.crit_bucket += flDamage;
        info.crit_bucket = std::min(info.crit_bucket, getBucketCap());
    }
    // Write other values important for iteration
    info.crit_attempts++;
    info.restore_data(wep);
}

// Calculate shots until crit
static int shotsUntilCrit(IClientEntity *wep)
{
    weapon_info info(wep);
    // How many shots until we can crit
    int shots = 0;
    // Use shots til bucket is full for reference
    for (shots = 0; shots < shots_to_fill_bucket + 1; shots++)
    {
        if (isAllowedToWithdrawFromBucket(wep, true))
            break;
        // Do calculations
        simulateNormalShot(wep, added_per_shot);
    }
    // Restore variables
    info.restore_data(wep);
    return shots;
}

// Calculate a weapon and player specific variable that determines how
// High your observed crit chance is allowed to be
// (this + 0.1f >= observed_chance)
static float getCritCap(IClientEntity *wep)
{
    // Player specific Multiplier
    float crit_mult = re::CTFPlayerShared::GetCritMult(re::CTFPlayerShared::GetPlayerShared(RAW_ENT(LOCAL_E)));

    // Weapon specific Multiplier
    float chance = 0.02f;
    if (g_pLocalPlayer->weapon_mode == weapon_melee)
        chance = 0.15f;
    float flMultCritChance = ATTRIB_HOOK_FLOAT(crit_mult * chance, "mult_crit_chance", wep, 0, 1);

    if (isRapidFire(wep))
    {
        // get the total crit chance (ratio of total shots fired we want to be crits)
        float flTotalCritChance = clamp(0.02f * crit_mult, 0.01f, 0.99f);
        // get the fixed amount of time that we start firing crit shots for
        float flCritDuration = 2.0f;
        // calculate the amount of time, on average, that we want to NOT fire crit shots for in order to achieve the total crit chance we want
        float flNonCritDuration = (flCritDuration / flTotalCritChance) - flCritDuration;
        // calculate the chance per second of non-crit fire that we should transition into critting such that on average we achieve the total crit chance we want
        float flStartCritChance = 1 / flNonCritDuration;
        flMultCritChance        = ATTRIB_HOOK_FLOAT(flStartCritChance, "mult_crit_chance", wep, 0, 1);
    }

    return flMultCritChance;
}

// Server gives us garbage so let's just calc our own
static float getObservedCritChance()
{
    if (!(cached_damage - round_damage))
        return 0.0f;
    // Same is used by server
    float normalized_damage = (float) crit_damage / 3.0f;
    return normalized_damage / (normalized_damage + (float) ((cached_damage - round_damage) - crit_damage));
}

// This returns two floats, first one being our current crit chance, second is what we need to be at/be lower than
static std::pair<float, float> critMultInfo(IClientEntity *wep)
{
    float cur_crit        = getCritCap(wep);
    float observed_chance = CE_FLOAT(LOCAL_W, netvar.flObservedCritChance);
    float needed_chance   = cur_crit + 0.1f;
    return std::pair<float, float>(observed_chance, needed_chance);
}

// How much damage we need until we can crit
static int damageUntilToCrit(IClientEntity *wep)
{
    // First check if we even need to deal damage at all
    auto crit_info = critMultInfo(wep);
    if (crit_info.first <= crit_info.second || g_pLocalPlayer->weapon_mode == weapon_melee)
        return 0;

    float target_chance = critMultInfo(wep).second;
    // Formula taken from TotallyNotElite
    int damage = std::ceil(crit_damage * (2.0f * target_chance + 1.0f) / (3.0f * target_chance));
    return damage - (cached_damage - round_damage);
}

// Calculate next tick we can crit at
static int nextCritTick(int loops = 4096)
{
    // Previous crit tick stuff
    static int previous_crit   = -1;
    static int previous_weapon = -1;

    auto wep = RAW_ENT(LOCAL_W);

    // Already have a tick, use it
    if (previous_weapon == wep->entindex() && previous_crit >= current_late_user_cmd->command_number)
        return previous_crit;

    int old_seed = MD5_PseudoRandom(current_late_user_cmd->command_number) & 0x7FFFFFFF;
    // Try 4096 times
    for (int i = 0; i < loops; i++)
    {
        int cmd_number = current_late_user_cmd->command_number + i;
        // Set random seed
        *g_PredictionRandomSeed = MD5_PseudoRandom(cmd_number) & 0x7FFFFFFF;
        // Save weapon state to not break anything
        weapon_info info(wep);
        bool is_crit = re::C_TFWeaponBase::CalcIsAttackCritical(wep);
        // Restore state
        info.restore_data(wep);
        // Is a crit
        if (is_crit)
        {
            *g_PredictionRandomSeed = old_seed;
            previous_crit           = cmd_number;
            previous_weapon         = wep->entindex();
            return cmd_number;
        }
    }
    // Reset
    *g_PredictionRandomSeed = old_seed;
    return -1;
}

static bool randomCritEnabled()
{
    static ConVar *tf_weapon_criticals = g_ICvar->FindVar("tf_weapon_criticals");
    return tf_weapon_criticals->GetBool();
}

// These are used when we want to force a crit regardless of states (e.g. for delayed weapons like sticky launchers)
bool force_crit_this_tick = false;

// Is the hack enabled?
bool isEnabled()
{
    // No crits without random crits
    if (!randomCritEnabled())
        return false;
    // Melee overrides the enabled switch
    if (melee || enabled)
        return true;
    // If none of these conditions pass, crithack is NOT on
    return false;
}

bool shouldMeleeCrit()
{
    if (!melee || g_pLocalPlayer->weapon_mode != weapon_melee)
        return false;
    if (hacks::tf2::backtrack::isBacktrackEnabled)
    {
        // Closest tick for melee (default filter carry)
        auto closest_tick = hacks::tf2::backtrack::getClosestTick(LOCAL_E->m_vecOrigin(), hacks::tf2::backtrack::defaultEntFilter, hacks::tf2::backtrack::defaultTickFilter);
        // Valid backtrack target
        if (closest_tick)
        {
            // Out of range, don't crit
            if ((*closest_tick).second.m_vecOrigin.DistTo(LOCAL_E->m_vecOrigin()) >= re::C_TFWeaponBaseMelee::GetSwingRange(RAW_ENT(LOCAL_W)) + 150.0f)
                return false;
        }
        else
            return false;
    }
    // Normal check, get closest entity and check distance
    else
    {
        auto ent = getClosestEntity(LOCAL_E->m_vecOrigin());
        if (!ent || ent->m_flDistance() >= re::C_TFWeaponBaseMelee::GetSwingRange(RAW_ENT(LOCAL_W)) + 150.0f)
            return false;
    }
    return true;
}

// Check some basic conditions
bool shouldCrit()
{
    // Melee mode with melee out and in range?
    if (shouldMeleeCrit())
        return true;
    // Crit key + enabled, for melee, the crit key MUST be set
    if (enabled && ((g_pLocalPlayer->weapon_mode != weapon_melee && !crit_key) || crit_key.isKeyDown()))
        return true;
    // Force crits on sticky launcher
    /*if (force_ticks)
    {
        if (LOCAL_W->m_iClassID() == CL_CLASS(CTFPipebombLauncher))
            return true;
        else
            force_ticks = 0;
    }*/
    // Tick is supposed to be forced because of something external to crithack
    if (force_crit_this_tick)
        return true;
    return false;
}

// BeggarsCanWeaponCrit
static bool can_beggars_crit   = false;
static bool attacked_last_tick = false;

bool canWeaponCrit(bool draw = false)
{
    // Is weapon elligible for crits?
    IClientEntity *weapon = RAW_ENT(LOCAL_W);
    if (!re::C_TFWeaponBase::IsBaseCombatWeapon(weapon))
        return false;
    if (!re::C_TFWeaponBase::AreRandomCritsEnabled(weapon) || !added_per_shot)
        return false;
    if (!re::C_TFWeaponBase::CanFireCriticalShot(weapon, false, nullptr))
        return false;
    if (!added_per_shot)
        return false;
    if (!getCritCap(weapon))
        return false;

    // Misc checks
    if (!isAllowedToWithdrawFromBucket(weapon))
        return false;
    if (!draw && !CanShoot() && !isRapidFire(weapon))
        return false;
    if (!draw && CE_INT(LOCAL_W, netvar.iItemDefinitionIndex) == 730 && !can_beggars_crit)
        return false;
    // Check if we have done enough damage to crit
    auto crit_mult_info = critMultInfo(weapon);
    if (crit_mult_info.first > crit_mult_info.second && g_pLocalPlayer->weapon_mode != weapon_melee)
        return false;
    return true;
}

// We cycle between the crit cmds so we want to store where we are currently at
int current_index = 0;
// Cache Weapons
std::map<int, std::vector<int>> crit_cmds;

// We need to store a bunch of data for when we kill someone with a crit
struct player_status
{
    int health{};
    int clazz{};
    // To make Player resource not be a hindrance
    bool just_updated{};
};
static std::array<player_status, 32> player_status_list{};

// Function for preventing a crit
bool prevent_crit()
{
    int tries = 0;
    // Only check one tick
    while (nextCritTick(5) == current_late_user_cmd->command_number && tries < 5)
    {
        current_late_user_cmd->command_number++;
        current_late_user_cmd->random_seed = MD5_PseudoRandom(current_late_user_cmd->command_number);
        tries++;
    }
    // Failed
    if (nextCritTick(5) == current_late_user_cmd->command_number)
        return false;
    // Suceeded
    return true;
}

// Main function that forces a crit
void force_crit()
{
    // New mode stuff (well when not using melee nor using pipe launcher)
    if (g_pLocalPlayer->weapon_mode != weapon_melee && LOCAL_W->m_iClassID() != CL_CLASS(CTFPipebombLauncher))
    {
        // We have valid crit command numbers
        if (crit_cmds.size() && crit_cmds.find(LOCAL_W->m_IDX) != crit_cmds.end() && crit_cmds.find(LOCAL_W->m_IDX)->second.size())
        {
            if (current_index >= crit_cmds.at(LOCAL_W->m_IDX).size())
                current_index = 0;

            // Magic crit cmds get used to force a crit
            current_late_user_cmd->command_number = crit_cmds.at(LOCAL_W->m_IDX).at(current_index);
            current_late_user_cmd->random_seed    = MD5_PseudoRandom(current_late_user_cmd->command_number) & 0x7FFFFFFF;
            current_index++;
        }
    }
    // We can just force to nearest crit for melee, and sticky launchers apparently
    else
    {
        int next_crit = nextCritTick();
        if (next_crit != -1)
        {
            if (LOCAL_W->m_iClassID() == CL_CLASS(CTFPipebombLauncher))
            {
                /*if (!force_ticks && isEnabled())
                    force_ticks = 3;
                if (force_ticks)
                    force_ticks--;*/
                /*// Prevent crits
                prevent_ticks = 3;
                if (prevent_ticks)
                {
                    prevent_crit();
                    prevent_ticks--;
                    return;
                }*/
            }
            current_late_user_cmd->command_number = next_crit;
            current_late_user_cmd->random_seed    = MD5_PseudoRandom(next_crit) & 0x7FFFFFFF;
        }
    }
    force_crit_this_tick = false;
}

// Update the magic crit commands numbers
// Basically, we can send any command number to the server, and as long as its in the future it will get
// used for crit calculations.
// Since we know how the random seed is made, and how the crits are predicted,
// We simply replicate the same behaviour and find out, "Which command numbers would be the best?"
// The answer is, ones that make RandomInt return 0, or something very low, as that will have the highest
// Chance of working.

static void updateCmds()
{
    auto weapon                = RAW_ENT(LOCAL_W);
    static int previous_weapon = 0;

    // Current command number
    int cur_cmdnum = current_late_user_cmd->command_number;

    // Are the cmds too old?
    bool old_cmds = false;

    // Try to find current weapon
    auto weapon_cmds = crit_cmds.find(weapon->entindex());

    // Nothing indexed, mark as outdated
    if (weapon_cmds == crit_cmds.end())
        old_cmds = true;
    // Else check if outdated
    else
    {
        for (auto &cmd : weapon_cmds->second)
            if (cmd < cur_cmdnum)
                old_cmds = true;
    }

    // Are the commands too old?
    if (old_cmds)
    {
        // Used later for the seed
        int xor_dat = (weapon->entindex() << 8 | LOCAL_E->m_IDX);
        if (g_pLocalPlayer->weapon_mode == weapon_melee)
            xor_dat = (weapon->entindex() << 16 | LOCAL_E->m_IDX << 8);

        // Clear old data
        crit_cmds[weapon->entindex()].clear();
        added_per_shot = 0.0f;

        // 100000 should be fine performance wise, as they are very spread out
        // j indicates the amount to store at max
        for (int i = cur_cmdnum + 200, j = 3; i <= cur_cmdnum + 100000 + 200 && j > 0; i++)
        {
            // Manually make seed
            int iSeed = MD5_PseudoRandom(i) & 0x7fffffff;
            // Replicate XOR behaviour used by game
            int tempSeed = iSeed ^ (xor_dat);
            RandomSeed(tempSeed);

            // The result of the crithack, 0 == best chance
            int iResult = RandomInt(0, 9999);

            // If it's 0, store it. Also do not take too close cmds as they will
            // have to be replaced very soon anyways.
            if (iResult == 0 && i > cur_cmdnum + 200)
            {
                // Add to magic crit array
                crit_cmds[weapon->entindex()].push_back(i);
                // We found a cmd, store it
                j--;
            }
        }
    }

    // We haven't calculated the amount added to the bucket yet
    if (added_per_shot == 0.0f || previous_weapon != weapon->entindex())
    {
        weapon_info info(weapon);
        int nProjectilesPerShot = GetWeaponData(weapon)->m_nBulletsPerShot;
        if (nProjectilesPerShot >= 1)
            nProjectilesPerShot = ATTRIB_HOOK_FLOAT(nProjectilesPerShot, "mult_bullets_per_shot", weapon, 0x0, true);
        else
            nProjectilesPerShot = 1;

        added_per_shot = GetWeaponData(weapon)->m_nDamage;
        added_per_shot = ATTRIB_HOOK_FLOAT(added_per_shot, "mult_dmg", weapon, 0x0, true);
        added_per_shot *= nProjectilesPerShot;
        shots_to_fill_bucket = getBucketCap() / added_per_shot;
        // Special boi
        if (isRapidFire(weapon))
        {
            taken_per_crit = added_per_shot;
            taken_per_crit *= 2.0f / GetWeaponData(weapon)->m_flTimeFireDelay;

            // Yes this looks dumb but i want to ensure that it matches with valve code
            int bucket_cap_recasted = (int) getBucketCap();
            // Never try to drain more than cap
            if (taken_per_crit * 3.0f > bucket_cap_recasted)
                taken_per_crit = (float) bucket_cap_recasted / 3.0f;
        }
    }

    previous_weapon = weapon->entindex();
}

// Fix observed crit chance
static void fixObservedCritchance(IClientEntity *weapon)
{
    weapon_info info(weapon);
    info.observed_crit_chance = getObservedCritChance();
    info.restore_data(weapon);
}

static std::vector<float> crit_mult_storage;
static float last_bucket_fix = -1;
// Fix bucket on non-local servers
void fixBucket(IClientEntity *weapon, CUserCmd *cmd)
{
    INetChannel *ch = (INetChannel *) g_IEngine->GetNetChannelInfo();
    if (!ch)
        return;

    static int last_weapon;
    // This tracks only when bucket is updated
    static int last_update_command;

    fixObservedCritchance(weapon);
    weapon_info info(weapon);

    float bucket = info.crit_bucket;

    // Changed bucket more than once this tick, or fastfire weapon. Note that we check if it is within 20 tick range just in case.
    if (weapon->entindex() == last_weapon && bucket != last_bucket_fix && last_update_command == cmd->command_number)
        bucket = last_bucket_fix;

    last_weapon = weapon->entindex();
    // Bucket changed, update
    if (last_bucket_fix != bucket)
        last_update_command = cmd->command_number;

    last_bucket_fix  = bucket;
    info.crit_bucket = bucket;
    info.restore_data(weapon);
}

// Is the player currently performing the double engi melee attack exploit?
bool isExploitingDoubleAttack()
{
    // Exploit doesn't work if you don't press IN_ATTACK2 + IN_ATTACK, aren't using a melee weapon or aren't playing engineer
    if (!(current_user_cmd->buttons & IN_ATTACK2) || g_pLocalPlayer->weapon_mode != weapon_melee || g_pLocalPlayer->clazz != tf_engineer)
        return false;

    // Get the entity that we are looking at and check if it is a building that we built
    int eindex;
    WhatIAmLookingAt(&eindex, nullptr);
    if (eindex == -1)
        return false;
    auto entity = ENTITY(eindex);
    if (CE_GOOD(entity) && entity->m_Type() == ENTITY_BUILDING && HandleToIDX(CE_INT(entity, netvar.m_hBuilder)) == LOCAL_E->m_IDX)
    {
        return true;
    }
    else
        return false;
}

// Damage this round
void CreateMove()
{
    // It should never go down, if it does we need to compensate for it
    if (g_pPlayerResource->GetDamage(g_pLocalPlayer->entity_idx) < round_damage)
        round_damage = g_pPlayerResource->GetDamage(g_pLocalPlayer->entity_idx);
    // Base on melee damage and server networked one rather than anything else
    cached_damage = g_pPlayerResource->GetDamage(g_pLocalPlayer->entity_idx) - melee_damage;

    // We need to update player states regardless, else we can't sync the observed crit chance
    for (int i = 1; i <= g_IEngine->GetMaxClients(); i++)
    {
        CachedEntity *ent = ENTITY(i);
        // no valid check needed, GetHealth only uses m_IDX
        if (g_pPlayerResource->GetHealth(ent))
        {
            auto &status = player_status_list[i - 1];
            // Only sync if not updated recently in player_hurt
            // new health is bigger,
            // or they changed classes. We do the rest in player_hurt
            if (!status.just_updated && (status.clazz != g_pPlayerResource->GetClass(ent) || status.health < g_pPlayerResource->GetHealth(ent)))
            {
                status.clazz  = g_pPlayerResource->GetClass(ent);
                status.health = g_pPlayerResource->GetHealth(ent);
            }
            status.just_updated = false;
        }
    }

    // Basic checks
    if (!isEnabled())
        return;
    // This is not a tick that will actually matter
    if (!current_late_user_cmd->command_number)
        return;
    if (CE_BAD(LOCAL_E) || CE_BAD(LOCAL_W))
        return;

    // Update magic crit commands
    updateCmds();

    // Beggars check
    if (CE_INT(LOCAL_W, netvar.iItemDefinitionIndex) == 730)
    {
        // Check if we released the barrage by releasing m1, also lock bool so people don't just release m1 and tap it again
        if (!can_beggars_crit)
            can_beggars_crit = !(current_late_user_cmd->buttons & IN_ATTACK) && attacked_last_tick;
        // Update
        attacked_last_tick = current_late_user_cmd->buttons & IN_ATTACK;
        if (!CE_INT(LOCAL_W, netvar.m_iClip1) && CE_INT(LOCAL_W, netvar.iReloadMode) == 0)
        {
            // Reset
            can_beggars_crit = false;
        }
    }
    else
        can_beggars_crit = false;

    // No point in forcing/preventing crits if we can't even crit
    if (!canWeaponCrit())
        return;

    // Not in attack? Do nothing, unless using the beggars/Sticky launcher
    if (!(current_late_user_cmd->buttons & IN_ATTACK))
    {
        if (LOCAL_W->m_iClassID() == CL_CLASS(CTFPipebombLauncher))
        {
            float chargebegin = *((float *) ((uintptr_t) RAW_ENT(LOCAL_W) + 3152));
            float chargetime  = g_GlobalVars->curtime - chargebegin;

            static bool currently_charging_pipe = false;

            // Sticky started charging
            if (chargetime < 6.0f && chargetime)
                currently_charging_pipe = true;

            // Sticky was released
            if (!(current_user_cmd->buttons & IN_ATTACK) && currently_charging_pipe)
            {
                currently_charging_pipe = false;
            }
            else
                return;
        }
        else if (!can_beggars_crit)
            return;
    }

    // Is the player currently performing the double engi melee attack exploit? If yes, force a crit this tick
    if (isExploitingDoubleAttack())
        force_crit_this_tick = true;

    // Should we even try to crit this tick?
    if (shouldCrit())
    {
        force_crit();
    }
    // Ok, we shouldn't crit for whatever reason, lets prevent crits
    else if (force_no_crit)
    {
        // if (prevent_ticks >= 1)
        //    prevent_ticks--;
        prevent_crit();
        // if (LOCAL_W->m_iClassID() == CL_CLASS(CTFPipebombLauncher) && current_late_user_cmd->buttons & IN_ATTACK)
        //    prevent_ticks = 3;
    }
    force_crit_this_tick = false;
}

// Storage
static int last_crit_tick   = -1;
static int last_bucket      = 0;
static int shots_until_crit = 0;
static int last_wep         = 0;

#if ENABLE_VISUALS

// Need our own Text drawing
static std::array<std::string, 32> crit_strings;
static size_t crit_strings_count{ 0 };
static std::array<rgba_t, 32> crit_strings_colors{ colors::empty };

static std::string bar_string = "";

void AddCritString(const std::string &string, const rgba_t &color)
{
    crit_strings[crit_strings_count]        = string;
    crit_strings_colors[crit_strings_count] = color;
    ++crit_strings_count;
}

void DrawCritStrings()
{
    // Positions, base on crit meter itself and draw Centered
    float x = *bar_x + *size;
    float y = *bar_y + *size / 5.0f;

    if (bar_string != "")
    {
        float sx, sy;
        fonts::center_screen->stringSize(bar_string, &sx, &sy);
        // Center and draw below
        draw::String(x - sx / 2, (y + sy), colors::red, bar_string.c_str(), *fonts::center_screen);
        y += fonts::center_screen->size + 1;
    }

    x = *draw_string_x;
    y = *draw_string_y;
    for (size_t i = 0; i < crit_strings_count; ++i)
    {
        float sx, sy;
        fonts::menu->stringSize(crit_strings[i], &sx, &sy);
        draw::String(x, y, crit_strings_colors[i], crit_strings[i].c_str(), *fonts::center_screen);
        y += fonts::center_screen->size + 1;
    }
    crit_strings_count = 0;
    bar_string         = "";
}

static Timer update_shots{};

void Draw()
{
    if (!isEnabled())
        return;
    if (!draw && !draw_meter)
        return;
    if (!g_IEngine->GetNetChannelInfo())
        last_crit_tick = -1;
    if (CE_GOOD(LOCAL_E) && CE_GOOD(LOCAL_W))
    {
        auto wep     = RAW_ENT(LOCAL_W);
        float bucket = weapon_info(wep).crit_bucket;

        // Fix observed crit chance
        // fixObservedCritchance(wep);

        // Used by multiple things
        bool can_crit = canWeaponCrit(true);

        if (bucket != last_bucket || wep->entindex() != last_wep || update_shots.test_and_set(500))
        {
            // Recalculate shots until crit
            if (!can_crit)
                shots_until_crit = shotsUntilCrit(wep);
        }

        // Reset because too old
        if (!shots_until_crit && added_per_shot && (wep->entindex() != last_wep || last_crit_tick - current_late_user_cmd->command_number < 0 || (last_crit_tick - current_late_user_cmd->command_number) * g_GlobalVars->interval_per_tick > 30))
            last_crit_tick = nextCritTick();

        // Get Crit multiplier info
        std::pair<float, float> crit_mult_info = critMultInfo(wep);

        // Draw Text
        if (draw)
        {
            // Display for when crithack is active
            if (shouldCrit())
            {
                if (can_crit)
                    AddCritString("Forcing Crits!", colors::red);
                else
                    AddCritString("Weapon can currently not crit!", colors::red);
            }

            // Weapon can't randomly crit
            if (!re::C_TFWeaponBase::CanFireCriticalShot(RAW_ENT(LOCAL_W), false, nullptr) || !added_per_shot)
            {
                AddCritString("Weapon cannot randomly crit.", colors::red);
                DrawCritStrings();
                return;
            }

            // We are out of sync. RIP
            if (is_out_of_sync)
                AddCritString("Out of sync.", colors::red);
            // Observed crit chance is not low enough, display how much damage is needed until we can crit again
            else if (crit_mult_info.first > crit_mult_info.second && g_pLocalPlayer->weapon_mode != weapon_melee)
                AddCritString("Damage Until crit: " + std::to_string(damageUntilToCrit(wep)), colors::orange);
            else if (!can_crit)
            {
                if (isRapidFire(wep))
                    AddCritString("Crit multiplier: " + std::to_string(getWithdrawMult(wep)), colors::orange);
                else
                    AddCritString("Shots until crit: " + std::to_string(shots_until_crit), colors::orange);
            }

            // Mark bucket as ready/not ready
            auto color = colors::red;
            if (can_crit && (crit_mult_info.first <= crit_mult_info.second || g_pLocalPlayer->weapon_mode != weapon_melee))
                color = colors::green;
            AddCritString("Crit Bucket: " + std::to_string(bucket), color);
        }

        // Draw Bar
        if (draw_meter)
        {
            // Can crit?
            if (re::C_TFWeaponBase::CanFireCriticalShot(RAW_ENT(LOCAL_W), false, nullptr) && added_per_shot)
            {
                rgba_t bucket_color = colors::FromRGBA8(0x53, 0xbc, 0x31, 255);
                // Forcing crits, change crit bucket color to a nice azure blue
                if (shouldCrit())
                    bucket_color = colors::FromRGBA8(0x34, 0xeb, 0xae, 255);
                // Color everything red
                if (!can_crit)
                    bucket_color = colors::red;

                // Get the percentage the bucket will take up
                float bucket_percentage = bucket / getBucketCap();

                // Get the percentage of the bucket after a crit
                float bucket_percentage_post_crit = bucket;

                // First run the add and subtract calculations
                bucket_percentage_post_crit += added_per_shot;
                if (bucket_percentage_post_crit > getBucketCap())
                    bucket_percentage_post_crit = getBucketCap();

                bucket_percentage_post_crit -= getWithdrawAmount(wep);

                if (bucket_percentage_post_crit < 0.0f)
                    bucket_percentage_post_crit = 0.0f;

                // Now convert to percentage
                bucket_percentage_post_crit /= getBucketCap();

                // Colors for all the different cases
                rgba_t reduction_color_base = bucket_color;
                rgba_t reduction_color      = colors::Fade(reduction_color_base, colors::white, g_GlobalVars->curtime, 2.0f);

                // Time to draw

                // Draw background
                static rgba_t background_color = colors::FromRGBA8(96, 96, 96, 150);
                float bar_bg_x_size            = *size * 2.0f;
                float bar_bg_y_size            = *size / 5.0f;
                draw::Rectangle(*bar_x - 5.0f, *bar_y - 5.0f, bar_bg_x_size + 10.0f, bar_bg_y_size + 10.0f, background_color);

                // Need special draw logic here
                if (is_out_of_sync || (crit_mult_info.first > crit_mult_info.second && g_pLocalPlayer->weapon_mode != weapon_melee) || !can_crit)
                {
                    draw::Rectangle(*bar_x, *bar_y, bar_bg_x_size * bucket_percentage, bar_bg_y_size, bucket_color);

                    if (is_out_of_sync)
                        bar_string = "Out of sync.";
                    else if (crit_mult_info.first > crit_mult_info.second && g_pLocalPlayer->weapon_mode != weapon_melee)
                        bar_string = std::to_string(damageUntilToCrit(wep)) + " Damage until Crit!";
                    else
                    {
                        if (!isRapidFire(wep))
                        {
                            bar_string = std::to_string(shots_until_crit) + " Shots until Crit!";
                        }
                        else
                            bar_string = "Crit multiplier: " + std::to_string(getWithdrawMult(wep));
                    }
                }
                // Still run when out of sync
                if (!((crit_mult_info.first > crit_mult_info.second && g_pLocalPlayer->weapon_mode != weapon_melee) || !can_crit))
                {

                    // Calculate how big the green part needs to be
                    float bucket_draw_percentage = bucket_percentage_post_crit;

                    // Are we subtracting more than we have in the buffer?
                    float x_offset_bucket = bucket_draw_percentage * bar_bg_x_size;
                    if (x_offset_bucket > 0.0f)
                        draw::Rectangle(*bar_x, *bar_y, x_offset_bucket, bar_bg_y_size, bucket_color);
                    else
                        x_offset_bucket = 0.0f;

                    // Calculate how big the Reduction part needs to be
                    float reduction_draw_percentage = bucket_percentage - bucket_percentage_post_crit;
                    if (bucket_draw_percentage < 0.0f)
                        reduction_draw_percentage += bucket_draw_percentage;
                    draw::Rectangle(*bar_x + x_offset_bucket, *bar_y, bar_bg_x_size * reduction_draw_percentage, bar_bg_y_size, reduction_color);
                }
            }
        }

        // Update
        last_bucket = bucket;
        last_wep    = wep->entindex();
        DrawCritStrings();
    }
}
#endif

// Listener for damage
class CritEventListener : public IGameEventListener
{
public:
    void FireGameEvent(KeyValues *event) override
    {
        // Reset cached Damage, round reset
        if (!strcmp(event->GetName(), "teamplay_round_start"))
        {
            crit_damage   = 0;
            melee_damage  = 0;
            round_damage  = g_pPlayerResource->GetDamage(g_pLocalPlayer->entity_idx);
            cached_damage = round_damage - melee_damage;
        }
        // Something took damage
        else if (!strcmp(event->GetName(), "player_hurt"))
        {
            int victim = g_IEngine->GetPlayerForUserID(event->GetInt("userid"));
            int health = event->GetInt("health");

            auto &status          = player_status_list[victim - 1];
            int health_difference = status.health - health;
            status.health         = health;
            status.just_updated   = true;

            // That something was hurt by us
            if (g_IEngine->GetPlayerForUserID(event->GetInt("attacker")) == g_pLocalPlayer->entity_idx)
            {
                // Don't count self damage
                if (victim != g_pLocalPlayer->entity_idx)
                {
                    // The weapon we damaged with
                    int weaponid   = event->GetInt("weaponid");
                    int weapon_idx = getWeaponByID(LOCAL_E, weaponid);

                    bool isMelee = false;
                    if (IDX_GOOD(weapon_idx))
                    {
                        int slot = re::C_BaseCombatWeapon::GetSlot(g_IEntityList->GetClientEntity(weapon_idx));
                        if (slot == 2)
                            isMelee = true;
                    }

                    // Iterate all the weapons of the local palyer for weaponid

                    // General damage counter
                    int damage = event->GetInt("damageamount");
                    if (damage > health_difference && !health)
                        damage = health_difference;

                    // Not a melee weapon
                    if (!isMelee)
                    {
                        // Crit handling
                        if (CE_BAD(LOCAL_E) || CE_BAD(LOCAL_W) || !re::CTFPlayerShared::IsCritBoosted(re::CTFPlayerShared::GetPlayerShared(RAW_ENT(LOCAL_E))))
                        {
                            // Crit damage counter
                            if (event->GetBool("crit"))
                                crit_damage += damage;
                        }
                    }
                    else
                    {
                        // Melee damage
                        melee_damage += damage;
                    }
                }
            }
        }
    }
};

static CritEventListener listener{};

void observedcritchance_nethook(const CRecvProxyData *data, void *pWeapon, void *out)
{
    // Do default action by default
    auto fl_observed_crit_chance = reinterpret_cast<float *>(out);
    *fl_observed_crit_chance     = data->m_Value.m_Float;
    if (CE_BAD(LOCAL_W) || !enabled)
        return;
    if (pWeapon != LOCAL_W->InternalEntity())
        return;

    float sent_chance = data->m_Value.m_Float;
    if (sent_chance)
    {
        // Before fix
        float old_observed_chance;
        if (debug_desync)
            old_observed_chance = getObservedCritChance();
        // Sync our chance, Player ressource is guranteed to be working, melee damage not, but it's fairly reliable
        int ranged_damage = g_pPlayerResource->GetDamage(g_pLocalPlayer->entity_idx) - melee_damage;

        // We need to do some checks for our sync process
        if (ranged_damage != 0 && 2.0f * sent_chance + 1 != 0.0f)
        {
            cached_damage = ranged_damage - round_damage;
            // Powered by math
            crit_damage = (3.0f * ranged_damage * sent_chance) / (2.0f * sent_chance + 1);
        }
        // We Were out of sync with the server
        if (debug_desync && sent_chance > old_observed_chance && fabsf(sent_chance - old_observed_chance) > 0.01f)
        {
            logging::Info("Out of sync! Observed crit chance is %f, but client expected: %f, fixed to %f", data->m_Value.m_Float, old_observed_chance, getObservedCritChance());
        }
    }
}

static ProxyFnHook observed_crit_chance_hook{};

// Reset everything
void LevelShutdown()
{
    last_crit_tick  = -1;
    cached_damage   = 0;
    crit_damage     = 0;
    melee_damage    = 0;
    last_bucket_fix = -1;
    round_damage    = 0;
    is_out_of_sync  = false;
    crit_cmds.clear();
    current_index = 0;
}

// Prints basically everything you need to know about crits
static CatCommand debug_print_crit_info("debug_print_crit_info", "Print a bunch of useful crit info", []() {
    if (CE_BAD(LOCAL_E))
        return;

    logging::Info("Player specific information:");
    logging::Info("Ranged Damage this round: %d", cached_damage - round_damage);
    logging::Info("Melee Damage this round: %d", melee_damage);
    logging::Info("Crit Damage this round: %d", crit_damage);
    logging::Info("Observed crit chance: %f", getObservedCritChance());
    if (CE_GOOD(LOCAL_W))
    {
        IClientEntity *wep = RAW_ENT(LOCAL_W);
        weapon_info info(wep);
        logging::Info("Weapon specific information:");
        logging::Info("Crit bucket: %f", info.crit_bucket);
        logging::Info("Needed Crit chance: %f", critMultInfo(wep).second);
        logging::Info("Added per shot: %f", added_per_shot);
        if (isRapidFire(wep))
            logging::Info("Subtracted per Rapidfire crit: %f", getWithdrawAmount(wep));
        else
            logging::Info("Subtracted per crit: %f", getWithdrawAmount(wep));
        logging::Info("Damage Until crit: %d", damageUntilToCrit(wep));
        logging::Info("Shots until crit: %d", shotsUntilCrit(wep));
    }
});

static InitRoutine init([]() {
    EC::Register(EC::CreateMoveLate, CreateMove, "crit_cm");
#if ENABLE_VISUALS
    EC::Register(EC::Draw, Draw, "crit_draw");
#endif
    EC::Register(EC::LevelShutdown, LevelShutdown, "crit_lvlshutdown");
    g_IGameEventManager->AddListener(&listener, false);
    HookNetvar({ "DT_TFWeaponBase", "LocalActiveTFWeaponData", "m_flObservedCritChance" }, observed_crit_chance_hook, observedcritchance_nethook);
    EC::Register(
        EC::Shutdown,
        []() {
            g_IGameEventManager->RemoveListener(&listener);
            observed_crit_chance_hook.restore();
        },
        "crit_shutdown");
    // Attached in game, out of sync
    if (g_IEngine->IsInGame())
        is_out_of_sync = true;
});
} // namespace criticals
