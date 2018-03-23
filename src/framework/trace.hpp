
/*
 *
 *
 *
 */

#pragma once

#include "../util/functions.hpp" // CMFunction
#include "../util/mathlib.hpp"   // CatVector
#include "entitys.hpp"

namespace trace {

// Trace line, returns end of trace
extern CMFunction<CatVector(CatVector, CatVector)> trace_terrain;

// Trace a line, returns entity hit or end of trace
extern CMFunction<std::pair<CatEntity*, CatVector>(CatVector, CatVector)> trace_sight;

// Check if line hits entity, this is for convienience
inline bool trace_entity(CatEntity* entity, CatVector src, CatVector dest) {
  auto trace_result = trace_sight(src, dest);
	return trace_result.first == entity || trace_result.second.DistTo(dest) < 1;
}

}
