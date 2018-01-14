
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

// The limit for the crumbs
static const int crumb_limit = 64;

// Something to store breadcrumbs created by followed players
static std::vector<CatVector> breadcrumbs;
//breadcrumbs.reserve(crumb_limit);

// Followed entity
static CatEntity* follow_target = nullptr;

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
  auto local_ent = GetLocalPlayer();
  if (!local_ent || !GetAlive((CatEntity*)local_ent)) {
    followbot_state = FB_STATE_IDLE;
    return;
  }

  // If we are idle, we need a new target
  if (!follow_target || followbot_state == FB_STATE_IDLE || breadcrumbs.size() > crumb_limit) {

    // Reset follow target and crumbs
    follow_target = nullptr;
    breadcrumbs.clear();

    // Try to get a new target
    for (int i = 0; i < GetEntityCount(); i++) {
      auto entity = GetEntity(i);
      // Test the entity if its good enough to follow
      if (GetDormant(entity)) continue; // Exist + dormant
      if (entity == (CatEntity*)local_ent) continue; // Follow self lol
      if (!GetAlive(entity)) continue; // Dont follow dead players`
      if (GetEnemy(entity)) continue; // Dont follow enemys
      // Check user setting and distance stuff
      if (GetDistance(entity) > follow_activation) continue;
      // Vis check
    if (!trace::TraceEnt(entity, GetCamera(local_ent), (GetCollision((CatEntity*)local_ent) != CatBox()) ? GetCollision((CatEntity*)local_ent).GetCenter() : GetOrigin(entity))) continue;

      // Update follow_target if nessesary
      if (!follow_target || GetDistance(follow_target) > GetDistance(entity)) {
        follow_target = entity;
        followbot_state = FB_STATE_NEW_TARGET;
      }
    }
  }

  // Generate new crumbs, whether we dont have any crumbs, or we player is too far from last crumb
	if ((breadcrumbs.empty() || GetOrigin(follow_target).DistTo(breadcrumbs[breadcrumbs.size()]) > 40.0F)) { //&& DistanceToGround(found_entity) < 40) {

		// Add to the crumb.
		breadcrumbs.push_back(GetOrigin(follow_target));
	}

  // Prune old and close crumbs that we wont need anymore
  for (const auto& crumb : breadcrumbs) {

    // Dont prune this crumb if we already have a little amount of them, or its too far away
    if (breadcrumbs.size() < 2 || GetOrigin((CatEntity*)local_ent).DistTo(crumb) > 60.f) break;

    // Crumb is too close, remove it
    breadcrumbs.erase(breadcrumbs.begin());

    // If we were a new target, start using normal followbot operation
    if (followbot_state == FB_STATE_NEW_TARGET)
      followbot_state = FB_STATE_FOLLOWING;
  }

  // Follow the crumbs when too far away, or just starting to follow
	if (!breadcrumbs.empty() && (followbot_state == FB_STATE_NEW_TARGET || GetOrigin((CatEntity*)local_ent).DistTo(GetOrigin(follow_target)) > follow_distance)) {

    // TODO, make walk to CMFunction
    // WalkTo();

		// TODO, go into idle if havent pruned in a while
    // Halp, i dont understand chrono ;(
	}
}

void Init(){

}

}}
