
/*
 *
 * A Simple triggerbot for nekohook.
 *
 */

#include "../util/catvars.hpp"
#include "../framework/input.hpp"

#include "triggerbot.hpp"

namespace features { namespace trigger {

static CatEnum trigger_menu({"TriggerBot"});
static CatVarBool enabled(trigger_menu, "trigger", false, "Enable TriggerBot", "Master TriggerBot switch");
static CatVarKey trigger_key(trigger_menu, "trigger_key", CATKEY_NONE, "Triggerbot key", "Triggerbot key.");
static CatVarEnum teammates(trigger_menu, {"ENEMY ONLY", "TEAMMATE ONLY", "BOTH"}, "aimbot_teammates", 0, "Teammates", "Use to choose which team/s to target");
static CatVarEnum hitbox_mode(trigger_menu, {"AUTO", "HEAD", "ANY"}, "aimbot_hitbox_mode", 0, "Hitbox Mode", "Hitbox selection mode");

/*static bool TargetGood(CatEntity* entity) {
  if (!entity || GetDormant(entity))
    return false;
  if (!GetAlive(entity)
    return false;

  return true;
}*/
static void WorldTick(){
  /*if (!enabled) return;

  auto local_ent = GetLocalPlayer();
  if (!local_ent) return;

  // Check if we can shoot in the first place
  //if (!ShouldShoot())
    //return;

  // Get what we see
  auto camera_pos = GetCamera(local_ent);
  auto camera_ang = GetCameraAngles(local_ent);
  auto trace_res = trace_sight(camera_pos, util::ExtendLine(camera_pos, camera_ang, 8192.0f));

  // Check if shootable
  if (!TargetGood(entity))
    return;*/



}

void Init() {

}

}}
