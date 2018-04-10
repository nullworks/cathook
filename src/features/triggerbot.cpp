
/*
 *
 * A Simple triggerbot for nekohook.
 *
 */

#include "../util/catvars.hpp"
#include "../framework/gameticks.hpp"
#include "../framework/trace.hpp"
#include "aimbot.hpp" // We will use the aimbot shoot target checks to make triggerbot simple

#include "triggerbot.hpp"

namespace features::trigger {

static CatEnum trigger_menu({"TriggerBot"});
static CatVarBool enabled(trigger_menu, "trigger", false, "Enable TriggerBot", "Master TriggerBot switch");
static CatVarKey trigger_key(trigger_menu, "trigger_key", CATKEY_NONE, "Triggerbot key", "Triggerbot key.");
static CatEnum teammates_enum({"ENEMY ONLY", "TEAMMATE ONLY", "BOTH"});
static CatVarEnum teammates(trigger_menu, teammates_enum, "aimbot_teammates", 0, "Teammates", "Use to choose which team/s to target");
static CatEnum hitbox_mode_enum({"AUTO", "HEAD", "ANY"});
static CatVarEnum hitbox_mode(trigger_menu, hitbox_mode_enum, "aimbot_hitbox_mode", 0, "Hitbox Mode", "Hitbox selection mode");

static void WorldTick(){
  if (!enabled) return;

  auto local_ent = GetLocalPlayer();
  if (!local_ent) return;

  // Check if we can shoot in the first place
  if (!aimbot::ShouldAim())
    return;

  // Get what we see
  auto camera_pos = GetCamera(local_ent);
  auto camera_ang = GetCameraAngle(local_ent);
  auto trace_res = trace::trace_sight(camera_pos, util::ExtendLine(camera_pos, camera_ang, 8192.0f));

  // Check if shootable
  if (!aimbot::IsTargetGood(trace_res.first).first)
    return;

  Attack(local_ent);
}

void Init() {

}

}
