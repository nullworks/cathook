/*
 * AutoHeal.cpp
 *
 *  Created on: Dec 3, 2016
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include <hacks/AutoHeal.hpp>
#include <hacks/FollowBot.hpp>
#include <settings/Bool.hpp>

static settings::Bool enable{ "autoheal.enable", "false" };
static settings::Bool silent{ "autoheal.silent", "true" };
static settings::Bool pop_uber_auto{ "autoheal.uber.enable", "true" };
static settings::Float pop_uber_percent{ "autoheal.uber.health-below-ratio", "0" };
static settings::Bool share_uber{ "autoheal.uber.share", "true" };

static settings::Bool auto_vacc{ "autoheal.vacc.enable", "false" };

static settings::Bool auto_vacc_bullets{ "autoheal.vacc.bullet.enable", "true" };
static settings::Int vacc_sniper{ "autoheal.vacc.bullet.sniper-pop", "true" };

static settings::Bool auto_vacc_fire_checking{ "autoheal.vacc.fire.enable", "true" };
static settings::Int auto_vacc_pop_if_pyro{ "autoheal.vacc.fire.pyro-pop", "1" };
static settings::Bool auto_vacc_check_on_fire{ "autoheal.vacc.fire.prevent-afterburn", "true" };
static settings::Int auto_vacc_pyro_range{ "autoheal.vacc.fire.pyro-range", "450" };

static settings::Bool auto_vacc_blast_checking{ "autoheal.vacc.blast.enable", "true" };
static settings::Bool auto_vacc_blast_crit_pop{ "autoheal.vacc.blast.crit-pop", "true" };
static settings::Int auto_vacc_blast_health{ "autoheal.vacc.blast.pop-near-rocket-health", "80" };
static settings::Int auto_vacc_proj_danger_range{ "autoheal.vacc.blast.danger-range", "650" };

static settings::Int change_timer{ "autoheal.vacc.reset-timer", "200" };

static settings::Int auto_vacc_bullet_pop_ubers{ "autoheal.vacc.bullet.min-charges", "0" };
static settings::Int auto_vacc_fire_pop_ubers{ "autoheal.vacc.fire.min-charges", "0" };
static settings::Int auto_vacc_blast_pop_ubers{ "autoheal.vacc.blast.min-charges", "0" };

static settings::Int default_resistance{ "autoheal.vacc.default-resistance", "0" };

namespace hacks::tf::autoheal
{

int m_iCurrentHealingTarget{ -1 };
int m_iNewTarget{ 0 };

int vaccinator_change_stage = 0;
int vaccinator_change_ticks = 0;
int vaccinator_ideal_resist = 0;
int vaccinator_change_timer = 0;

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
        if (CE_BAD(ent))
            continue;
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
            if (!IsEntityVisible(ent, head) &&
                !IsVectorVisible(ENTITY(m_iCurrentHealingTarget)
                                     ->hitboxes.GetHitbox(head)
                                     ->center,
                                 ent->hitboxes.GetHitbox(head)->center, true))
                continue;
        return vacc_sniper ? 2 : 1;
    }
    return any_zoomed_snipers;
}

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
            if (CE_BAD(ent))
                continue;
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


bool IsProjectile(CachedEntity *ent)
{
    return (ent->m_iClassID() == CL_CLASS(CTFProjectile_Rocket) ||
            ent->m_iClassID() == CL_CLASS(CTFProjectile_Flare) ||
            ent->m_iClassID() == CL_CLASS(CTFProjectile_EnergyBall) ||
            ent->m_iClassID() == CL_CLASS(CTFProjectile_HealingBolt) ||
            ent->m_iClassID() == CL_CLASS(CTFProjectile_Arrow) ||
            ent->m_iClassID() == CL_CLASS(CTFProjectile_SentryRocket) ||
            ent->m_iClassID() == CL_CLASS(CTFProjectile_Cleaver) ||
            ent->m_iClassID() == CL_CLASS(CTFGrenadePipebombProjectile) ||
            ent->m_iClassID() == CL_CLASS(CTFProjectile_EnergyRing));
}

int NearbyEntities()
{
    int ret = 0;
    if (CE_BAD(LOCAL_E))
        return ret;
    for (int i = 0; i < HIGHEST_ENTITY; i++)
    {
        CachedEntity *ent = ENTITY(i);
        if (CE_BAD(ent))
            continue;
        if (ent == LOCAL_E)
            continue;
        if (!ent->m_bAlivePlayer())
            continue;
        if (ent->m_flDistance() <= 300.0f)
            ret++;
    }
    return ret;
}

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
    if (current_user_cmd->buttons & IN_RELOAD)
    {
        vaccinator_change_ticks = 8;
        return;
    }
    else
    {
        if (vaccinator_change_ticks <= 0)
        {
            current_user_cmd->buttons |= IN_RELOAD;
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
    "autoheal_heal_steamid", "Heals a player with SteamID",
    [](const CCommand &args) {
        if (args.ArgC() < 2)
        {
            logging::Info("Invalid call!");
            force_healing_target = 0;
            return;
        }
        if (strtol(args.Arg(1), nullptr, 10) == 0x0)
        {
            force_healing_target = 0;
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

static CatCommand vaccinator_bullet("vacc_bullet", "Bullet Vaccinator",
                                    []() { SetResistance(0); });
static CatCommand vaccinator_blast("vacc_blast", "Blast Vaccinator",
                                   []() { SetResistance(1); });
static CatCommand vaccinator_fire("vacc_fire", "Fire Vaccinator",
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
            current_user_cmd->buttons |= IN_ATTACK2;
        }
    }
    if (!force_healing_target && !enable)
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
            AimAt(g_pLocalPlayer->v_Eye, out, current_user_cmd);
            current_user_cmd->buttons |= IN_ATTACK;
        }
    }
    if (!enable)
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

    AimAt(g_pLocalPlayer->v_Eye, out, current_user_cmd);
    if (silent)
        g_pLocalPlayer->bUseSilentAngles = true;
    if (!m_iNewTarget && (g_GlobalVars->tickcount % 300))
        current_user_cmd->buttons |= IN_ATTACK;
    /*if (m_iNewTarget || !(g_GlobalVars->tickcount % 300)) {
        if (silent) g_pLocalPlayer->bUseSilentAngles = true;
        AimAt(g_pLocalPlayer->v_Eye, out, current_user_cmd);
        current_user_cmd->buttons |= IN_ATTACK;
    }*/
    if (IsVaccinator() && CE_GOOD(target) && auto_vacc)
    {
        int opt = OptimalResistance(target, &pop);
        if (!pop && opt != -1)
            SetResistance(opt);
        if (pop && CurrentResistance() == opt)
        {
            current_user_cmd->buttons |= IN_ATTACK2;
        }
    }
    else
    {
        if (pop_uber_auto && ShouldPop())
            current_user_cmd->buttons |= IN_ATTACK2;
    }
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
    priority += hacks::shared::followbot::ClassPriority(ent) * 1.3;
    switch (playerlist::AccessData(ent).state)
    {
    case playerlist::k_EState::FRIEND:
        priority += 70 * (1 - healthp);
        priority += 5 * (1 - overhealp);
        break;
    case playerlist::k_EState::IPC:
        priority += 100 * (1 - healthp);
        priority += 10 * (1 - overhealp);
        break;
    default:
        priority += 40 * (1 - healthp);
        priority += 3 * (1 - overhealp);
    }
#if ENABLE_IPC
    if (ipc::peer)
    {
        if (hacks::shared::followbot::isEnabled() &&
            hacks::shared::followbot::getTarget() == idx)
        {
            priority *= 6.0f;
        }
    }
#endif
/*    player_info_s info;
    g_IEngine->GetPlayerInfo(idx, &info);
    info.name[31] = 0;
    if (strcasestr(info.name, ignore.GetString()))
        priority = 0.0f;*/
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
