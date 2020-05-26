/*
 * Made on 23rd of August 2020
 * Author: BenCat07
 */

#include "common.hpp"
#include "crits.hpp"

namespace hacks::tf2::nospread
{
static settings::Boolean enabled("aimbot.no-spread", "false");
static bool should_nospread = false;
void CreateMove()
{
    if (CE_BAD(LOCAL_E) || CE_BAD(LOCAL_W))
        return;

    if (!enabled)
        return;

    // Credits to https://www.unknowncheats.me/forum/team-fortress-2-a/139094-projectile-nospread.html

    // Set up Random Seed
    int cmd_num = current_user_cmd->command_number;
    // Crithack uses different things
    if (criticals::isEnabled() && g_pLocalPlayer->weapon_mode != weapon_melee && criticals::crit_cmds.find(LOCAL_W->m_IDX) != criticals::crit_cmds.end() && criticals::crit_cmds.find(LOCAL_W->m_IDX)->second.size())
    {
        int array_index = criticals::current_index;
        if (array_index >= criticals::crit_cmds.at(LOCAL_W->m_IDX).size())
            array_index = 0;
        // Adjust for nospread
        cmd_num = criticals::crit_cmds.at(LOCAL_W->m_IDX).at(array_index);
    }
    RandomSeed(MD5_PseudoRandom(cmd_num) & 0x7FFFFFFF);
    SharedRandomInt(MD5_PseudoRandom(cmd_num) & 0x7FFFFFFF, "SelectWeightedSequence", 0, 0, 0);
    for (int i = 0; i < 6; ++i)
        RandomFloat();

    // Projectile/Huntsman check
    if (g_pLocalPlayer->weapon_mode != weapon_projectile && LOCAL_W->m_iClassID() != CL_CLASS(CTFCompoundBow))
        return;

    // Beggars check
    if (CE_INT(LOCAL_W, netvar.iItemDefinitionIndex) == 730)
    {
        // Check if we released the barrage by releasing m1, also lock bool so people don't just release m1 and tap it again
        if (!should_nospread)
            should_nospread = !(current_user_cmd->buttons & IN_ATTACK) && g_pLocalPlayer->bAttackLastTick;

        if (!CE_INT(LOCAL_W, netvar.m_iClip1) && CE_INT(LOCAL_W, netvar.iReloadMode) == 0)
        {
            // Reset
            should_nospread = false;
            return;
        }
        // Cancel
        if (!should_nospread)
            return;
    }
    // Huntsman check
    else if (LOCAL_W->m_iClassID() == CL_CLASS(CTFCompoundBow))
    {
        if (!g_pLocalPlayer->bAttackLastTick || (current_user_cmd->buttons & IN_ATTACK))
            return;
    }
    // Rest of weapons
    else if (!(current_user_cmd->buttons & IN_ATTACK))
        return;

    switch (LOCAL_W->m_iClassID())
    {
    case CL_CLASS(CTFSyringeGun):
    {
        float spread = 1.5f;
        current_user_cmd->viewangles.x -= RandomFloat(-spread, spread);
        current_user_cmd->viewangles.y -= RandomFloat(-spread, spread);
        fClampAngle(current_user_cmd->viewangles);
        g_pLocalPlayer->bUseSilentAngles = true;
        break;
    }
    case CL_CLASS(CTFCompoundBow):
    {
        Vector view = current_user_cmd->viewangles;
        if (g_pLocalPlayer->bUseSilentAngles)
            view = g_pLocalPlayer->v_OrigViewangles;

        Vector spread;
        Vector src;

        re::C_TFWeaponBase::GetProjectileFireSetupHuntsman(RAW_ENT(LOCAL_W), RAW_ENT(LOCAL_E), Vector(23.5f, -8.f, 8.f), &src, &spread, false, 2000.0f);

        spread -= view;
        current_user_cmd->viewangles -= spread;
        g_pLocalPlayer->bUseSilentAngles = true;
        fClampAngle(current_user_cmd->viewangles);
        break;
    }
    default:
        Vector view = current_user_cmd->viewangles;
        if (g_pLocalPlayer->bUseSilentAngles)
            view = g_pLocalPlayer->v_OrigViewangles;

        Vector spread = re::C_TFWeaponBase::GetSpreadAngles(RAW_ENT(LOCAL_W));

        spread -= view;
        current_user_cmd->viewangles -= spread;
        g_pLocalPlayer->bUseSilentAngles = true;
        fClampAngle(current_user_cmd->viewangles);
        break;
    }
}

static InitRoutine init([]() { EC::Register(EC::CreateMove, CreateMove, "nospread_cm", EC::very_late); });
} // namespace hacks::tf2::nospread
