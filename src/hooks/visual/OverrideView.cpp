/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <MiscTemporary.hpp>
#include "HookedMethods.hpp"

static CatVar
    override_fov_zoomed(CV_FLOAT, XORSTR("fov_zoomed"), XORSTR("0"), XORSTR("FOV override (zoomed)"),
                        XORSTR("Overrides FOV with this value when zoomed in ")
                        XORSTR("(default FOV when zoomed is 20)"));
static CatVar override_fov(CV_FLOAT, XORSTR("fov"), XORSTR("0"), XORSTR("FOV override"),
                           XORSTR("Overrides FOV with this value"));

static CatVar freecam(CV_KEY, XORSTR("debug_freecam"), XORSTR("0"), XORSTR("Freecam"));

namespace hooked_methods
{

DEFINE_HOOKED_METHOD(OverrideView, void, void *this_, CViewSetup *setup)
{
    static bool zoomed;
    original::OverrideView(this_, setup);
    if (!cathook)
        return;
    if (g_pLocalPlayer->bZoomed && override_fov_zoomed)
    {
        setup->fov = override_fov_zoomed;
    }
    else
    {
        if (override_fov)
        {
            setup->fov = override_fov;
        }
    }

    if (spectator_target)
    {
        CachedEntity *spec = ENTITY(spectator_target);
        if (CE_GOOD(spec) && !CE_BYTE(spec, netvar.iLifeState))
        {
            setup->origin =
                spec->m_vecOrigin() + CE_VECTOR(spec, netvar.vViewOffset);
            // why not spectate yourself
            if (spec == LOCAL_E)
            {
                setup->angles =
                    CE_VAR(spec, netvar.m_angEyeAnglesLocal, QAngle);
            }
            else
            {
                setup->angles = CE_VAR(spec, netvar.m_angEyeAngles, QAngle);
            }
        }
        if (g_IInputSystem->IsButtonDown(ButtonCode_t::KEY_SPACE))
        {
            spectator_target = 0;
        }
    }

    if (freecam)
    {
        static Vector freecam_origin{ 0 };
        static bool freecam_last{ false };
        if (freecam.KeyDown())
        {
            if (not freecam_last)
            {
                freecam_origin = setup->origin;
            }
            float sp, sy, cp, cy;
            QAngle angle;
            Vector forward;
            g_IEngine->GetViewAngles(angle);
            sy        = sinf(DEG2RAD(angle[1]));
            cy        = cosf(DEG2RAD(angle[1]));
            sp        = sinf(DEG2RAD(angle[0]));
            cp        = cosf(DEG2RAD(angle[0]));
            forward.x = cp * cy;
            forward.y = cp * sy;
            forward.z = -sp;
            forward *= 4;
            freecam_origin += forward;
            setup->origin = freecam_origin;
        }
        freecam_last = freecam.KeyDown();
    }

    draw::fov = setup->fov;
}
}