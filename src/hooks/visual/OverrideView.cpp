/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <MiscTemporary.hpp>
#include <settings/Float.hpp>
#include "HookedMethods.hpp"

static settings::Float override_fov{ "visual.fov", "0" };
static settings::Float freecam_speed{ "visual.freecam-speed", "800.0f" };
static settings::Button freecam{ "visual.freecam-button", "<none>" };
static settings::Button zoom_key{ "visual.zoom-key", "<null>" };
static settings::Int zoom_fov{ "visual.zoom-key.fov", "20" };
bool freecam_is_toggled{ false };
bool zoomed_last_tick{ false };

namespace hooked_methods
{

DEFINE_HOOKED_METHOD(OverrideView, void, void *this_, CViewSetup *setup)
{
    original::OverrideView(this_, setup);

    if (!isHackActive() || g_Settings.bInvalid || CE_BAD(LOCAL_E))
        return;

    bool zoomed = setup->fov < CE_INT(LOCAL_E, netvar.iDefaultFOV) || g_pLocalPlayer->bZoomed;
    if (*no_zoom && zoomed)
    {
        auto fov   = g_ICvar->FindVar("fov_desired");
        setup->fov = override_fov ? *override_fov : fov->GetInt();
    }
    else if (override_fov && !zoomed)
    {
        setup->fov = *override_fov;
    }
    if (zoom_key && zoom_key.isKeyDown())
    {
        auto default_fov               = g_ICvar->FindVar("default_fov");
        float sens_factor              = (*zoom_fov / setup->fov) * (setup->fov / default_fov->GetFloat());
        setup->fov                     = *zoom_fov;
        g_CHUD->GetSensitivityFactor() = sens_factor;
        zoomed_last_tick               = true;
    }
    else if (zoomed_last_tick)
    {
        g_CHUD->GetSensitivityFactor() = 0.0f;
        zoomed_last_tick               = false;
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
static InitRoutine override_init([]() {
    EC::Register(
        EC::Shutdown, []() { g_CHUD->GetSensitivityFactor() = 0.0f; }, "override_shutdown");
});
} // namespace hooked_methods
