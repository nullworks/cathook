
/*
 *
 * A Simple triggerbot for nekohook.
 *
 */

#include "../util/catvars.hpp"
#include "../util/chrono.hpp"
#include "../framework/gameticks.hpp"
#include "../framework/trace.hpp"
#include "aimbot.hpp" // We will use the aimbot shoot target checks to make triggerbot simple
#include "antiaim.hpp" // AA safe ticks

#include "triggerbot.hpp"

namespace features::trigger {

static CatEnum trigger_menu({"TriggerBot"});
static CatVarBool enabled(trigger_menu, "trigger", false, "Enable TriggerBot", "Master TriggerBot switch");
static CatVarInt delay(trigger_menu, "trigger_delay", 0, "TriggerDelay", "How long to delay triggerbot in ms", 3000);
static CatVarInt accuracy(trigger_menu, "trigger_accuracy", 0, "TriggerAccuracy", "How much to shrink the hitboxes and check them", 3000);

static void WorldTick(){
  if (!enabled) return;

  // Check the timer
  bool delaying = false;
  CatTimer delay_timer;
  if (delaying && !delay_timer.CheckTime(std::chrono::milliseconds(delay)))
    return;

  auto local_ent = GetLocalPlayer();
  if (!local_ent) {
    delaying = false;
    return;
  }

  // Check if we can shoot in the first place
  if (!aimbot::ShouldAim()) {
    delaying = false;
    return;
  }

  // Get what we see
  auto camera_pos = GetCamera(local_ent);
  auto dest = util::ExtendLine(camera_pos, GetCameraAngle(local_ent), 8192.0f); // used for accuracy check
  auto trace_res = trace::trace_sight(camera_pos, dest);

  // Check if shootable
  // TODO, this will not work as it visual checks for us, we need a better solution, c+p from aimbot?
  if (!aimbot::IsTargetGood(trace_res.first).first) {
    delaying = false;
    return;
  }

  // Check accuracy
  if (accuracy) {
    // Check hitboxes, TODO, remove IsTargetGood and add the hitbox thing to it so it isnt static
    CatBox tmp;
    if (GetBone(trace_res.first, EBone_head, tmp)) {
      if (!tmp.LineIntersects(camera_pos, dest))
        return;
    }
  }

  // Only set timer if we arent already delaying
  if (!delaying && delay) {
    delaying = true;
    delay_timer.Reset();
    return;
  }

  // Disable antiaim and attack
  antiaim::AddAATicks(3);
  Attack(local_ent);
}

void Init() {
  wtickmgr.REventDuring(WorldTick);
}

}
