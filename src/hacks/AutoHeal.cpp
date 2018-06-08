/*
 * AutoHeal.cpp
 *
 *  Created on: Dec 3, 2016
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include <hacks/AutoHeal.hpp>
#include <hacks/FollowBot.hpp>

namespace hacks
{
namespace tf
{
namespace autoheal
{

static CatVar enabled(CV_SWITCH, XORSTR("autoheal_enabled"), XORSTR("0"), XORSTR("AutoHeal"),
                      XORSTR("Automatically heals nearby teammates"));
static CatVar silent(CV_SWITCH, XORSTR("autoheal_silent"), XORSTR("1"), XORSTR("Silent AutoHeal"),
                     XORSTR("Silent AutoHeal. Disable this to make ghetto followbot"));
// extern CatVar target_only;

int m_iCurrentHealingTarget{ -1 };
int m_iNewTarget{ 0 };

static CatVar pop_uber_auto(CV_SWITCH, XORSTR("autoheal_uber"), XORSTR("1"), XORSTR("AutoUber"),
                            XORSTR("Use ubercharge automatically"));
static CatVar
    pop_uber_percent(CV_FLOAT, XORSTR("autoheal_uber_health"), XORSTR("30"),
                     XORSTR("Pop uber if health% <"),
                     XORSTR("When under a percentage of health, use ubercharge"));
static CatVar share_uber(
    CV_SWITCH, XORSTR("autoheal_share_uber"), XORSTR("1"), XORSTR("Share ubercharge"),
    XORSTR("Aimbot will attempt to share uber charge with un-ubered players"));

int vaccinator_change_stage = 0;
int vaccinator_change_ticks = 0;
int vaccinator_ideal_resist = 0;
int vaccinator_change_timer = 0;

static CatVar auto_vacc_bullets(CV_SWITCH, XORSTR("auto_vacc_bullets"), XORSTR("1"),
                                XORSTR("Check bullet danger"));
static CatEnum vacc_sniper_enum({ XORSTR("NEVER"), XORSTR("ZOOM & VISIBLE"), XORSTR("ANY ZOOMED") });
static CatVar vacc_sniper(vacc_sniper_enum, XORSTR("auto_vacc_sniper_pop"), XORSTR("1"),
                          XORSTR("Pop if Sniper"),
                          XORSTR("Defines Auto-Vacc behaviour with snipers"));
static CatVar ignore(CV_STRING, XORSTR("autoheal_ignore"), XORSTR(""), XORSTR("Ignore"),
                     XORSTR("Ignore people with this name"));

int ChargeCount()
{
    return (CE_FLOAT(LOCAL_W, netvar.m_flChargeLevel) / 0.25f);
}

// TODO Angle Checking
int BulletDangerValue(CachedEntity *patient)
{
    // Find zoomed in snipers in other team
    bool any_zoomed_snipers = false;
    for (int i = 1; i < 32 && i < HIGHEST_ENTITY; i++)
    {
        CachedEntity *ent = ENTITY(i);
        if (!ent->m_bEnemy())
            continue;
        if (g_pPlayerResource->GetClass(ent) != tf_sniper)
            continue;
        if (CE_BYTE(ent, netvar.iLifeState))
            continue;
        if (!HasCondition<TFCond_Zoomed>(ent))
            continue;
        any_zoomed_snipers = true;
        // TODO VisCheck from patient.
        if ((int) vacc_sniper == 1)
            if (!IsEntityVisible(ent, head))
                continue;
        return vacc_sniper ? 2 : 1;
    }
    return any_zoomed_snipers;
}

static CatVar auto_vacc_fire_checking(CV_SWITCH, XORSTR("auto_vacc_fire"), XORSTR("1"),
                                      XORSTR("Check fire danger"));
static CatEnum pyro_enum({ XORSTR("NEVER"), XORSTR("PRIMARY OUT"), XORSTR("ALWAYS") });
static CatVar auto_vacc_pop_if_pyro(pyro_enum, XORSTR("auto_vacc_fire_pop_pyro"), XORSTR("1"),
                                    XORSTR("Pop if pyro is near"),
                                    XORSTR("Defines Auto-Vacc behaviour with pyros"));
static CatVar auto_vacc_check_on_fire(CV_SWITCH, XORSTR("auto_vacc_afterburn"), XORSTR("1"),
                                      XORSTR("Anti-Afterburn"));
static CatVar auto_vacc_pyro_range(CV_INT, XORSTR("auto_vacc_pyro_range"), XORSTR("450"),
                                   XORSTR("Pyro Danger Range"));

int FireDangerValue(CachedEntity *patient)
{
    // Find nearby pyros
    if (!auto_vacc_fire_checking)
        return 0;
    if (auto_vacc_pop_if_pyro)
    {
        for (int i = 1; i < 32 && i < HIGHEST_ENTITY; i++)
        {
            CachedEntity *ent = ENTITY(i);
            if (!ent->m_bEnemy())
                continue;
            if (g_pPlayerResource->GetClass(ent) != tf_pyro)
                continue;
            if (CE_BYTE(ent, netvar.iLifeState))
                continue;
            if (patient->m_vecOrigin().DistTo(ent->m_vecOrigin()) >
                (int) auto_vacc_pyro_range)
                continue;
            if ((int) auto_vacc_pop_if_pyro == 2)
                return 2;
            IClientEntity *pyro_weapon = g_IEntityList->GetClientEntity(
                CE_INT(ent, netvar.hActiveWeapon) & 0xFFF);
            return (pyro_weapon &&
                    pyro_weapon->GetClientClass()->m_ClassID ==
                        CL_CLASS(CTFFlameThrower))
                       ? 2
                       : 0;
        }
    }
    if (HasCondition<TFCond_OnFire>(patient))
    {
        return (bool) auto_vacc_check_on_fire;
    }
    return 0;
}

struct proj_data_s
{
    int eid;
    Vector last_pos;
};

std::vector<proj_data_s> proj_data_array;

static CatVar auto_vacc_blast_health(CV_INT, XORSTR("auto_vacc_blast_pop_health"), XORSTR("80"),
                                     XORSTR("Pop Blast if rocket & HP <"));
static CatVar auto_vacc_blast_crit_pop(CV_SWITCH, XORSTR("auto_vacc_blast_pop_crit"),
                                       XORSTR("1"), XORSTR("Pop Blast if crit rocket near"));
static CatVar auto_vacc_blast_checking(CV_SWITCH, XORSTR("auto_vacc_blast"), XORSTR("1"),
                                       XORSTR("Check blast danger"));
static CatVar auto_vacc_proj_danger_range(CV_INT, XORSTR("auto_vacc_rocket_range"),
                                          XORSTR("650"), XORSTR("Rocket Danger Range"),
                                          XORSTR("This range should be high enough to "
                                          "give more time to change "
                                          "resistances."));

int BlastDangerValue(CachedEntity *patient)
{
    if (!auto_vacc_blast_checking)
        return 0;
    // Check rockets for being closer
    bool hasCritRockets = false;
    bool hasRockets     = false;
    for (auto it = proj_data_array.begin(); it != proj_data_array.end();)
    {
        const auto &d     = *it;
        CachedEntity *ent = ENTITY(d.eid);
        if (CE_GOOD(ent))
        {
            // Rocket is getting closer
            if (patient->m_vecOrigin().DistToSqr(d.last_pos) >
                patient->m_vecOrigin().DistToSqr(ent->m_vecOrigin()))
            {
                if (ent->m_bCritProjectile())
                    hasCritRockets = true;
                hasRockets         = true;
            }
            it++;
        }
        else
        {
            proj_data_array.erase(it);
        }
    }
    if (hasRockets)
    {
        if (patient->m_iHealth() < (int) auto_vacc_blast_health ||
            (auto_vacc_blast_crit_pop && hasCritRockets))
        {
            return 2;
        }
        return 1;
    }
    // Find crit rockets/pipes nearby
    for (int i = 32; i < HIGHEST_ENTITY; i++)
    {
        CachedEntity *ent = ENTITY(i);
        if (CE_BAD(ent))
            continue;
        if (!ent->m_bEnemy())
            continue;
        if (ent->m_Type() != ENTITY_PROJECTILE)
            continue;
        if (patient->m_vecOrigin().DistTo(ent->m_vecOrigin()) >
            (int) auto_vacc_proj_danger_range)
            continue;
        proj_data_array.push_back(proj_data_s{ i, ent->m_vecOrigin() });
    }
    return 0;
}

int CurrentResistance()
{
    if (LOCAL_W->m_iClassID() != CL_CLASS(CWeaponMedigun))
        return 0;
    return CE_INT(LOCAL_W, netvar.m_nChargeResistType);
}

static CatVar change_timer(CV_INT, XORSTR("auto_vacc_reset_timer"), XORSTR("200"),
                           XORSTR("Reset Timer"),
                           XORSTR("If no dangers were detected for # ticks, "
                           "resistance will be reset to default, 0 to disable"));

static CatVar auto_vacc_bullet_pop_ubers(
    CV_INT, XORSTR("auto_vacc_bullet_pop_ubers"), XORSTR("0"), XORSTR("Pop Bullet if Ubers >="),
    XORSTR("Only pop an uber if you have >= # Ubercharges in your Vaccinator"), 0, 4);
static CatVar auto_vacc_fire_pop_ubers(
    CV_INT, XORSTR("auto_vacc_fire_pop_ubers"), XORSTR("0"), XORSTR("Pop Fire if Ubers >="),
    XORSTR("Only pop an uber if you have >= # Ubercharges in your Vaccinator"), 0, 4);
static CatVar auto_vacc_blast_pop_ubers(
    CV_INT, XORSTR("auto_vacc_blast_pop_ubers"), XORSTR("0"), XORSTR("Pop Blast if Ubers >="),
    XORSTR("Only pop an uber if you have >= # Ubercharges in your Vaccinator"), 0, 4);

int OptimalResistance(CachedEntity *patient, bool *shouldPop)
{
    int bd = BlastDangerValue(patient), fd = FireDangerValue(patient),
        hd = BulletDangerValue(patient);
    if (shouldPop)
    {
        int charges = ChargeCount();
        if (bd > 1 && charges >= (int) auto_vacc_blast_pop_ubers)
            *shouldPop = true;
        if (fd > 1 && charges >= (int) auto_vacc_fire_pop_ubers)
            *shouldPop = true;
        if (hd > 1 && charges >= (int) auto_vacc_bullet_pop_ubers)
            *shouldPop = true;
    }
    if (!hd && !fd && !bd)
        return -1;
    vaccinator_change_timer = (int) change_timer;
    if (hd >= fd && hd >= bd)
        return 0;
    if (bd >= fd && bd >= hd)
        return 1;
    if (fd >= hd && fd >= bd)
        return 2;
    return -1;
}

static CatEnum resistances_enum({ XORSTR("BULLET"), XORSTR("BLAST"), XORSTR("FIRE") });
static CatVar default_resistance(resistances_enum, XORSTR("auto_vacc_default_resist"),
                                 XORSTR("0"), XORSTR("Default Resistance"),
                                 XORSTR("Select default resistance type"));

void SetResistance(int resistance)
{
    resistance              = _clamp(0, 2, resistance);
    vaccinator_change_timer = (int) change_timer;
    vaccinator_ideal_resist = resistance;
    int cur                 = CurrentResistance();
    if (resistance == cur)
        return;
    if (resistance > cur)
        vaccinator_change_stage = resistance - cur;
    else
        vaccinator_change_stage = 3 - cur + resistance;
}

void DoResistSwitching()
{
    if (vaccinator_change_timer > 0)
    {
        if (vaccinator_change_timer == 1)
        {
            SetResistance((int) default_resistance);
        }
        vaccinator_change_timer--;
    }
    if (!vaccinator_change_stage)
        return;
    if (CurrentResistance() == vaccinator_ideal_resist)
    {
        vaccinator_change_ticks = 0;
        vaccinator_change_stage = 0;
        return;
    }
    if (g_pUserCmd->buttons & IN_RELOAD)
    {
        vaccinator_change_ticks = 8;
        return;
    }
    else
    {
        if (vaccinator_change_ticks <= 0)
        {
            g_pUserCmd->buttons |= IN_RELOAD;
            vaccinator_change_stage--;
            vaccinator_change_ticks = 8;
        }
        else
        {
            vaccinator_change_ticks--;
        }
    }
}

int force_healing_target{ 0 };
static CatCommand heal_steamid(
    XORSTR("autoheal_heal_steamid"),
    XORSTR("Heals a player with SteamID (ONCE. Use for easy airstuck med setup)"),
    [](const CCommand &args) {
        if (args.ArgC() < 2)
        {
            logging::Info(XORSTR("Invalid call!"));
            return;
        }
        for (int i = 1; i <= 32 && i < HIGHEST_ENTITY; i++)
        {
            CachedEntity *ent = ENTITY(i);
            if (CE_BAD(ent))
                continue;
            if (ent->m_Type() != ENTITY_PLAYER)
                continue;
            if (ent->player_info.friendsID == strtol(args.Arg(1), nullptr, 10))
            {
                force_healing_target = i;
                return;
            }
        }
    });

static CatCommand vaccinator_bullet(XORSTR("vacc_bullet"), XORSTR("Bullet Vaccinator"),
                                    []() { SetResistance(0); });
static CatCommand vaccinator_blast(XORSTR("vacc_blast"), XORSTR("Blast Vaccinator"),
                                   []() { SetResistance(1); });
static CatCommand vaccinator_fire(XORSTR("vacc_fire"), XORSTR("Fire Vaccinator"),
                                  []() { SetResistance(2); });

bool IsPopped()
{
    CachedEntity *weapon = g_pLocalPlayer->weapon();
    if (CE_BAD(weapon) || weapon->m_iClassID() != CL_CLASS(CWeaponMedigun))
        return false;
    return CE_BYTE(weapon, netvar.bChargeRelease);
}

bool ShouldChargePlayer(int idx)
{
    CachedEntity *target = ENTITY(idx);
    const float damage_accum_duration =
        g_GlobalVars->curtime - data[idx].accum_damage_start;
    const int health = target->m_iHealth();
    if (!data[idx].accum_damage_start)
        return false;
    if (health > 30 && data[idx].accum_damage < 45)
        return false;
    const float dd = ((float) data[idx].accum_damage / damage_accum_duration);
    if (dd > 40)
    {
        return true;
    }
    if (health < 30 && data[idx].accum_damage > 10)
        return true;
    return false;
}

bool ShouldPop()
{
    if (IsPopped())
        return false;
    if (m_iCurrentHealingTarget != -1)
    {
        CachedEntity *target = ENTITY(m_iCurrentHealingTarget);
        if (CE_GOOD(target))
        {
            if (ShouldChargePlayer(m_iCurrentHealingTarget))
                return true;
        }
    }
    return ShouldChargePlayer(LOCAL_E->m_IDX);
}

bool IsVaccinator()
{
    // DefIDX: 998
    return CE_INT(LOCAL_W, netvar.iItemDefinitionIndex) == 998;
}

static CatVar auto_vacc(CV_SWITCH, XORSTR("auto_vacc"), XORSTR("0"), XORSTR("Auto Vaccinator"),
                        XORSTR("Pick resistance for incoming damage types"));

void CreateMove()
{
    bool pop = false;
    if (IsVaccinator() && auto_vacc)
    {
        DoResistSwitching();
        int my_opt = OptimalResistance(LOCAL_E, &pop);
        if (my_opt >= 0 && my_opt != CurrentResistance())
        {
            SetResistance(my_opt);
        }
        if (pop && CurrentResistance() == my_opt)
        {
            g_pUserCmd->buttons |= IN_ATTACK2;
        }
    }
    if (!enabled && !force_healing_target)
        return;
    if (GetWeaponMode() != weapon_medigun)
        return;
    if (force_healing_target)
    {
        CachedEntity *target = ENTITY(force_healing_target);
        if (CE_GOOD(target))
        {
            Vector out;
            GetHitbox(target, 7, out);
            AimAt(g_pLocalPlayer->v_Eye, out, g_pUserCmd);
            g_pUserCmd->buttons |= IN_ATTACK;
            force_healing_target = 0;
        }
    }
    if (!enabled)
        return;
    UpdateData();
    int old_target          = m_iCurrentHealingTarget;
    m_iCurrentHealingTarget = BestTarget();
    if (m_iNewTarget > 0 && m_iNewTarget < 10)
        m_iNewTarget++;
    else
        m_iNewTarget = 0;
    bool new_target  = (old_target != m_iCurrentHealingTarget);
    if (new_target)
    {
        m_iNewTarget = 1;
    }
    if (m_iCurrentHealingTarget == -1)
        return;
    CachedEntity *target = ENTITY(m_iCurrentHealingTarget);
    Vector out;
    GetHitbox(target, 7, out);

    AimAt(g_pLocalPlayer->v_Eye, out, g_pUserCmd);
    if (silent)
        g_pLocalPlayer->bUseSilentAngles = true;
    if (!m_iNewTarget && (g_GlobalVars->tickcount % 300))
        g_pUserCmd->buttons |= IN_ATTACK;
    /*if (m_iNewTarget || !(g_GlobalVars->tickcount % 300)) {
        if (silent) g_pLocalPlayer->bUseSilentAngles = true;
        AimAt(g_pLocalPlayer->v_Eye, out, g_pUserCmd);
        g_pUserCmd->buttons |= IN_ATTACK;
    }*/
    if (IsVaccinator() && CE_GOOD(target) && auto_vacc)
    {
        int opt = OptimalResistance(target, &pop);
        if (!pop && opt != -1)
            SetResistance(opt);
        if (pop && CurrentResistance() == opt)
        {
            g_pUserCmd->buttons |= IN_ATTACK2;
        }
    }
    else
    {
        if (pop_uber_auto && ShouldPop())
            g_pUserCmd->buttons |= IN_ATTACK2;
    }
    return;
}

std::vector<patient_data_s> data(32);
void UpdateData()
{
    for (int i = 1; i < 32; i++)
    {
        CachedEntity *ent = ENTITY(i);
        if (CE_GOOD(ent))
        {
            int health = ent->m_iHealth();
            if (data[i].last_damage > g_GlobalVars->curtime)
            {
                data[i].last_damage = 0.0f;
            }
            if (g_GlobalVars->curtime - data[i].last_damage > 5.0f)
            {
                data[i].accum_damage       = 0;
                data[i].accum_damage_start = 0.0f;
            }
            const int last_health = data[i].last_health;
            if (health != last_health)
            {
                data[i].last_health = health;
                if (health < last_health)
                {
                    data[i].accum_damage += (last_health - health);
                    if (!data[i].accum_damage_start)
                        data[i].accum_damage_start = g_GlobalVars->curtime;
                    data[i].last_damage            = g_GlobalVars->curtime;
                }
            }
        }
    }
}

int BestTarget()
{
    int best       = -1;
    int best_score = -65536;
    for (int i = 0; i < 32 && i < HIGHEST_ENTITY; i++)
    {
        int score = HealingPriority(i);
        if (score > best_score && score != -1)
        {
            best       = i;
            best_score = score;
        }
    }
    return best;
}

int HealingPriority(int idx)
{
    if (!CanHeal(idx))
        return -1;
    CachedEntity *ent = ENTITY(idx);
    if (share_uber && IsPopped())
    {
        return !HasCondition<TFCond_Ubercharged>(ent);
    }

    int priority        = 0;
    int health          = CE_INT(ent, netvar.iHealth);
    int maxhealth       = g_pPlayerResource->GetMaxHealth(ent);
    int maxbuffedhealth = maxhealth * 1.5;
    int maxoverheal     = maxbuffedhealth - maxhealth;
    int overheal        = maxoverheal - (maxbuffedhealth - health);
    float overhealp     = ((float) overheal / (float) maxoverheal);
    float healthp       = ((float) health / (float) maxhealth);
    switch (playerlist::AccessData(ent).state)
    {
    case playerlist::k_EState::FRIEND:
        priority += 70 * (1 - healthp);
        priority += 15 * (1 - overhealp);
        break;
    case playerlist::k_EState::IPC:
        priority += 100 * (1 - healthp);
        priority += 20 * (1 - overhealp);
        break;
    default:
        priority += 50 * (1 - healthp);
        priority += 10 * (1 - overhealp);
    }
#if ENABLE_IPC
    if (ipc::peer)
    {
        if (hacks::shared::followbot::followbot &&
            hacks::shared::followbot::follow_target == idx)
        {
            priority *= 3.0f;
        }
    }
#endif
    player_info_s info;
    g_IEngine->GetPlayerInfo(idx, &info);
    info.name[31] = 0;
    if (strcasestr(info.name, ignore.GetString()))
        priority = 0.0f;
    return priority;
}

bool CanHeal(int idx)
{
    CachedEntity *ent = ENTITY(idx);
    if (!ent)
        return false;
    if (CE_BAD(ent))
        return false;
    if (ent->m_Type() != ENTITY_PLAYER)
        return false;
    if (g_IEngine->GetLocalPlayer() == idx)
        return false;
    if (!ent->m_bAlivePlayer())
        return false;
    if (ent->m_bEnemy())
        return false;
    if (ent->m_flDistance() > 420)
        return false;
    // TODO visible any hitbox
    if (!IsEntityVisible(ent, 7))
        return false;
    if (IsPlayerInvisible(ent))
        return false;
    return true;
}
}
}
}
