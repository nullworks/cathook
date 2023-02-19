/*
 * AutoHeal.cpp
 *
 *  Created on: Dec 3, 2016
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "hacks/FollowBot.hpp"
#include "settings/Bool.hpp"
#include "PlayerTools.hpp"
#include "MiscAimbot.hpp"

namespace hacks::tf::autoheal
{
std::vector<int> called_medic{};
static settings::Boolean enable{ "autoheal.enable", "false" };
static settings::Boolean steamid_only{ "autoheal.steam-only", "false" };
static settings::Boolean silent{ "autoheal.silent", "true" };
static settings::Boolean look_at_target{ "autoheal.look-at-target", "false" };
static settings::Boolean friendsonly{ "autoheal.friends-only", "false" };
static settings::Boolean pop_uber_auto{ "autoheal.uber.enable", "true" };
static settings::Boolean pop_uber_voice{ "autoheal.popvoice", "true" };
static settings::Float pop_uber_percent{ "autoheal.uber.health-below-ratio", "0" };
static settings::Boolean share_uber{ "autoheal.uber.share", "false" };
static settings::Boolean heal_disguised{ "autoheal.disguised", "false" };

static settings::Boolean auto_vacc{ "autoheal.vacc.enable", "false" };

static settings::Int vacc_sniper{ "autoheal.vacc.bullet.sniper-pop", "1" };
static settings::Int vacc_sniper_fov{ "autoheal.vacc.bullet.sniper-fov", "20" };

static settings::Boolean auto_vacc_fire_checking{ "autoheal.vacc.fire.enable", "true" };
static settings::Int auto_vacc_pop_if_pyro{ "autoheal.vacc.fire.pyro-pop", "1" };
static settings::Boolean auto_vacc_check_on_fire{ "autoheal.vacc.fire.prevent-afterburn", "true" };
static settings::Int auto_vacc_pyro_range{ "autoheal.vacc.fire.pyro-range", "450" };

static settings::Boolean auto_vacc_blast_checking{ "autoheal.vacc.blast.enable", "true" };
static settings::Boolean auto_vacc_blast_crit_pop{ "autoheal.vacc.blast.crit-pop", "true" };
static settings::Int auto_vacc_blast_health{ "autoheal.vacc.blast.pop-near-rocket-health", "80" };
static settings::Int auto_vacc_proj_danger_range{ "autoheal.vacc.blast.danger-range", "650" };

static settings::Int change_timer{ "autoheal.vacc.reset-timer", "200" };

static settings::Int auto_vacc_bullet_pop_ubers{ "autoheal.vacc.bullet.min-charges", "0" };
static settings::Int auto_vacc_fire_pop_ubers{ "autoheal.vacc.fire.min-charges", "0" };
static settings::Int auto_vacc_blast_pop_ubers{ "autoheal.vacc.blast.min-charges", "0" };

static settings::Int default_resistance{ "autoheal.vacc.default-resistance", "0" };
static settings::Int steam_var{ "autoheal.steamid", "0" };

// Per class Heal Priorities, You should heal low hp classes earlier because they have an overall smaller healthpool
static settings::Int healp_scout{ "autoheal.priority-scout", "60" };
static settings::Int healp_soldier{ "autoheal.priority-soldier", "50" };
static settings::Int healp_pyro{ "autoheal.priority-pyro", "40" };
static settings::Int healp_demoman{ "autoheal.priority-demoman", "40" };
static settings::Int healp_heavy{ "autoheal.priority-heavy", "20" };
static settings::Int healp_engineer{ "autoheal.priority-engineer", "40" };
static settings::Int healp_medic{ "autoheal.priority-medic", "50" };
static settings::Int healp_sniper{ "autoheal.priority-sniper", "60" };
static settings::Int healp_spy{ "autoheal.priority-spy", "40" };

struct patient_data_s
{
    float last_damage{ 0.0f };
    int last_health{ 0 };
    int accum_damage{ 0 }; // accumulated damage over X seconds (data stored for AT least 5 seconds)
    float accum_damage_start{ 0.0f };
};

int CurrentHealingTargetIDX{ 0 };

int vaccinator_change_stage = 0;
int vaccinator_change_ticks = 0;
int vaccinator_ideal_resist = 0;
int vaccinator_change_timer = 0;

std::array<Timer, PLAYER_ARRAY_SIZE> reset_cd{};
std::vector<patient_data_s> data(PLAYER_ARRAY_SIZE);

struct proj_data_s
{
    int eid;
    Vector last_pos;
};
std::vector<proj_data_s> proj_data_array;

int ChargeCount()
{
    return (CE_FLOAT(LOCAL_W, netvar.m_flChargeLevel) / 0.25f);
}

// TODO Angle Checking
int BulletDangerValue(CachedEntity *patient)
{
    if (!vacc_sniper)
        return 0;
    bool any_zoomed_snipers = false;
    // Find dangerous snipers in other team
    for (auto const &ent : entity_cache::player_cache)
    {

        if (CE_BAD(ent))
            continue;
        if (!ent->m_bAlivePlayer() || !ent->m_bEnemy())
            continue;
        if (g_pPlayerResource->GetClass(ent) != tf_sniper)
            continue;
        if (!HasCondition<TFCond_Zoomed>(ent))
            continue;
        if (!player_tools::shouldTarget(ent))
            continue;
        any_zoomed_snipers = true;
        if (*vacc_sniper == 2)
        {
            // If vacc_sniper == 2 ("Any zoomed") then return 2
            // Why would you want this?????
            return 2;
        }
        else
        {
            if (IsEntityVisible(ent, head))
            {
                if (playerlist::AccessData(ent).state == playerlist::k_EState::RAGE)
                    return 2;
                else
                {
                    if (GetFov(ent->m_vecAngle(), ent->hitboxes.GetHitbox(head)->center, patient->hitboxes.GetHitbox(head)->center) < *vacc_sniper_fov)
                        return 2;
                }
            }
        }
    }
    return any_zoomed_snipers ? 1 : 0;
}

int FireDangerValue(CachedEntity *patient)
{
    // Find nearby pyros
    if (!auto_vacc_fire_checking)
        return 0;
    uint8_t should_switch = 0;
    if (auto_vacc_pop_if_pyro)
    {
        for (auto const &ent : entity_cache::player_cache)
        {

            if (CE_BAD(ent))
                continue;
            if (!ent->m_bEnemy())
                continue;
            if (!ent->m_bAlivePlayer())
                continue;
            if (!player_tools::shouldTarget(ent))
                continue;
            if (g_pPlayerResource->GetClass(ent) != tf_pyro)
                continue;
            if (patient->m_vecOrigin().DistTo(ent->m_vecOrigin()) > (int) auto_vacc_pyro_range)
                continue;
            if (*auto_vacc_pop_if_pyro == 2)
                return 2;
            CachedEntity *weapon = ENTITY(HandleToIDX(CE_INT(ent, netvar.hActiveWeapon)));
            if (CE_GOOD(weapon) && weapon->m_iClassID() == CL_CLASS(CTFFlameThrower))
            {
                if (HasCondition<TFCond_OnFire>(patient))
                    return 2;
                else
                    should_switch = 1;
            }
        }
    }
    if (*auto_vacc_check_on_fire && HasCondition<TFCond_OnFire>(patient))
    {
        if (patient->m_iHealth() < 35)
            return 2;
        else
            should_switch = 1;
    }
    return should_switch;
}

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
            if (patient->m_vecOrigin().DistToSqr(d.last_pos) > patient->m_vecOrigin().DistToSqr(ent->m_vecOrigin()))
            {
                if (ent->m_bCritProjectile())
                    hasCritRockets = true;
                hasRockets = true;
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
        if (patient->m_iHealth() < (int) auto_vacc_blast_health || (auto_vacc_blast_crit_pop && hasCritRockets))
        {
            return 2;
        }
        return 1;
    }
    // Find rockets/pipes nearby
    for (auto const &ent : entity_cache::valid_ents)
    {
        if (!ent->m_bEnemy())
            continue;
        if (ent->m_Type() != ENTITY_PROJECTILE)
            continue;
        if (ent->m_iClassID() == CL_CLASS(CTFProjectile_Flare))
            continue;
        if (patient->m_vecOrigin().DistTo(ent->m_vecOrigin()) > (int) auto_vacc_proj_danger_range)
            continue;
        proj_data_array.push_back(proj_data_s{ ent->m_IDX, ent->m_vecOrigin() });
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
    return (ent->m_iClassID() == CL_CLASS(CTFProjectile_Rocket) || ent->m_iClassID() == CL_CLASS(CTFProjectile_Flare) || ent->m_iClassID() == CL_CLASS(CTFProjectile_EnergyBall) || ent->m_iClassID() == CL_CLASS(CTFProjectile_HealingBolt) || ent->m_iClassID() == CL_CLASS(CTFProjectile_Arrow) || ent->m_iClassID() == CL_CLASS(CTFProjectile_SentryRocket) || ent->m_iClassID() == CL_CLASS(CTFProjectile_Cleaver) || ent->m_iClassID() == CL_CLASS(CTFGrenadePipebombProjectile) || ent->m_iClassID() == CL_CLASS(CTFProjectile_EnergyRing));
}

int NearbyEntities()
{
    int ret = 0;
    if (CE_BAD(LOCAL_E) || CE_BAD(LOCAL_W))
        return ret;
    for (auto const &ent : entity_cache::valid_ents)
    {

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
    int bd = BlastDangerValue(patient), fd = FireDangerValue(patient), hd = BulletDangerValue(patient);
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
        if (vaccinator_change_timer == 1 && *default_resistance)
        {
            SetResistance(*default_resistance - 1);
        }
        vaccinator_change_timer--;
    }
    else
        vaccinator_change_timer = *change_timer;
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

unsigned int steamid = 0;

static CatCommand heal_steamid("autoheal_heal_steamid", "Heals a player with SteamID",
                               [](const CCommand &args)
                               {
                                   if (args.ArgC() < 2)
                                   {
                                       logging::Info("Invalid call!");
                                       steam_var = 0;
                                       return;
                                   }
                                   if (strtol(args.Arg(1), nullptr, 10) == 0x0)
                                   {
                                       steam_var = 0;
                                       return;
                                   }
                                   try
                                   {
                                       steam_var = std::stoul(args.Arg(1));
                                   }
                                   catch (const std::invalid_argument &)
                                   {
                                       logging::Info("Invalid steamid! Setting current to null.");
                                       steam_var = 0;
                                   }
                               });

static CatCommand vaccinator_bullet("vacc_bullet", "Bullet Vaccinator", []() { SetResistance(0); });
static CatCommand vaccinator_blast("vacc_blast", "Blast Vaccinator", []() { SetResistance(1); });
static CatCommand vaccinator_fire("vacc_fire", "Fire Vaccinator", []() { SetResistance(2); });

bool IsPopped()
{
    CachedEntity *weapon = g_pLocalPlayer->weapon();
    if (CE_BAD(weapon) || weapon->m_iClassID() != CL_CLASS(CWeaponMedigun))
        return false;
    return CE_BYTE(weapon, netvar.bChargeRelease);
}

bool ShouldChargePlayer(int idx)
{
    CachedEntity *target              = ENTITY(idx);
    const float damage_accum_duration = g_GlobalVars->curtime - data[idx].accum_damage_start;
    const int health                  = target->m_iHealth();

    if (health > g_pPlayerResource->GetMaxHealth(target))
        return false;
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
    if (CurrentHealingTargetIDX != -1)
    {
        CachedEntity *target = ENTITY(CurrentHealingTargetIDX);
        if (CE_GOOD(target))
        {
            if (ShouldChargePlayer(CurrentHealingTargetIDX))
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

void UpdateData()
{
    for (auto const &ent : entity_cache::player_cache)
    {
        int i = ent->m_IDX;
        if (reset_cd[i].test_and_set(10000))
            data[i] = {};

        if (CE_GOOD(ent) && ent->m_bAlivePlayer())
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
            if (health != last_health && health <= g_pPlayerResource->GetMaxHealth(ent))
            {
                reset_cd[i].update();
                data[i].last_health = health;
                if (health < last_health)
                {
                    data[i].accum_damage += (last_health - health);
                    if (!data[i].accum_damage_start)
                        data[i].accum_damage_start = g_GlobalVars->curtime;
                    data[i].last_damage = g_GlobalVars->curtime;
                }
            }
        }
    }
}

bool CanHeal(int idx)
{
    // Already healing, no need to check
    if (idx == HandleToIDX(CE_INT(LOCAL_W, netvar.m_hHealingTarget)))
        return true;

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
    if (friendsonly && !playerlist::IsFriend(ent))
        return false;
    if (!heal_disguised && IsPlayerDisguised(ent))
        return false;
    return true;
}

static bool inited_class;
// we'll use this to shrink the code down immensely
static std::array<settings::Int *, 9> class_list;
int HealingPriority(int idx)
{
    if (!CanHeal(idx))
        return -1;
    if (!inited_class)
    {
        class_list[tf_scout - 1]    = &healp_scout;
        class_list[tf_soldier - 1]  = &healp_soldier;
        class_list[tf_pyro - 1]     = &healp_pyro;
        class_list[tf_demoman - 1]  = &healp_demoman;
        class_list[tf_heavy - 1]    = &healp_heavy;
        class_list[tf_engineer - 1] = &healp_engineer;
        class_list[tf_medic - 1]    = &healp_medic;
        class_list[tf_sniper - 1]   = &healp_sniper;
        class_list[tf_spy - 1]      = &healp_spy;
        inited_class                = true;
    }

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
    // Base Class priority
    priority += hacks::shared::followbot::ClassPriority(ent) * 1.3;

    // wait that's illegal
    if (g_pPlayerResource->GetClass(ent) == 0)
        return 0.0f;
    // Healthpoint priority
    float healpp = **class_list[g_pPlayerResource->GetClass(ent) - 1];
    switch (playerlist::AccessData(ent).state)
    {
    case playerlist::k_EState::PARTY:
    case playerlist::k_EState::FRIEND:
        // "Normal" default is 40 while friend default is 70, 70 = 40 * (1 + 3/4)
        priority += healpp * (1 + 3 / 4) * (1 - healthp);
        // 5 is default here, in this case just /8
        priority += healpp / 8 * (1 - overhealp);
        break;
    case playerlist::k_EState::IPC:
        // "Normal" default is 40 while IPC default is 100, 100 = 40 * 2.5
        priority += healpp * 2.5f * (1 - healthp);
        // 10 is default here, so it's simply /4
        priority += healpp / 4 * (1 - overhealp);
        break;
    default:
        priority += healpp * (1 - healthp);
        // 3 is default here, in this case it's ~40/13
        priority += healpp / 3 * (1 - overhealp);
    }
#if ENABLE_IPC
    if (ipc::peer)
    {
        if (hacks::shared::followbot::isEnabled() && hacks::shared::followbot::follow_target == idx)
        {
            priority *= 6.0f;
        }
    }
#endif
    /*    player_info_s info;
        GetPlayerInfo(idx, &info);
        info.name[31] = 0;
        if (strcasestr(info.name, ignore.GetString()))
            priority = 0.0f;*/
    return priority;
}

int BestTarget()
{
    int best       = 0;
    int best_score = INT_MIN;
    if (steamid_only)
        return best;
    for (auto const &ent : entity_cache::player_cache)
    {
        int score = HealingPriority(ent->m_IDX);
        if (score > best_score && score != -1)
        {
            best       = ent->m_IDX;
            best_score = score;
        }
    }
    return best;
}

static void CreateMove()
{
    if (CE_BAD(LOCAL_W))
        return;

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
    if ((!steamid && !enable) || GetWeaponMode() != weapon_medigun)
        return;
    bool healing_steamid = false;
    if (steamid)
    {
        unsigned int current_id = 0;
        if (CurrentHealingTargetIDX)
        {
            CachedEntity *current_ent = ENTITY(CurrentHealingTargetIDX);
            if (CE_GOOD(current_ent))
                current_id = current_ent->player_info->friendsID;
        }
        if (current_id != steamid)
        {
            for (auto const &ent : entity_cache::player_cache)
            {
                int i = ent->m_IDX;
                if (!ent->player_info->friendsID)
                    continue;
                if (ent->player_info->friendsID == steamid && CanHeal(i))
                {
                    CurrentHealingTargetIDX = i;
                    healing_steamid         = true;
                    break;
                }
            }
        }
        else
        {
            healing_steamid = true;
        }
    }

    if (CurrentHealingTargetIDX && (CE_BAD(ENTITY(CurrentHealingTargetIDX)) || !CanHeal(CurrentHealingTargetIDX)))
        CurrentHealingTargetIDX = 0;

    if (enable)
    {
        // if no target or after 2 seconds, pick new target
        if (!CurrentHealingTargetIDX || ((g_GlobalVars->tickcount % 132) == 0 && !healing_steamid))
        {
            CurrentHealingTargetIDX = BestTarget();
        }
    }

    UpdateData();

    if (!CurrentHealingTargetIDX)
        return;

    CachedEntity *target = ENTITY(CurrentHealingTargetIDX);

    bool target_is_healing_target = HandleToIDX(CE_INT(LOCAL_W, netvar.m_hHealingTarget)) == CurrentHealingTargetIDX;
    auto out                      = target->hitboxes.GetHitbox(spine_2);
    if (out)
    {
        if (silent)
            g_pLocalPlayer->bUseSilentAngles = true;

        // Follow the target using slowaim
        if (target_is_healing_target && look_at_target)
        {
            Vector angles = GetAimAtAngles(g_pLocalPlayer->v_Eye, out->center);
            hacks::tf2::misc_aimbot::DoSlowAim(angles);
            current_user_cmd->viewangles = angles;
        }

        // Set angles to new target
        if (!target_is_healing_target)
            current_user_cmd->viewangles = GetAimAtAngles(g_pLocalPlayer->v_Eye, out->center);
    }

    int autoheal_mode = g_ICvar->FindVar("tf_medigun_autoheal")->GetInt();
    // Hold down if we are currently healing our target or not healing anyone
    if (autoheal_mode == 0 && (target_is_healing_target || CE_INT(LOCAL_W, netvar.m_hHealingTarget) == -1))
        current_user_cmd->buttons |= IN_ATTACK;

    // Press once if we are not healing our target
    else if (autoheal_mode == 1 && !target_is_healing_target)
        current_user_cmd->buttons |= IN_ATTACK;

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

    // Uber on "CHARGE ME DOCTOR!"
    if (pop_uber_voice)
    {
        // Hey you wanna get deleted?
        auto uber_array = std::move(called_medic);
        for (auto i : uber_array)
            if (i == CurrentHealingTargetIDX)
                current_user_cmd->buttons |= IN_ATTACK2;
    }
}

void rvarCallback(settings::VariableBase<int> &var, int after)
{
    if (!after)
    {
        steamid = 0;
        return;
    }
    steamid = after;
}

// void LevelInit(){}

static InitRoutine Init(
    []()
    {
        steam_var.installChangeCallback(rvarCallback);
        EC::Register(EC::CreateMove, CreateMove, "autoheal", EC::average);
        // EC::Register(EC::LevelInit, LevelInit, "autoheal_lvlinit", EC::average);
    });
} // namespace hacks::tf::autoheal
