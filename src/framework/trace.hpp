
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
extern CMFunction<bool(CatVector, CatVector)> TraceLine;

// Trace line to get entity
extern CMFunction<CatEntity*(CatVector, CatVector)> TraceLineToEnt;

// Trace a line, returns true if end of line or entity are hit
extern CMFunction<bool(CatEntity*, CatVector, CatVector)> TraceEnt;

}
