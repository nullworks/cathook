//
// Created by bencat07 on 28.09.18.
//

#include "common.hpp"
#include "settings/Bool.hpp"
#include "settings/Int.hpp"
#include "settings/Key.hpp"
#include "PlayerTools.hpp"
#include "hacks/Trigger.hpp"
#include "hacks/Achievement.hpp"

namespace hacks::tf2::misc_aimbot
{
static settings::Boolean sandwichaim_enabled{ "sandwichaim.enable", "false" };
static settings::Button sandwichaim_aimkey{ "sandwichaim.aimkey", "<null>" };
static settings::Int sandwichaim_aimkey_mode{ "sandwichaim.aimkey-mode", "0" };

float sandwich_speed = 350.0f;
float grav           = 0.25f;
int prevent          = -1;

std::pair<CachedEntity *, Vector> FindBestEnt(bool teammate, bool Predict, bool zcheck)
{
    CachedEntity *bestent = nullptr;
    float bestscr         = FLT_MAX;
    Vector predicted{};
    for (int i = 0; i < 1; i++)
    {
        if (prevent != -1)
        {
            auto ent = ENTITY(prevent);
            if (CE_BAD(ent) || !ent->m_bAlivePlayer() || (teammate && ent->m_iTeam() != LOCAL_E->m_iTeam()) || ent == LOCAL_E)
                continue;
            if (!teammate && ent->m_iTeam() == LOCAL_E->m_ItemType())
                continue;
            if (!ent->hitboxes.GetHitbox(1))
                continue;
            if (!teammate && !player_tools::shouldTarget(ent))
                continue;
            Vector target{};
            if (Predict)
                target = ProjectilePrediction(ent, 1, sandwich_speed, grav, PlayerGravityMod(ent));
            else
                target = ent->hitboxes.GetHitbox(1)->center;
            if (!IsEntityVectorVisible(ent, target))
                continue;
            if (zcheck && (ent->m_vecOrigin().z - LOCAL_E->m_vecOrigin().z) > 80.0f)
                continue;
            float scr = ent->m_flDistance();
            if (g_pPlayerResource->GetClass(ent) == tf_medic)
                scr *= 0.1f;
            if (scr < bestscr)
            {
                bestent   = ent;
                predicted = target;
                bestscr   = scr;
                prevent   = ent->m_IDX;
            }
        }
        if (bestent && predicted.z)
            return { bestent, predicted };
    }
    prevent = -1;
    for (int i = 0; i <= g_IEngine->GetMaxClients(); i++)
    {
        CachedEntity *ent = ENTITY(i);
        if (CE_BAD(ent) || !(ent->m_bAlivePlayer()) || (teammate && ent->m_iTeam() != LOCAL_E->m_iTeam()) || ent == LOCAL_E)
            continue;
        if (!teammate && ent->m_iTeam() == LOCAL_E->m_iTeam())
            continue;
        if (!ent->hitboxes.GetHitbox(1))
            continue;
        Vector target{};
        if (Predict)
            target = ProjectilePrediction(ent, 1, sandwich_speed, grav, PlayerGravityMod(ent));
        else
            target = ent->hitboxes.GetHitbox(1)->center;
        if (!IsEntityVectorVisible(ent, target))
            continue;
        if (zcheck && (ent->m_vecOrigin().z - LOCAL_E->m_vecOrigin().z) > 80.0f)
            continue;
        float scr = ent->m_flDistance();
        if (g_pPlayerResource->GetClass(ent) == tf_medic)
            scr *= 0.1f;
        if (scr < bestscr)
        {
            bestent   = ent;
            predicted = target;
            bestscr   = scr;
            prevent   = ent->m_IDX;
        }
    }
    return { bestent, predicted };
}
static float slow_change_dist_y{};
static float slow_change_dist_p{};
void DoSlowAim(Vector &input_angle)
{

    auto viewangles = current_user_cmd->viewangles;

    // Yaw
    if (viewangles.y != input_angle.y)
    {

        // Check if input angle and user angle are on opposing sides of yaw so
        // we can correct for that
        bool slow_opposing = false;
        if ((input_angle.y < -90 && viewangles.y > 90) || (input_angle.y > 90 && viewangles.y < -90))
            slow_opposing = true;

        // Direction
        bool slow_dir = false;
        if (slow_opposing)
        {
            if (input_angle.y > 90 && viewangles.y < -90)
                slow_dir = true;
        }
        else if (viewangles.y > input_angle.y)
            slow_dir = true;

        // Speed, check if opposing. We dont get a new distance due to the
        // opposing sides making the distance spike, so just cheap out and reuse
        // our last one.
        if (!slow_opposing)
            slow_change_dist_y = std::abs(viewangles.y - input_angle.y) / 5;

        // Move in the direction of the input angle
        if (slow_dir)
            input_angle.y = viewangles.y - slow_change_dist_y;
        else
            input_angle.y = viewangles.y + slow_change_dist_y;
    }

    // Pitch
    if (viewangles.x != input_angle.x)
    {
        // Get speed
        slow_change_dist_p = std::abs(viewangles.x - input_angle.x) / 5;

        // Move in the direction of the input angle
        if (viewangles.x > input_angle.x)
            input_angle.x = viewangles.x - slow_change_dist_p;
        else
            input_angle.x = viewangles.x + slow_change_dist_p;
    }

    // Clamp as we changed angles
    fClampAngle(input_angle);
}

static void SandwichAim()
{
    if (!*sandwichaim_enabled)
        return;
    if (CE_BAD(LOCAL_E) || !LOCAL_E->m_bAlivePlayer() || CE_BAD(LOCAL_W))
        return;
    if (sandwichaim_aimkey)
    {
        switch (*sandwichaim_aimkey_mode)
        {
        case 1:
            if (!sandwichaim_aimkey.isKeyDown())
                return;
            break;
        case 2:
            if (sandwichaim_aimkey.isKeyDown())
                return;
            break;
        default:
            break;
        }
    }
    if (LOCAL_W->m_iClassID() != CL_CLASS(CTFLunchBox))
        return;
    Vector Predict;
    CachedEntity *bestent = nullptr;
    std::pair<CachedEntity *, Vector> result{};
    result  = FindBestEnt(true, true, false);
    bestent = result.first;
    Predict = result.second;
    if (bestent)
    {
        Vector tr = Predict - g_pLocalPlayer->v_Eye;
        Vector angles;
        VectorAngles(tr, angles);
        // Clamping is important
        fClampAngle(angles);
        current_user_cmd->viewangles = angles;
        current_user_cmd->buttons |= IN_ATTACK2;
        g_pLocalPlayer->bUseSilentAngles = true;
    }
}

static bool charge_aimbotted = false;
static settings::Boolean charge_aim{ "chargeaim.enable", "false" };
static settings::Button charge_key{ "chargeaim.key", "<null>" };

static void ChargeAimbot()
{
    charge_aimbotted = false;
    if (!*charge_aim)
        return;
    if (charge_key && !charge_key.isKeyDown())
        return;
    if (CE_BAD(LOCAL_E) || !LOCAL_E->m_bAlivePlayer() || CE_BAD(LOCAL_W))
        return;
    if (!HasCondition<TFCond_Charging>(LOCAL_E))
        return;
    std::pair<CachedEntity *, Vector> result{};
    result                = FindBestEnt(false, false, true);
    CachedEntity *bestent = result.first;
    if (bestent && result.second.IsValid())
    {
        Vector tr = result.second - g_pLocalPlayer->v_Eye;
        Vector angles;
        VectorAngles(tr, angles);
        // Clamping is important
        fClampAngle(angles);
        DoSlowAim(angles);
        current_user_cmd->viewangles = angles;
        charge_aimbotted             = true;
    }
}

static settings::Boolean autosapper_enabled("autosapper.enabled", "false");
static settings::Boolean autosapper_silent("autosapper.silent", "true");

static void SapperAimbot()
{
    if (!autosapper_enabled)
        return;
    if (CE_BAD(LOCAL_E) || CE_BAD(LOCAL_W) || !g_pLocalPlayer->holding_sapper)
        return;

    CachedEntity *target = nullptr;
    float distance       = FLT_MAX;

    for (int i = 0; i < entity_cache::max; i++)
    {
        CachedEntity *ent = ENTITY(i);
        if (CE_BAD(ent))
            continue;
        if (ent->m_Type() != ENTITY_BUILDING)
            continue;
        if (!ent->m_bEnemy())
            continue;
        if (CE_BYTE(ent, netvar.m_bHasSapper))
            continue;
        float new_distance = g_pLocalPlayer->v_Eye.DistTo(GetBuildingPosition(ent));
        if (distance <= new_distance)
            continue;

        distance = new_distance;
        target   = ent;
    }
    if (target)
    {
        float range    = re::C_TFWeaponBaseMelee::GetSwingRange(RAW_ENT(LOCAL_W));
        Vector angle   = GetAimAtAngles(g_pLocalPlayer->v_Eye, GetBuildingPosition(target));
        Vector forward = GetForwardVector(g_pLocalPlayer->v_Eye, angle, range);
        trace_t trace;
        if (IsEntityVectorVisible(target, forward, MASK_SHOT, &trace))
        {
            if (trace.DidHit() && (IClientEntity *) trace.m_pEnt == RAW_ENT(target))
            {
                current_user_cmd->viewangles = angle;
                if (autosapper_silent)
                    g_pLocalPlayer->bUseSilentAngles = true;
                current_user_cmd->buttons |= IN_ATTACK;
            }
        }
    }
}
// auto repair
static settings::Int autorepair_priority{ "autorepair.priority", "0" };
static settings::Boolean autorepair_enabled("autorepair.enabled", "false");
static settings::Boolean autorepair_silent("autorepair.silent", "true");
static settings::Boolean autorepair_autoequip("autorepair.autoequip", "false");
static settings::Boolean autorepair_repair_sentry("autorepair.sentry", "false");
static settings::Boolean autorepair_repair_dispenser("autorepair.dispenser", "false");
static settings::Boolean autorepair_repair_teleport("autorepair.teleport", "false");
// auto upgrade
static settings::Boolean autoupgrade_enabled("autoupgrade.enabled", "false");
static settings::Boolean autoupgrade_sentry("autoupgrade.sentry", "false");
static settings::Boolean autoupgrade_dispenser("autoupgrade.dispenser", "false");
static settings::Boolean autoupgrade_teleport("autoupgrade.teleport", "false");

static void RepairAimbot()
{
    if (!autorepair_enabled)
        return;

    if (CE_BAD(LOCAL_E) || CE_BAD(LOCAL_W))
        return;

    CachedEntity *target = nullptr;
    float distance       = FLT_MAX;
    int cur_ammo         = CE_INT(LOCAL_E, netvar.m_iAmmo + 12);

    if ( cur_ammo == 0 )
        return;

    if ( g_pLocalPlayer->clazz != tf_engineer )
        return;

    if ( GetWeaponMode() != weaponmode::weapon_melee )
        return;

    for ( int i = 0; i < entity_cache::max; i++ ) // loop
    {
        CachedEntity *ent = ENTITY(i);

        if ( CE_BAD(ent) )
            continue;

        if ( ent->m_Type() != ENTITY_BUILDING )
            continue;

        if ( ent->m_bEnemy() )
            continue;

        float new_distance = g_pLocalPlayer->v_Eye.DistTo(GetBuildingPosition(ent));

        if (distance <= new_distance)
            continue;

        //if ( ent->m_iHealth() == ent->m_iMaxHealth() )
            //continue;

        auto id = ent->m_iClassID();

        float wrench_range = re::C_TFWeaponBaseMelee::GetSwingRange(RAW_ENT(LOCAL_W));
        wrench_range += (float)50.f;

        switch ( id ) {
        case CL_CLASS( CObjectSentrygun ): { // start

            if ( !autorepair_repair_sentry )
                break;

            static bool sentry_need_repair = false;

            int sentry_health_percentage = ent->m_iHealth() / ent->m_iMaxHealth();

            float s_distance = ent->m_vecOrigin().DistTo(LOCAL_E->m_vecOrigin());

            if (ent->m_iHealth() == ent->m_iMaxHealth())
                break;

            if (s_distance > wrench_range)
                break;

            if (ent->m_iHealth() < ent->m_iMaxHealth())
            {
                sentry_need_repair = true;
                target = ent;
            }
            else
            {
                sentry_need_repair = false;
                target = nullptr;
            }

            if (!sentry_need_repair)
                break;

        } // end
        case CL_CLASS(CObjectDispenser): { // start

            if ( !autorepair_repair_dispenser )
                break;

            static bool dispenser_need_repair = false;

            int dispenser_health_percentage = ent->m_iHealth() / ent->m_iMaxHealth();
            float d_distance = ent->m_vecOrigin().DistTo(LOCAL_E->m_vecOrigin());

            if ( ent->m_iHealth() == ent->m_iMaxHealth() )
                break;

            if ( d_distance > wrench_range )
                break;

            if ( ent->m_iHealth() < ent->m_iMaxHealth() )
            {
                dispenser_need_repair = true;
                target = ent;
            }
            else
            {
                dispenser_need_repair = false;
                target = nullptr;
            }

            if( dispenser_need_repair == false)
                break;

        } // end
        case CL_CLASS(CObjectTeleporter): { // start

            if ( !autorepair_repair_teleport )
                break;;

            static bool teleport_need_repair = false;

            int teleport_health_percentage = ent->m_iHealth() / ent->m_iMaxHealth();
            float t_distance = ent->m_vecOrigin().DistTo(LOCAL_E->m_vecOrigin());

            if ( ent->m_iHealth() == ent->m_iMaxHealth() )
                break;

            if (t_distance > wrench_range)
                break;

            if (ent->m_iHealth() < ent->m_iMaxHealth() )
            {
                teleport_need_repair = true;
                target = ent;
            }
            else
            {
                teleport_need_repair = false;
                target = nullptr;
            }

            if ( teleport_need_repair == false )
                break;

        } // end

        default:
            continue;
      } // end

        distance = new_distance;
        //target   = ent;

    } // end of loop



    if (target) // target is set
    {
        float range    = re::C_TFWeaponBaseMelee::GetSwingRange(RAW_ENT(LOCAL_W));
        range         += (float)50.f;

        Vector angle   = GetAimAtAngles(g_pLocalPlayer->v_Eye, GetBuildingPosition(target));
        Vector forward = GetForwardVector(g_pLocalPlayer->v_Eye, angle, range);

        trace_t trace;

        if (IsEntityVectorVisible(target, forward, MASK_SHOT, &trace))
        {
            if (trace.DidHit() && (IClientEntity *) trace.m_pEnt == RAW_ENT(target))
            {
                current_user_cmd->viewangles = angle;
                if (autorepair_silent)
                    g_pLocalPlayer->bUseSilentAngles = true;
                current_user_cmd->buttons |= IN_ATTACK;
            }
        }
    }
}

static void CreateMove()
{
    SandwichAim();
    ChargeAimbot();
    SapperAimbot();
    RepairAimbot();
}

static InitRoutine init([]() {
    EC::Register(EC::CreateMove, CreateMove, "cm_miscaimbot", EC::late);
    static BytePatch patch(gSignatures.GetClientSignature, "75 16 F3 0F 10 45", 0x0, { 0x90, 0x90 });
    patch.Patch();
});
} // namespace hacks::tf2::misc_aimbot
