
/*
 *
 *
 *
 */

#include "../util/functions.hpp" // CMFunction
#include "../util/mathlib.hpp"   // CatVector
#include "entitys.hpp"

namespace trace {

// Trace line to get whether it hit the end of the line
extern CMFunction<bool(const CatVector&, const CatVector&)> TraceLine;

// Trace line to get entity
extern CMFunction<const CatEntity*(const CatVector&, const CatVector&)> TraceLineToEnt;

// Trace a line, returns true if end of line or entity are hit
extern CMFunction<bool(const CatEntity&, const CatVector&, const CatVector&)> TraceEnt;

}
