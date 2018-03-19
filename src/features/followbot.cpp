
/*
 *
 *
 *
 *
 */

#include <vector>

#include "../util/chrono.hpp"
#include "../util/logging.hpp"
#include "../util/catvars.hpp"
#include "../framework/gameticks.hpp" // So we can run things in draw and world tick
#include "../framework/trace.hpp"
#include "../framework/drawing.hpp"	// draw some crumbs

#include "followbot.hpp"

namespace features::followbot {

static CatEnum followbot_menu({"Autonomy", "Followbot"});
static CatVarBool followbot(followbot_menu, "fb", false, "Followbot Switch", "Set to 1 in followbots' configs");
static CatVarBool roambot(followbot_menu, "fb_roaming", true, "Roambot", "Followbot will roam free, finding targets it can");
static CatVarBool draw_crumb(followbot_menu, "fb_draw", true, "Draw crumbs", "Self explanitory");
static CatVarInt follow_distance(followbot_menu, "fb_distance", 175, "Follow Distance", "How close the bots should stay to the target");
static CatVarInt follow_activation(followbot_menu, "fb_activation", 175, "Activation Distance", "How close a player should be until the followbot will pick them as a target");
static CatVarInt follow_steam(followbot_menu, "fb_steam", 0, "Follow Steam Id", "Set a steam id to let followbot prioritize players");

// Something to store breadcrumbs created by followed players
static std::vector<CatVector> breadcrumbs;
static const int crumb_limit = 64; // limit

// Followed entity, externed for highlight color
CatEntity* follow_target = nullptr;

static void WorldTick() {
  if (!followbot) return;

  // We need a local player to control
  auto local_ent = GetLocalPlayer();
  if (!local_ent || !GetAlive((CatEntity*)local_ent)) {
    follow_target = nullptr;
    return;
  }

  // Still good check
  if (follow_target) {
    // Overflow protection
    if (breadcrumbs.size() > crumb_limit)
      follow_target = nullptr;
    // Still good check
    else if (GetDormant(follow_target) || !GetAlive(follow_target))
      follow_target = nullptr;
  }

  // Target Selection
  if (!follow_target) {
    breadcrumbs.clear(); // no target == no path

    if (follow_steam) {
      // Find a target with the steam id, as it is prioritized
      for (int i = 0; i < GetEntityCount(); i++) {
        auto entity = GetEntity(i);
        if (!entity || GetDormant(entity)) // Exist + dormant
          continue;
        if (GetType(entity) != ETYPE_PLAYER)
          continue;
        if (follow_steam != GetSteamId(entity)) // steamid check
          continue;
        if (!GetAlive(entity)) // Dont follow dead players
          continue;
        auto coll = GetCollision(entity);
        if (!trace::trace_entity(entity, GetCamera(local_ent), // vis check
                                (coll != CatBox()) ? coll.GetCenter() : GetOrigin(entity)))
          continue;
        follow_target = entity;
        break;
      }
    }
    // If we dont have a follow target from that, we look again for someone else who is suitable
    if (!follow_target && roambot) {
      // Try to get a new target
      for (int i = 0; i < GetEntityCount(); i++) {
        auto entity = GetEntity(i);
        if (!entity || GetDormant(entity)) // Exist + dormant
          continue;
        if (GetType(entity) != ETYPE_PLAYER)
          continue;
        if (entity == (CatEntity*)local_ent) // Follow self lol
          continue;
        if (!GetAlive(entity) || GetEnemy(entity)) // Dont follow dead players
          continue;
        if (follow_activation && GetDistance(entity) > follow_activation)
          continue;
        auto coll = GetCollision(entity);
        if (!trace::trace_entity(entity, GetCamera(local_ent), // vis check
                                (coll != CatBox()) ? coll.GetCenter() : GetOrigin(entity)))
          continue;
        if (follow_target && GetDistance(follow_target) > GetDistance(entity)) // favor closer entitys
          continue;
        // ooooo, a target
        follow_target = entity;
      }
    }
    // last check for entity before we continue
    if (!follow_target)
      return;
  }

  // If the player is close enough, we dont need to follow the path
  auto tar_orig = GetOrigin(follow_target);
  auto loc_orig = GetOrigin((CatEntity*)local_ent);
  auto dist_to_target = loc_orig.DistTo(tar_orig);
  if (dist_to_target < 30)
    breadcrumbs.clear();

  // Update timer on new target
  static CatTimer idle_time;
  if (breadcrumbs.empty())
    idle_time.Reset();

  // New crumbs, we add one if its empty so we have something to follow
	if (breadcrumbs.empty() || tar_orig.DistTo(breadcrumbs.at(breadcrumbs.size() - 1)) > 40.0F) //&& DistanceToGround(found_entity) < 40) {
		breadcrumbs.push_back(tar_orig);

  // Prune old and close crumbs that we wont need anymore, update idle timer too
  while (breadcrumbs.size() > 2 && loc_orig.DistTo(breadcrumbs.at(0)) < 60.f) {
    idle_time.Reset();
    breadcrumbs.erase(breadcrumbs.begin());
  }

  // Follow the crumbs when too far away, or just starting to follow
	if (dist_to_target > follow_distance) {

    // Check for idle
    if (idle_time.CheckTime(std::chrono::seconds(3))) {
      follow_target = nullptr;
      return;
    }
    // TODO, make walk to CMFunction in entitys framework
    // WalkTo(breadcrumbs.at(0));
	} else
    idle_time.Reset();
}

static void DrawTick(){
  if (!followbot || !draw_crumb) return;
  if (breadcrumbs.size() < 2) return;
  for (size_t i = 0; i < breadcrumbs.size() - 1; i++) {
    CatVector wts1, wts2;
    if (draw::WorldToScreen(breadcrumbs.at(i), wts1) && draw::WorldToScreen(breadcrumbs.at(i + 1), wts2)) {
      draw::Line(wts1.x, wts1.y, wts2.x - wts1.x, wts2.y - wts1.y, colors::white);
    }
  }
  CatVector wts;
  if (!draw::WorldToScreen(breadcrumbs.at(0), wts)) return;
  draw::RectFilled(wts.x - 4, wts.y - 4, 8, 8, colors::white);
  draw::Rect(wts.x - 5, wts.y - 5, 10, 10, colors::black);
}

void Init(){
  wtickmgr.REventDuring(WorldTick);
  drawmgr.REventDuring(DrawTick);
}

}
