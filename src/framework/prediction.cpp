
/*
 *
 *  This should have multiple values such as gravity, air resist and so on, to eventually make a iteration prediction engine using various info from the game
 *
 *
 */

#include <chrono>
#include <unordered_map>

#include "gameticks.hpp"
#include "drawing.hpp"

#include "prediction.hpp"

// A modular function in the case that a module changes this to something more exact
CMFunction<bool(CatEntity*)> OnGround {[](CatEntity* entity) {return DistanceToGround(entity) < 2.5f;}};

namespace pred {

// A CatEntity should have a group of points as well as a "time keeper", using an unordered_map is jsut for ease of use and should be phased out
struct PredictionCache_s {
  std::vector<CatVector> origins;
  short last;
};
// TODO, replace with fast map
static std::unordered_map<CatEntity*, PredictionCache_s> origin_cache;
static short time_keeper = 0;// We use this to keep time so we can

// Update cache for entity
static PredictionCache_s& UpdateCache(CatEntity* entity) {
  auto ent_find = origin_cache.find(entity);
  if (ent_find != origin_cache.end()) {
    // We dont need to update if we are on the same tick
    if (ent_find->second.last == time_keeper)
      return ent_find->second;
  } else
    ent_find = origin_cache.emplace(entity, PredictionCache_s()).first;
  // Update cache
  ent_find->second.origins.push_back(GetOrigin(entity));
  ent_find->second.last = time_keeper;
  return ent_find->second;
}

// Cache Cleanup duty
static void AfterWorldTick(){
  // Prune the cache of un-needed caches that havent been updated this tick
  for(auto i = origin_cache.begin(); i != origin_cache.end();) {
    if (i->second.last != time_keeper)
      origin_cache.erase(i);
    else {
      // prune old origins
      while (i->second.origins.size() > 2) i->second.origins.erase(i->second.origins.begin());
      i++;
    }
  }
  time_keeper++;
}

// We seperate gravity from velocity so we can do iteration prediction to it

// Gives the downward acceleration that a player recieves every worldtick
CMFunction<float(CatEntity*)> GetGravity {[](CatEntity* entity) -> float{
  const auto& p_data = UpdateCache(entity);
  if (p_data.origins.size() < 2)
    return 0;
  float down_velocity;
  for (size_t i = 0; i < p_data.origins.size() - 1; i++) {
    auto delta = p_data.origins[i].z - p_data.origins[i + 1].z;
    if (i == 0) // If its the first time looping, we cant average 2 deltas
      down_velocity = delta;
    else // meanwhile we average these
      down_velocity = (delta + down_velocity) * 0.5f;
  }
  return down_velocity;
}};

// Gives velocity of an object
CatVector GetVelocity(CatEntity* entity) {
  const auto& p_data = UpdateCache(entity);
  if (p_data.origins.size() < 2)
    return CatVector();
  // Generate velocity, NOTE: this is a general velocity, we need something better to account a change over time in the case of airstrafing. This is a maybe though...
  CatVector velocity;
  for (size_t i = 0; i < p_data.origins.size() - 1; i++) {
    auto delta = p_data.origins[i] - p_data.origins[i + 1];
    if (i == 0) // If its the first time looping, we cant average 2 deltas
      velocity = delta;
    else // meanwhile we average these
      velocity = (delta + velocity) * 0.5f;
  }
  return velocity;
}

// A prediction iteration function
// Iter is supposed to be a function to test whether to continue, return true to continue.
// This returns the end origin
CatVector PredIter(CatEntity* entity, bool(*iter)(CatEntity*, CatVector, int)){
  const auto& p_data = UpdateCache(entity);
  if (p_data.origins.size() < 2) // ensure we cached enough to get velocity and gravity
    return GetOrigin(entity);

  // This is the position that we will be iterating with
  CatVector cur_pos = GetOrigin(entity);
  CatVector cur_vel = GetVelocity(entity);
  float grav = GetGravity(entity);
  CatBox delta_box = GetCollision(entity) - cur_pos; // we will use this for collision checking
  for (int tick = 1; iter(entity, cur_pos, tick); tick++) {
    // check if airborne
    if (DistanceToGround(delta_box + cur_pos) < 1)
      cur_vel.z = 0; // set down velocity to 0 as we are on the ground
    else
      cur_vel.z += grav;// if we are airborne, we should apply gravity
    // TODO, collision check
    cur_pos + cur_vel; // add the velocity
  }
  return cur_pos;
}

void DrawTick(){
  /*for (int i = 0; i < GetEntityCount(); i++) {
    auto entity = GetEntity(i);
    if (!entity || GetDormant(entity) || !GetAlive(entity) || GetType(entity) != ETYPE_PLAYER) continue;
    static CatVector last;
    last = GetOrigin(entity);
    CatVector wts1;
    if (draw::WorldToScreen(last, wts1))
      draw::Rect(wts1.x - 4, wts1.y - 4, 8, 8, colors::white);
    if (draw::WorldToScreen(PredIter(entity, [](CatEntity*, CatVector cur_pos, int ii) -> bool{
      if (ii > 100) return false;
      CatVector wts1, wts2;
      if (draw::WorldToScreen(cur_pos, wts1) && draw::WorldToScreen(cur_pos, wts2))
        draw::Line(wts1.x, wts1.y, wts2.x - wts1.x, wts2.y - wts1.y, colors::white);
      last = cur_pos;
      return true;
    }), wts1))
      draw::Rect(wts1.x - 4, wts1.y - 4, 8, 8, colors::white);
  }*/
}
void Init(){
  wtickmgr.REventAfter(AfterWorldTick);
  //drawmgr.REventDuring(DrawTick);
}

}
