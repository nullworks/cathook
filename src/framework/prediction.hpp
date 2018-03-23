
/*
 *
 *
 *
 */
#pragma once

#include "entitys.hpp"
#include "trace.hpp"

// Used for distance to ground, the following expands on that to make it more accurate
inline float DistanceToGround(CatVector point, int max = 8192) {
  // Get a point far below the "ground" then trace to it, return distance from start to dest
  return point.DistTo(trace::trace_terrain(point, util::ExtendLine(point, CatVector(89), max)));
}
inline float DistanceToGround(const CatBox& cat_box, int max = 8192) {
  float ret = max;
  // If we can get a collision, we test from the bottom points to the ground to get distance
  auto del = cat_box.max - cat_box.min;
  std::array<CatVector, 4> points;
  points[0] = cat_box.min;
  points[1] = cat_box.min + CatVector(del.x, 0, 0);
  points[2] = cat_box.min + CatVector(del.x, del.y, 0);
  points[3] = cat_box.min + CatVector(0, del.y, 0);
  // We use the smallest result as collision would "collide" with what it would hit
  for (const auto& i : points)
    ret = std::min(ret, DistanceToGround(i, ret));
  return ret;
}
inline float DistanceToGround(CatEntity* entity, int max = 8192) {
  // Collision is prefered to get accurate prediction
  auto coll = GetCollision(entity);
  if (coll != CatBox())
    return DistanceToGround(coll, max);
  else
    return DistanceToGround(GetOrigin(entity), max);
}

extern CMFunction<bool(CatEntity*)> OnGround;

namespace pred {

// Gives the downward acceleration that a player recieves every worldtick
extern CMFunction<float(CatEntity*)> GetGravity;

// Gives velocity of an object
CatVector GetVelocity(CatEntity* entity);

void Init();

}
