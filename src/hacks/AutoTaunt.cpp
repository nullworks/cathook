/*
 * AutoTaunt.cpp
 *
 *  Created on: Jul 27, 2017
 *      Author: nullifiedcat
 */

#include <settings/Float.hpp>
#include "common.hpp"
#include "hack.hpp"
#include "PlayerTools.hpp"

namespace hacks::tf::autotaunt
{
static settings::Boolean enable{ "autotaunt.enable", "false" };
static settings::Float chance{ "autotaunt.chance", "100" };
static settings::Float safety{ "autotaunt.safety-distance", "1000" };
static settings::Int switch_weapon{ "autotaunt.auto-weapon", "0" };

enum slots
{
    primary      = 1,
    secondary    = 2,
    melee        = 3,
    pda          = 4,
    disguise     = 4,
    destruct_pda = 5
};
static bool in_taunt = false;
static int prev_slot = -1;
static Timer taunt_t{};
class AutoTauntListener : public IGameEventListener2
{
public:
    virtual void FireGameEvent(IGameEvent *event)
    {
        if (!enable)
        {
            return;
        }
        if (GetPlayerForUserID(event->GetInt("attacker")) == g_IEngine->GetLocalPlayer())
        {
            bool nearby = false;
            for (auto const &ent : entity_cache::valid_ents)
            {
                if (CE_VALID(ent) && (ent->m_Type() == ENTITY_PLAYER || ent->m_iClassID() == CL_CLASS(CObjectSentrygun)) && ent->m_bEnemy() && ent->m_bAlivePlayer())
                {
                    if (!player_tools::shouldTarget(ent))
                        continue;
                    if (ent->m_vecDormantOrigin() && ent->m_vecDormantOrigin()->DistTo(LOCAL_E->m_vecOrigin()) < *safety)
                    {
                        nearby = true;
                        break;
                    }
                }
            }
            if (!nearby && RandomFloat(0, 100) <= float(chance))
            {
                if (switch_weapon)
                {
                    if (CE_GOOD(LOCAL_E) && CE_GOOD(LOCAL_W))
                    {
                        IClientEntity *weapon = RAW_ENT(LOCAL_W);
                        // IsBaseCombatWeapon()
                        if (re::C_BaseCombatWeapon::IsBaseCombatWeapon(weapon))
                            prev_slot = re::C_BaseCombatWeapon::GetSlot(weapon);
                        int new_slot = *switch_weapon;
                        if (new_slot == disguise && g_pLocalPlayer->clazz != tf_spy && g_pLocalPlayer->clazz != tf_engineer)
                            new_slot = primary;
                        if (new_slot == destruct_pda && g_pLocalPlayer->clazz != tf_engineer)
                            new_slot = primary;
                        hack::ExecuteCommand(format("slot", new_slot));
                        taunt_t.update();
                    }
                }
            }
        }
    }
};

AutoTauntListener listener;

InitRoutine init(
    []()
    {
        g_IEventManager2->AddListener(&listener, "player_death", false);
        EC::Register(
            EC::Shutdown, []() { g_IEventManager2->RemoveListener(&listener); }, "Shutdown_Autotaunt");
        EC::Register(
            EC::CreateMove,
            []()
            {
                if (prev_slot != -1 && CE_GOOD(LOCAL_E) && CE_GOOD(LOCAL_W) && LOCAL_E->m_bAlivePlayer() && taunt_t.test_and_set(100))
                {
                    if (in_taunt)
                    {
                        if (!HasCondition<TFCond_Taunting>(LOCAL_E))
                        {
                            hack::ExecuteCommand(format("slot", prev_slot + 1));
                            prev_slot = -1;
                            in_taunt  = false;
                        }
                        else
                            taunt_t.update();
                    }
                    else
                    {
                        hack::ExecuteCommand("taunt");
                        in_taunt = true;
                        taunt_t.update();
                    }
                }
            },
            "Autotaunt_CM");
    });
} // namespace hacks::tf::autotaunt
