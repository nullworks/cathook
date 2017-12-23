
/*
 *
 *
 *
 *
 */

#include <vector>

#include "../util/catvars.hpp"
#include "../framework/entitys.hpp"
#include "../framework/trace.hpp"

#include "followbot.hpp"

namespace features { namespace followbot {

static CatEnum followbot_menu({"Autonomy", "Followbot"});
static CatVarBool followbot(followbot_menu, "fb_bot", false, "Master Followbot Switch", "Set to 1 in followbots' configs");
static CatVarInt follow_distance(followbot_menu, "fb_distance", 175, "Follow Distance", "How close the bots should stay to the target");
static CatVarInt follow_activation(followbot_menu, "fb_activation", 175, "Activation Distance", "How close a player should be until the followbot will pick them as a target");

// Something to store breadcrumbs created by followed players
static std::vector<CatVector> breadcrumbs;

// Followed entity
static const CatEntity* follow_target = nullptr;

// Followbot state
enum {
  FB_STATE_IDLE,
  FB_STATE_NEW_TARGET,
  FB_STATE_FOLLOWING
};
static int followbot_state = FB_STATE_IDLE;

static void WorldTick(){

  if (!followbot) return;

  // We need a local player to control
  if (!g_LocalPlayer.entity || !g_LocalPlayer.entity->alive) {
    followbot_state = FB_STATE_IDLE;
    return;
  }

  // If we are idle, we need a new target
  if (!follow_target || followbot_state == FB_STATE_IDLE) {

    // Reset follow target and crumbs
    follow_target = nullptr;
    breadcrumbs.clear();

    // Try to get a new target
    for (const auto& entity : g_CatEntitys) {

      // Test the entity if its good enough to follow
      if (CE_BAD(entity)) continue; // Exist + dormant
      if (&entity == g_LocalPlayer.entity) continue; // Follow self lol
      if (!entity.alive) continue; // Dont follow dead players
      if (entity.Enemy()) continue; // Dont follow enemys
      // Check user setting and distance stuff
      if (entity.Distance() > follow_activation) continue;
      // Vis check
      if (!trace::TraceEnt(entity, g_LocalPlayer.GetCamera(), (entity.collision != CatBox()) ? entity.collision.center() : entity.origin)) continue;

      // Update follow_target if nessesary
      if (!follow_target || follow_target->Distance() > entity.Distance()) {
        follow_target = &entity;
        followbot_state = FB_STATE_NEW_TARGET;
      }
    }
  }

  // Generate new crumbs, whether we dont have any crumbs, or we player is too far from last crumb
	if ((breadcrumbs.empty() || follow_target->origin.DistTo(breadcrumbs[breadcrumbs.size()]) > 40.0F)) { //&& DistanceToGround(found_entity) < 40) {

		// Add to the crumb.
		breadcrumbs.push_back(follow_target->origin);
	}

  // Prune old and close crumbs that we wont need anymore
  for (const auto& crumb : breadcrumbs) {

    // Dont prune this crumb if we already have a little amount of them, or its too far away
    if (breadcrumbs.size() < 2 || g_LocalPlayer.entity->origin.DistTo(crumb) > 60.f) break;

    // Crumb is too close, remove it
    breadcrumbs.erase(breadcrumbs.begin());

    // If we were a new target, start using normal followbot operation
    if (followbot_state == FB_STATE_NEW_TARGET)
      followbot_state = FB_STATE_FOLLOWING;
  }

  // Follow the crumbs when too far away, or just starting to follow
	if (!breadcrumbs.empty() && (followbot_state == FB_STATE_NEW_TARGET || g_LocalPlayer.entity->origin.DistTo(follow_target->origin) > follow_distance)) {

    // TODO, make walk to CMFunction
    // WalkTo();

		// TODO, go into idle if havent pruned in a while
    // Halp, i dont understand chrono ;(
	}
}

void Init(){

}

}}
