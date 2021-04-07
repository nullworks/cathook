/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include "MiscTemporary.hpp"
#include "Warp.hpp"
#include "nospread.hpp"

std::array<Timer, 32> timers{};
std::array<int, 32> bruteint{};

int spectator_target;
CLC_VoiceData *voicecrash{};
bool firstcm = false;
Timer DelayTimer{};
float prevflow            = 0.0f;
int prevflowticks         = 0;
int stored_buttons        = 0;
bool calculated_can_shoot = false;
bool ignoredc             = false;

bool *bSendPackets{ nullptr };
bool ignoreKeys{ false };
settings::Boolean clean_chat{ "chat.clean", "false" };

settings::Boolean crypt_chat{ "chat.crypto", "true" };
settings::Boolean clean_screenshots{ "visual.clean-screenshots", "false" };
#if ENABLE_TEXTMODE
settings::Boolean nolerp{ "misc.no-lerp", "true" };
#else
settings::Boolean nolerp{ "misc.no-lerp", "false" };
#endif
float backup_lerp = 0.0f;
settings::Int fakelag_amount{ "misc.fakelag", "0" };
settings::Boolean fakelag_midair{ "misc.fakelag-midair-only", "false" };
settings::Boolean no_zoom{ "remove.zoom", "false" };
settings::Boolean no_scope{ "remove.scope", "false" };
settings::Boolean disable_visuals{ "visual.disable", "false" };
settings::Int print_r{ "print.rgb.r", "183" };
settings::Int print_g{ "print.rgb.b", "27" };
settings::Int print_b{ "print.rgb.g", "139" };
Color menu_color{ *print_r, *print_g, *print_b, 255 };

void color_callback(settings::VariableBase<int> &, int)
{
    menu_color = Color(*print_r, *print_g, *print_b, 255);
}
DetourHook cl_warp_sendmovedetour;
DetourHook cl_nospread_sendmovedetour;

static InitRoutine misc_init([]() {
    static auto cl_sendmove_addr = gSignatures.GetEngineSignature("55 89 E5 57 56 53 81 EC 2C 10 00 00 C6 85 ? ? ? ? 01");
    // Order matters!
    cl_warp_sendmovedetour.Init(cl_sendmove_addr, (void *) hacks::tf2::warp::CL_SendMove_hook);
    cl_nospread_sendmovedetour.Init(cl_sendmove_addr, (void *) hacks::tf2::nospread::CL_SendMove_hook);

    static std::optional<BytePatch> patch;
    static std::optional<BytePatch> patch2;
    print_r.installChangeCallback(color_callback);
    print_g.installChangeCallback(color_callback);
    print_b.installChangeCallback(color_callback);
    no_scope.installChangeCallback([](settings::VariableBase<bool> &, bool after) {
        if (!patch)
        {
            // Remove scope
            patch = BytePatch(gSignatures.GetClientSignature, "81 EC ? ? ? ? A1 ? ? ? ? 8B 7D 08 8B 10 89 04 24 FF 92", 0x0, { 0x5B, 0x5E, 0x5F, 0x5D, 0xC3 });
            // Keep rifle visible
            patch2 = BytePatch(gSignatures.GetClientSignature, "74 ? A1 ? ? ? ? 8B 40 ? 85 C0 75 ? C9", 0x0, { 0x70 });
        }
        if (after)
        {
            patch->Patch();
            if (no_zoom)
                patch2->Patch();
        }
        else
        {
            patch->Shutdown();
            if (patch2)
                patch2->Shutdown();
        }
    });
    no_zoom.installChangeCallback([](settings::VariableBase<bool> &, bool after) {
        // std::optional so the addresses are searched when needed, not on inject
        if (!patch2)
            // Keep rifle visible
            patch2 = BytePatch(gSignatures.GetClientSignature, "74 ? A1 ? ? ? ? 8B 40 ? 85 C0 75 ? C9", 0x0, { 0x70 });
        if (after)
        {
            if (no_scope)
                patch2->Patch();
        }
        else
        {
            patch2->Shutdown();
        }
    });
    nolerp.installChangeCallback([](settings::VariableBase<bool> &, bool after) {
        if (!after)
        {
            if (backup_lerp)
            {
                cl_interp->SetValue(backup_lerp);
                backup_lerp = 0.0f;
            }
        }
        else
        {
            backup_lerp = cl_interp->GetFloat();
            // We should adjust cl_interp to be as low as possible
            if (cl_interp->GetFloat() > 0.152f)
                cl_interp->SetValue(0.152f);
        }
    });
    EC::Register(
        EC::Shutdown,
        []() {
            cl_warp_sendmovedetour.Shutdown();
            cl_nospread_sendmovedetour.Shutdown();
            if (backup_lerp)
            {
                cl_interp->SetValue(backup_lerp);
                backup_lerp = 0.0f;
            }
            patch.reset();
            patch2.reset();
        },
        "misctemp_shutdown");
#if ENABLE_TEXTMODE
    // Ensure that we trigger the callback for textmode builds
    nolerp = false;
    nolerp = true;
#endif
});
