
/*
 *
 *
 *
 */

#include "entitys.hpp"
#include "trace.hpp"

inline float DistanceToGround(CatEntity* entity) {
  float ret = 8192;
  // If we can get a collision, we test from the bottom points to the ground to get distance
  auto coll = GetCollision(entity);
  if (coll != CatBox()) {
    auto del = coll.max - coll.min;
    std::array<CatVector, 4> points;
    points[0] = coll.min;
    points[1] = coll.min + CatVector(del.x, 0, 0);
    points[2] = coll.min + CatVector(del.x, del.y, 0);
    points[3] = coll.min + CatVector(0, del.y, 0);

    // We use the smallest result as collision would "collide" with what it would hit
    for (const auto& i : points) {
      ret = std::min(ret, i.DistTo(trace::trace_terrain(i, util::ExtendLine(i, CatVector(89), 8192.0f))));
    }
  } else {
    auto orig = GetOrigin(entity);
    ret = orig.DistTo(trace::trace_terrain(orig, util::ExtendLine(orig, CatVector(89), 8192.0f)));
  }

  return ret;
}
