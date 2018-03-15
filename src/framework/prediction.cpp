
/*
 *
 *  This should have multiple values such as gravity, air resist and so on, to eventually make a iteration prediction engine using various info from the game
 *
 *
 */

#include <chrono>
#include <unordered_map>

#include "gameticks.hpp"

#include "prediction.hpp"

// A modular function in the case that a module changes this to something more exact
CMFunction<bool(CatEntity*)> OnGround {[](CatEntity* entity) {return DistanceToGround(entity) < 2.5f;}};

namespace pred {

// A CatEntity should have a group of points as well as a "time keeper", using an unordered_map is jsut for ease of use and should be phased out
struct PredictionCache_s {
  std::vector<CatVector> origins;
  std::vector<std::chrono::time_point<std::chrono::steady_clock>> time_points;
  short last;
};
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
  ent_find->second.time_points.push_back(std::chrono::steady_clock::now());
  ent_find->second.last = time_keeper;
  return ent_find->second;
}

// Cleanup duty
static void AfterWorldTick(){
  // Prune the cache of un-needed caches that havent been updated this tick
  for(auto i = origin_cache.begin(); i != origin_cache.end();) {
    if (i->second.last != time_keeper)
      origin_cache.erase(i);
    else {
      // prune old origins
      while (i->second.origins.size() > 2) i->second.origins.erase(i->second.origins.begin());
      while (i->second.time_points.size() > 2) i->second.time_points.erase(i->second.time_points.begin());
      i++;
    }
  }
  time_keeper++;
}

// Gives the downward acceleration that a player recieves every worldtick
CMFunction<float(CatEntity*)> GetGravity{[](auto){return 0.0f;}}; // TODO!

// Gives velocity of an object
CatVector GetVelocity(CatEntity* entity) {
  const auto p_data = UpdateCache(entity);
  if (p_data.origins.size() < 2)
    return CatVector();
  // Generate velocity, NOTE: this is a general velocity, we need something better to account a change over time in the case of airstrafing. This is a maybe though...
  CatVector velocity;
  for (size_t i = 0; i < p_data.origins.size() - 1; i++) {
    auto delta = p_data.origins.at(i) - p_data.origins.at(i + 1);
    if (i == 0) // If its the first time looping, we cant average 2 deltas
      velocity = delta;
    else // meanwhile we average these
      velocity = (delta + velocity) * 0.5f;
  }
  return velocity;
}

void Init(){
  wtickmgr.REventAfter(AfterWorldTick);
}

}
