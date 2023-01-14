
#include "HookedMethods.hpp"
#include "WeaponData.hpp"

namespace hooked_methods
{
int last_tick   = 0;
int last_weapon = 0;

// Credits to blackfire for telling me to do this :)
DEFINE_HOOKED_METHOD(RunCommand, void, IPrediction *prediction, IClientEntity *entity, CUserCmd *usercmd, IMoveHelper *move)
{
    if (CE_GOOD(LOCAL_E) && CE_GOOD(LOCAL_W) && entity && entity->entindex() == g_pLocalPlayer->entity_idx && usercmd && usercmd->command_number)
    {
        original::RunCommand(prediction, entity, usercmd, move);
        criticals::fixBucket(RAW_ENT(LOCAL_W), usercmd);
    }
    else
        return original::RunCommand(prediction, entity, usercmd, move);
}

static boost::unordered_flat_map<int, int> previous_ammo;

// Also fix heavy M2 causing bucket to fill faster, same for pyro
DEFINE_HOOKED_METHOD(CalcIsAttackCriticalHelper_brokenweps, bool, IClientEntity *ent)
{
    if (CE_GOOD(LOCAL_E) && CE_GOOD(LOCAL_W) && ent && re::C_TFWeaponBase::GetOwnerViaInterface(ent) == LOCAL_E->InternalEntity() && !criticals::calling_crithelper)
    {
        auto current_ammo = CE_INT(LOCAL_E, netvar.m_iAmmo + 4);
        if (previous_ammo[ent->entindex()] == current_ammo)
        {
            weapon_info info(ent);
            auto ret = original::CalcIsAttackCriticalHelper_brokenweps(ent);
            info.restore_data(ent);
            return ret;
        }
        previous_ammo[ent->entindex()] = current_ammo;
    }

    if (LOCAL_W->m_iClassID() == CL_CLASS(CTFMinigun))
    {
        int weapon_mode     = NET_INT(ent, 0xb08);
        NET_INT(ent, 0xb08) = 0;
        auto ret            = original::CalcIsAttackCriticalHelper_brokenweps(ent);
        NET_INT(ent, 0xb08) = weapon_mode;
        return ret;
    }
    else
        return original::CalcIsAttackCriticalHelper_brokenweps(ent);
}

static hooks::VMTHook minigun_hook{};
static Timer minigun_check_timer{};
static InitRoutine minigun_check(
    []()
    {
        EC::Register(
            EC::CreateMove,
            []()
            {
                if (CE_BAD(LOCAL_E) || HasCondition<TFCond_HalloweenGhostMode>(LOCAL_E) || !LOCAL_E->m_bAlivePlayer() || !minigun_check_timer.test_and_set(1000))
                    return;
                // Grab the handle and store it into the var
                int *hWeapons = &CE_INT(LOCAL_E, netvar.hMyWeapons);
                if (!hWeapons)
                    return;
                // Go through the handle array and search for the item
                for (int i = 0; hWeapons[i]; i++)
                {
                    if (IDX_BAD(HandleToIDX(hWeapons[i])))
                        continue;
                    // Get the weapon
                    CachedEntity *weapon = ENTITY(HandleToIDX(hWeapons[i]));
                    // if weapon is what we are looking for, hook and move on
                    if (CE_VALID(weapon) && (weapon->m_iClassID() == CL_CLASS(CTFMinigun) || weapon->m_iClassID() == CL_CLASS(CTFFlameThrower)) && !minigun_hook.IsHooked(weapon->InternalEntity()))
                    {
                        logging::Info("Found and hooked Minigun/Flamethrower!");
                        minigun_hook.Set(weapon->InternalEntity());
                        minigun_hook.HookMethod(HOOK_ARGS(CalcIsAttackCriticalHelper_brokenweps));
                        minigun_hook.Apply();
                        break;
                    }
                }
            },
            "cm_runcommand");
    });

} // namespace hooked_methods
