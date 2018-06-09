/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include "MiscTemporary.hpp"
CatVar minigun_jump(CV_SWITCH, XORSTR("minigun_jump"), XORSTR("0"), XORSTR("TF2C minigun jump"),
                    XORSTR("Allows jumping while shooting with minigun"));

CatVar jointeam(CV_SWITCH, XORSTR("fb_autoteam"), XORSTR("1"),
                XORSTR("Joins player team automatically (NYI)"));
CatVar joinclass(CV_STRING, XORSTR("fb_autoclass"), XORSTR("spy"),
                 XORSTR("Class that will be picked after joining a team (NYI)"));

CatVar nolerp(CV_SWITCH, XORSTR("nolerp"), XORSTR("1"), XORSTR("NoLerp mode (experimental)"));

CatVar engine_pred(CV_SWITCH, XORSTR("engine_prediction"), XORSTR("0"), XORSTR("Engine Prediction"));
CatVar debug_projectiles(CV_SWITCH, XORSTR("debug_projectiles"), XORSTR("0"),
                         XORSTR("Debug Projectiles"));

CatVar fakelag_amount(CV_INT, XORSTR("fakelag"), XORSTR("0"), XORSTR("Bad Fakelag"));
CatVar serverlag_amount(
    CV_INT, XORSTR("serverlag"), XORSTR("0"), XORSTR("serverlag"),
    XORSTR("Lag the server by spamming this many voicecommands per tick"));
CatVar serverlag_string(CV_STRING, XORSTR("serverlag_string"), XORSTR("voicemenu 0 0"),
                        XORSTR("serverlag string"), XORSTR("String to spam with serverlag"));
CatVar servercrash(CV_SWITCH, XORSTR("servercrash"), XORSTR("0"), XORSTR("crash servers"),
                   XORSTR("Crash servers by spamming signon net messages"));
CatVar semiauto(CV_INT, XORSTR("semiauto"), XORSTR("0"), XORSTR("Semiauto"));
bool *bSendPackets;

CatVar crypt_chat(
    CV_SWITCH, XORSTR("chat_crypto"), XORSTR("1"), XORSTR("Crypto chat"),
    XORSTR("Start message with !! and it will be only visible to cathook users"));

int spectator_target;
CLC_VoiceData *voicecrash{};
bool firstcm = false;
Timer DelayTimer{};
CatVar delay(
    CV_INT, XORSTR("delay"), XORSTR("0"), XORSTR("Delay"),
    XORSTR("Delay actions like chat spam and serverlag/crash by this many seconds."));
CatVar adjust(CV_INT, XORSTR("serverlag_ramp"), XORSTR("0"), XORSTR("Ramp lag"),
              XORSTR("keep lag around this many seconds"));
float prevflow    = 0.0f;
int prevflowticks = 0;
