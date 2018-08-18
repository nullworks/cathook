/*
  Created on 29.07.18.
*/

#include <hacks/Thirdperson.hpp>
#include <settings/Bool.hpp>
#include <localplayer.hpp>
#include <entitycache.hpp>
#include <core/sdk.hpp>

static settings::Bool enable{ "visual.thirdperson.enable", "false" };
static settings::Bool angles{ "visual.thirdperson.real-angles", "false" };
static bool was_enabled{ false };

void hacks::tf::thirdperson::frameStageNotify()
{
    if (CE_BAD(LOCAL_E))
        return;

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
    if (angles && g_IInput->CAM_IsThirdPerson())
    {
        CE_FLOAT(LOCAL_E, netvar.deadflag + 4) =
            g_Settings.brute.last_angles[LOCAL_E->m_IDX].x;
        CE_FLOAT(LOCAL_E, netvar.deadflag + 8) =
            g_Settings.brute.last_angles[LOCAL_E->m_IDX].y;
    }
}
