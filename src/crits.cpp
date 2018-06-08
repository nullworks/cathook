/*
 * crits.cpp
 *
 *  Created on: Feb 25, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"

static CatVar crit_info(CV_SWITCH, XORSTR("crit_info"), XORSTR("0"), XORSTR("Show crit info"));
static CatVar crit_key(CV_KEY, XORSTR("crit_key"), XORSTR("0"), XORSTR("Crit Key"));
static CatVar crit_melee(CV_SWITCH, XORSTR("crit_melee"), XORSTR("0"), XORSTR("Melee crits"));
static CatVar crit_legiter(
    CV_SWITCH, XORSTR("crit_force_gameplay"), XORSTR("0"), XORSTR("Don't hinder gameplay"),
    XORSTR("Attempt to crit when possible but do not hinder normal gameplay"));
static CatVar crit_experimental(CV_SWITCH, XORSTR("crit_experimental"), XORSTR("0"),
                                XORSTR("Experimental crithack"));

std::unordered_map<int, int> command_number_mod{};

int *g_PredictionRandomSeed = nullptr;

namespace criticals
{

int find_next_random_crit_for_weapon(IClientEntity *weapon)
{
    int tries = 0, number = g_pUserCmd->command_number, found = 0, seed,
        seed_md5, seed_backup;

    crithack_saved_state state;
    state.Save(weapon);

    seed_backup = *g_PredictionRandomSeed;

    while (!found && tries < 4096)
    {
        seed_md5                = MD5_PseudoRandom(number) & 0x7FFFFFFF;
        *g_PredictionRandomSeed = seed_md5;
        seed  = seed_md5 ^ (LOCAL_E->m_IDX | (LOCAL_W->m_IDX << 8));
        found = re::C_TFWeaponBase::CalcIsAttackCritical(weapon);
        if (found)
            break;
        ++tries;
        ++number;
    }

    *g_PredictionRandomSeed = seed_backup;
    state.Load(weapon);
    if (found)
        return number;
    return 0;
}

void unfuck_bucket(IClientEntity *weapon)
{
    static bool changed;
    static float last_bucket;
    static int last_weapon;

    if (g_pUserCmd->command_number)
        changed = false;

    float &bucket = re::C_TFWeaponBase::crit_bucket_(weapon);
    ;
    if (GetWeaponMode() == weapon_melee)
        bucket = 1000.0f;

    if (bucket != last_bucket)
    {
        if (changed && weapon->entindex() == last_weapon)
        {
            bucket = last_bucket;
        }
        changed = true;
    }
    last_weapon = weapon->entindex();
    last_bucket = bucket;
}

struct cached_calculation_s
{
    int command_number;
    int init_command;
    int weapon_entity;
};

cached_calculation_s cached_calculation{};

static int number                = 0;
static int lastnumber            = 0;
static int lastusercmd           = 0;
static const model_t *lastweapon = nullptr;

bool force_crit(IClientEntity *weapon)
{
    if (lastnumber < g_pUserCmd->command_number ||
        lastweapon != weapon->GetModel() ||
        lastnumber - g_pUserCmd->command_number > 1000)
    {
        if (cached_calculation.init_command > g_pUserCmd->command_number ||
            g_pUserCmd->command_number - cached_calculation.init_command >
                4096 ||
            (g_pUserCmd->command_number &&
             (cached_calculation.command_number < g_pUserCmd->command_number)))
            cached_calculation.weapon_entity = 0;
        if (cached_calculation.weapon_entity == weapon->entindex())
            return bool(cached_calculation.command_number);

        number = find_next_random_crit_for_weapon(weapon);
    }
    else
        number = lastnumber;
    logging::Info(XORSTR("Found critical: %d -> %d"), g_pUserCmd->command_number,
                  number);
    lastweapon = weapon->GetModel();
    lastnumber = number;
    if (crit_experimental)
    {
        if (!crit_legiter)
        {
            if (number && number != g_pUserCmd->command_number)
                command_number_mod[g_pUserCmd->command_number] = number;

            cached_calculation.command_number = number;
            cached_calculation.weapon_entity  = LOCAL_W->m_IDX;
        }
        else
        {
            if (number && number - 30 < g_pUserCmd->command_number)
                command_number_mod[g_pUserCmd->command_number] = number;

            cached_calculation.command_number = number;
            cached_calculation.weapon_entity  = LOCAL_W->m_IDX;
        }
    }
    else
    {
        if (!crit_legiter)
        {
            if (g_pUserCmd->command_number != number && number &&
                number != g_pUserCmd->command_number)
                g_pUserCmd->buttons &= ~IN_ATTACK;
            else
                g_pUserCmd->buttons |= IN_ATTACK;
        }
        else
        {
            if (g_pUserCmd->command_number + 30 > number && number &&
                number != g_pUserCmd->command_number)
                g_pUserCmd->buttons &= ~IN_ATTACK;
            else
                g_pUserCmd->buttons |= IN_ATTACK;
        }
    }
    return !!number;
}

void create_move()
{
    if (!crit_key && !crit_melee)
        return;
    if (!random_crits_enabled())
        return;
    if (CE_BAD(LOCAL_W))
        return;
    if (g_pUserCmd->command_number)
        lastusercmd       = g_pUserCmd->command_number;
    IClientEntity *weapon = RAW_ENT(LOCAL_W);
    if (!re::C_TFWeaponBase::IsBaseCombatWeapon(weapon))
        return;
    if (!re::C_TFWeaponBase::AreRandomCritsEnabled(weapon))
        return;
    unfuck_bucket(weapon);
    if ((g_pUserCmd->buttons & IN_ATTACK) && crit_key.KeyDown() &&
        g_pUserCmd->command_number && crit_key)
    {
        force_crit(weapon);
    }
    else if ((g_pUserCmd->buttons & IN_ATTACK) && g_pUserCmd->command_number &&
             GetWeaponMode() == weapon_melee && crit_melee &&
             g_pLocalPlayer->weapon()->m_iClassID() != CL_CLASS(CTFKnife))
    {
        force_crit(weapon);
    }
}

bool random_crits_enabled()
{
    static ConVar *tf_weapon_criticals =
        g_ICvar->FindVar(XORSTR("tf_weapon_criticals"));
    return tf_weapon_criticals->GetBool();
}

#if ENABLE_VISUALS
void draw()
{
    if (CE_BAD(LOCAL_W))
        return;
    IClientEntity *weapon = RAW_ENT(LOCAL_W);
    if (!weapon)
        return;
    if (!re::C_TFWeaponBase::IsBaseCombatWeapon(weapon))
        return;
    if (!re::C_TFWeaponBase::AreRandomCritsEnabled(weapon))
        return;
    if (crit_info && CE_GOOD(LOCAL_W))
    {
        if (crit_key.KeyDown())
        {
            AddCenterString(XORSTR("FORCED CRITS!"), colors::red);
        }
        IF_GAME(IsTF2())
        {
            if (!random_crits_enabled())
                AddCenterString(XORSTR("Random crits are disabled"), colors::yellow);
            else
            {
                if (!re::C_TFWeaponBase::CanFireCriticalShot(RAW_ENT(LOCAL_W),
                                                             false, nullptr))
                    AddCenterString(XORSTR("Weapon can't randomly crit"),
                                    colors::yellow);
                else if (lastusercmd)
                {
                    if (number > lastusercmd)
                    {
                        float nextcrit =
                            ((float) number - (float) lastusercmd) / (float) 90;
                        if (nextcrit > 0.0f)
                        {
                            AddCenterString(
                                format(XORSTR("Time to next crit: "), nextcrit, XORSTR("s")),
                                colors::orange);
                        }
                    }
                    AddCenterString(XORSTR("Weapon can randomly crit"));
                }
            }
            if (GetWeaponMode() == weapon_melee)
                AddCenterString(format(XORSTR("Bucket: 1000")));
            else
                AddCenterString(
                    format(XORSTR("Bucket: "),
                           re::C_TFWeaponBase::crit_bucket_(RAW_ENT(LOCAL_W))));
        }
        // AddCenterString(format(XORSTR("Time: "),
        // *(float*)((uintptr_t)RAW_ENT(LOCAL_W) + 2872u)));
    }
}
#endif
}

void crithack_saved_state::Load(IClientEntity *entity)
{
    *(float *) (uintptr_t(entity) + 2868) = unknown2868;
    *(float *) (uintptr_t(entity) + 2864) = unknown2864;
    *(float *) (uintptr_t(entity) + 2880) = unknown2880;
    *(float *) (uintptr_t(entity) + 2616) = bucket2616;
    *(int *) (uintptr_t(entity) + 2620)   = unknown2620;
    *(int *) (uintptr_t(entity) + 2876)   = seed2876;
    *(char *) (uintptr_t(entity) + 2839)  = unknown2839;
}

void crithack_saved_state::Save(IClientEntity *entity)
{
    unknown2868 = *(float *) (uintptr_t(entity) + 2868);
    unknown2864 = *(float *) (uintptr_t(entity) + 2864);
    unknown2880 = *(float *) (uintptr_t(entity) + 2880);
    bucket2616  = *(float *) (uintptr_t(entity) + 2616);
    unknown2620 = *(int *) (uintptr_t(entity) + 2620);
    seed2876    = *(int *) (uintptr_t(entity) + 2876);
    unknown2839 = *(char *) (uintptr_t(entity) + 2839);
}
