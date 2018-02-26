
/*
 *
 * A Simple triggerbot for nekohook.
 *
 */

#include "../util/catvars.hpp"
#include "../framework/input.hpp"

#include "triggerbot.hpp"

namespace features { namespace triggerbot {

static CatEnum trigger_menu({"TriggerBot"});
static CatVarBool enabled(trigger_menu, "trigger", false, "Enable TriggerBot", "Master TriggerBot switch");
static CatVarKey trigger_key(trigger_menu, "trigger_key", CATKEY_NONE, "Triggerbot key", "Triggerbot key.");
static CatVarEnum teammates(trigger_menu, {"ENEMY ONLY", "TEAMMATE ONLY", "BOTH"}, "aimbot_teammates", 0, "Teammates", "Use to choose which team/s to target");
static CatVarEnum hitbox_mode(trigger_menu, {"AUTO", "HEAD", "ANY"}, "aimbot_hitbox_mode", 0, "Hitbox Mode", "Hitbox selection mode");

static void WorldTick(){
  if (!enabled) return;

}

void Init() {

}

}}
