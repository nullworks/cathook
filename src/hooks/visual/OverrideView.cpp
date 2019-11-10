/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include "MiscTemporary.hpp"
#include <settings/Float.hpp>
#include "HookedMethods.hpp"

static settings::Float override_fov_zoomed{ "visual.fov-zoomed", "0" };
static settings::Float override_fov{ "visual.fov", "0" };
static settings::Float freecam_speed{ "visual.freecam-speed", "800.0f" };
static settings::Button freecam{ "visual.freecam-button", "<none>" };
bool freecam_is_toggled{ false };
static std::optional<Vector> vec;
std::optional<Vector> viewmodel_aimbot(Vector angles)
{
    if (CE_BAD(LOCAL_E) || !LOCAL_E->m_bAlivePlayer())
        return std::nullopt;

    static bool first          = true;
    static Vector oAngle       = Vector(0, 0, 0);
    static float timeremaining = 1000.f;
    static float maxtime       = 0.4f;
    if (!(*aim_angles).IsValid() || (*aim_angles).IsZero())
        return std::nullopt;

    Vector view_angles = angles;
    if ((view_angles - *aim_angles).Length() < 1.f || timeremaining < 0.f)
    {
        *aim_angles = *original_aim_angles = oAngle = Vector(0, 0, 0);
        first                                       = true;
        timeremaining                               = 1000.f;
        return std::nullopt;
    } // reset
    auto Viewmodel = g_IEntityList->GetClientEntity(HandleToIDX(CE_INT(LOCAL_E, netvar.m_hViewModel)));
    if (!Viewmodel)
        return std::nullopt;
    if (first || oAngle != *original_aim_angles)
    { // if we just set the angle or if we started aimboting someone else reset
        oAngle        = *aim_angles;
        first         = false;
        timeremaining = maxtime;
    }

    auto deltaAngle = *aim_angles - view_angles;
    fClampAngle(deltaAngle);

    vec = view_angles + deltaAngle;

    timeremaining -= g_GlobalVars->absoluteframetime;

    aim_angles = Lerp(timeremaining / maxtime, view_angles, oAngle);
    fClampAngle(*aim_angles);
    return vec;
}

namespace hooked_methods
{

DEFINE_HOOKED_METHOD(CalcViewModelView, void, IClientEntity *_this, IClientEntity *player, Vector &eyePos, QAngle &eyeAng)
{
    if (vec)
        eyeAng = VectorToQAngle(*vec);
    return original::CalcViewModelView(_this, player, eyePos, eyeAng);
}

static InitRoutine hook_calcviewmodelview([]() {
    EC::Register(
        EC::Shutdown, []() { hooks::viewmodel.Release(); }, "init_calcmodel");
    EC::Register(
        EC::CreateMove,
        []() {
            if (CE_BAD(LOCAL_E) || !LOCAL_E->m_bAlivePlayer())
                return;
            auto Viewmodel = g_IEntityList->GetClientEntity(HandleToIDX(CE_INT(LOCAL_E, netvar.m_hViewModel)));
            if (!Viewmodel || hooks::viewmodel.IsHooked(Viewmodel))
                return;
            hooks::viewmodel.Set(Viewmodel);
            hooks::viewmodel.HookMethod(HOOK_ARGS(CalcViewModelView));
            hooks::viewmodel.Apply();
        },
        "calcviewmodel_hook");
});

DEFINE_HOOKED_METHOD(OverrideView, void, void *this_, CViewSetup *setup)
{
    original::OverrideView(this_, setup);

    if (!isHackActive() || g_Settings.bInvalid || CE_BAD(LOCAL_E))
        return;

    if (!viewmodel_aimbot(QAngleToVector(setup->angles)))
        vec = std::nullopt;

    if (g_pLocalPlayer->bZoomed && override_fov_zoomed)
    {
        setup->fov = *override_fov_zoomed;
    }
    else if (override_fov)
    {
        setup->fov = *override_fov;
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
