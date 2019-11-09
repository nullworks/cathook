/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <MiscTemporary.hpp>
#include <settings/Float.hpp>
#include <settings/Bool.hpp>
#include "HookedMethods.hpp"

static settings::Float override_fov_zoomed{ "visual.fov-zoomed", "0" };
static settings::Float override_fov{ "visual.fov", "0" };
static settings::Float freecam_speed{ "visual.freecam-speed", "800.0f" };
static settings::Button freecam{ "visual.freecam-button", "<none>" };
static settings::Boolean vm_aimbot{ "visual.vm-aimbot.enabled", "true" };

bool freecam_is_toggled{ false };

namespace hooked_methods
{

DEFINE_HOOKED_METHOD(OverrideView, void, void *this_, CViewSetup *setup)
{    
    original::OverrideView(this_, setup);

    if (!isHackActive() || g_Settings.bInvalid || CE_BAD(LOCAL_E))
        return;

    if (g_pLocalPlayer->bZoomed && override_fov_zoomed)
    {
        setup->fov = *override_fov_zoomed;
    }
    else if (override_fov)
    {
        setup->fov = *override_fov;
    }

    if (CE_GOOD(LOCAL_E) && LOCAL_E->m_bAlivePlayer())
    {
        static Vector oViewmodelAimbotAngle = Vector(0, 0, 0);
        static Vector ViewmodelAimbotAngle  = Vector(0, 0, 0);
        bool first                          = true;
        static Vector oAngle                = Vector(0, 0, 0);
        static float timeremaining          = 1000.f;
        static float maxtime                = 0.4f;

        ViewmodelAimbotAngle = *aim_angles;

        if (ViewmodelAimbotAngle.IsValid() && !ViewmodelAimbotAngle.IsZero())
        {
            Vector vec_setup_angles = *(Vector *) &setup->angles;

            if ((vec_setup_angles - ViewmodelAimbotAngle).Length() < 1.f || timeremaining < 0.f)
            {
                ViewmodelAimbotAngle = oViewmodelAimbotAngle = oAngle = Vector(0, 0, 0);
                first                                                 = true;
                timeremaining                                         = 1000.f;

            } // reset
            else
            {
                auto Viewmodel = (IClientEntity *) g_IEntityList->GetClientEntityFromHandle(CE_INT(LOCAL_E, netvar.hViewModel));
                if (Viewmodel)
                {

                    if (first && oAngle != oViewmodelAimbotAngle)
                    {
                        oAngle        = ViewmodelAimbotAngle;
                        first         = false;
                        timeremaining = maxtime;
                    }

                    auto deltaAngle = ViewmodelAimbotAngle - vec_setup_angles;

                    fClampAngle(deltaAngle);

                    QAngle &write = const_cast<QAngle &>(Viewmodel->GetAbsAngles());

                    write         = VectorToQAngle(vec_setup_angles + deltaAngle);

                    timeremaining -= g_GlobalVars->frametime;

                    ViewmodelAimbotAngle = QAngleToVector(Lerp<QAngle>(timeremaining / maxtime, setup->angles, VectorToQAngle(oAngle)));

                    fClampAngle(ViewmodelAimbotAngle);
                }
            }
        }
}

if (spectator_target)
{
    CachedEntity *spec = ENTITY(spectator_target);
    if (CE_GOOD(spec) && !CE_BYTE(spec, netvar.iLifeState))
    {
        setup->origin = spec->m_vecOrigin() + CE_VECTOR(spec, netvar.vViewOffset);
        // why not spectate yourself
        if (spec == LOCAL_E)
            {
                setup->angles = CE_VAR(spec, netvar.m_angEyeAnglesLocal, QAngle);
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

    if (freecam && current_user_cmd)
    {
        static Vector freecam_origin{ 0 };
        static bool freecam_last{ false };
        static bool first_toggle{ true };

        if (freecam.isKeyDown() && !freecam_last)
        {
            freecam_is_toggled = !freecam_is_toggled;
            first_toggle       = true;
        }
        if (freecam_is_toggled)
        {
            if (first_toggle)
            {
                freecam_origin = setup->origin;
                first_toggle   = false;
            }
            QAngle ang;
            Vector angle;

            float speed = *freecam_speed * g_GlobalVars->absoluteframetime;
            g_IEngine->GetViewAngles(ang);
            angle = QAngleToVector(ang);
            if (stored_buttons & IN_FORWARD)
                freecam_origin += GetForwardVector({ 0.0f, 0.0f, 0.0f }, angle, speed);
            if (stored_buttons & IN_BACK)
                freecam_origin -= GetForwardVector({ 0.0f, 0.0f, 0.0f }, angle, speed);
            if (stored_buttons & IN_MOVELEFT || stored_buttons & IN_MOVERIGHT)
            {
                Vector new_angle = angle;
                new_angle.y += 90.0f;
                new_angle.x = 0.0f;
                fClampAngle(new_angle);
                Vector touse = GetForwardVector({ 0.0f, 0.0f, 0.0f }, new_angle, speed);
                if (stored_buttons & IN_MOVELEFT)
                    freecam_origin += touse;
                if (stored_buttons & IN_MOVERIGHT)
                    freecam_origin -= touse;
            }
            setup->origin = freecam_origin;
        }
        freecam_last = freecam.isKeyDown();
    }
    else if (current_user_cmd)
        freecam_is_toggled = false;

    draw::fov = setup->fov;
}
} // namespace hooked_methods
