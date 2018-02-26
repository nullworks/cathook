
/*
 *
 * Ray tracing is used to get visibility checks on things ingame.
 * This file is what handles all of the potential commands needed for such actions.
 *
 *
 */

#include "trace.hpp"

namespace trace {

// Trace line, returns end of trace
CMFunction<CatVector(CatVector, CatVector)> trace_terrain {
  [](CatVector, CatVector dest) -> auto {return dest;}
};

// Trace a line, returns entity hit or end of trace
CMFunction<std::pair<CatEntity*, CatVector>(CatVector, CatVector)> trace_sight {
  [](CatVector, CatVector dest) -> std::pair<CatEntity*, CatVector> {
    return {nullptr, dest};} // Return dest to make everything think vis check worked
};

}
