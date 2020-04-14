/*
  Created on 29.07.18.
*/

#include <hacks/Thirdperson.hpp>
#include <settings/Bool.hpp>
#include <localplayer.hpp>
#include <entitycache.hpp>
#include <core/sdk.hpp>
#include "AntiAntiAim.hpp"

namespace hacks::tf::thirdperson
{
static settings::Boolean enable{ "visual.thirdperson.enable", "false" };
static settings::Button thirdperson_key{ "visual.thirdperson-button", "<null>" };
static settings::Boolean real_angles{ "visual.thirdperson.real-angles", "false" };
static settings::Boolean disable_zoomed{ "visual.thirdperson.disable-zoomed", "false" };
static bool was_enabled{ false };

void frameStageNotify()
{
    if (CE_BAD(LOCAL_E))
        return;

    static bool tp_key_down{ false };
    if (thirdperson_key && thirdperson_key.isKeyDown() && !tp_key_down)
    {
        enable      = !enable;
        tp_key_down = true;
    }
    if (thirdperson_key && !thirdperson_key.isKeyDown() && tp_key_down)
    {
        tp_key_down = false;
    }

    static bool was_tp{ false };
    if (enable && disable_zoomed && g_pLocalPlayer->holding_sniper_rifle)
    {
        if (g_pLocalPlayer->bZoomed)
        {
            was_tp = true;
            enable = false;
        }
    }
    if (was_tp && disable_zoomed && g_pLocalPlayer->holding_sniper_rifle)
    {
        if (!g_pLocalPlayer->bZoomed)
        {
            was_tp = false;
            enable = true;
        }
    }

    if (enable)
    {
        // Add thirdperson
        if (!g_pLocalPlayer->life_state)
            CE_INT(LOCAL_E, netvar.nForceTauntCam) = 1;
        was_enabled = true;
    }
    if (!enable && was_enabled)
    {
        // Remove thirdperson
        CE_INT(LOCAL_E, netvar.nForceTauntCam) = 0;
        was_enabled                            = false;
    }
    if (real_angles && g_IInput->CAM_IsThirdPerson())
    {
        CE_FLOAT(LOCAL_E, netvar.deadflag + 4) = g_pLocalPlayer->realAngles.x;
        CE_FLOAT(LOCAL_E, netvar.deadflag + 8) = g_pLocalPlayer->realAngles.y;
    }
}
} // namespace hacks::tf::thirdperson
