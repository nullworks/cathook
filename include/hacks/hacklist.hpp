/*
 * hacklist.h
 *
 *  Created on: Dec 18, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include "config.h"

#if ENABLE_VISUALS

#include "ESP.hpp"
#include "LightESP.hpp"
#include "SkinChanger.hpp"
#include "Radar.hpp"
#include "SpyAlert.hpp"

#endif
#if not LAGBOT_MODE
#include "Aimbot.hpp"
#include "AntiAim.hpp"
#include "AntiDisguise.hpp"
#include "AutoHeal.hpp"
#include "AutoReflect.hpp"
#include "AutoSticky.hpp"
#include "AutoDetonator.hpp"
#include "AntiCheat.hpp"
#include "AutoDeadringer.hpp"
#include "Bunnyhop.hpp"
#include "LagExploit.hpp"
#include "SeedPrediction.hpp"
#endif
#if ENABLE_VISUALS
#include "Radar.hpp"
#endif
#if not LAGBOT_MODE
#include "Walkbot.hpp"
#endif
#include "AutoJoin.hpp"
#if not LAGBOT_MODE
#include "AntiBackstab.hpp"
#include "AutoBackstab.hpp"
#include "FollowBot.hpp"
#include "HealArrows.hpp"
#include "Misc.hpp"
#include "Trigger.hpp"
#include "KillSay.hpp"
#include "UberSpam.hpp"
#include "Achievement.hpp"
#endif
#include "Spam.hpp"
#if not LAGBOT_MODE
#include "Noisemaker.hpp"
#include "FollowBot.hpp"
#include "Announcer.hpp"
#include "Killstreak.hpp"
#endif
#include "CatBot.hpp"
#if not LAGBOT_MODE
#include "Backtrack.hpp"
#endif
