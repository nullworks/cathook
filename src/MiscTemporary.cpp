/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include "MiscTemporary.hpp"
std::array<Timer, 32> timers{};
std::array<int, 32> bruteint{};

int spectator_target;
CLC_VoiceData *voicecrash{};
bool firstcm = false;
Timer DelayTimer{};
Timer LookAtPathTimer{};
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
static settings::Boolean no_zoom{ "remove.scope", "false" };
settings::Boolean disable_visuals{ "visual.disable", "false" };
settings::Int print_r{ "print.rgb.r", "183" };
settings::Int print_g{ "print.rgb.b", "27" };
settings::Int print_b{ "print.rgb.g", "139" };
Color menu_color{ *print_r, *print_g, *print_b, 255 };
#if !TEXTMODE
settings::Boolean null_graphics("hack.nullgraphics", "false");
#else
settings::Boolean null_graphics("hack.nullgraphics", "true");
#endif

void color_callback(settings::VariableBase<int> &, int)
{
    menu_color = Color(*print_r, *print_g, *print_b, 255);
}
static InitRoutine color_init([]() {
    print_r.installChangeCallback(color_callback);
    print_g.installChangeCallback(color_callback);
    print_b.installChangeCallback(color_callback);
    no_zoom.installChangeCallback([](settings::VariableBase<bool> &, bool after) {
        static BytePatch patch(gSignatures.GetClientSignature, "81 EC ? ? ? ? A1 ? ? ? ? 8B 7D 08 8B 10 89 04 24 FF 92", 0x0, { 0x5B, 0x5E, 0x5F, 0x5D, 0xC3 });
        after ? patch.Patch() : patch.Shutdown();
    });
});
