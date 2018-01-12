
/*
 *
 * Ray tracing is used to get visibility checks on things ingame.
 * This file is what handles all of the potential commands needed for such actions.
 *
 *
 */

#include "trace.hpp"

namespace trace {

// Trace line to get whether it hit the end of the line
CMFunction<bool(CatVector, CatVector)> TraceLine {[](CatVector, CatVector) -> bool { return true; }};

// Trace line to get entity
CMFunction<CatEntity*(CatVector, CatVector)> TraceLineToEnt {[](CatVector, CatVector) -> CatEntity* {return nullptr;}};

// Trace a line, returns true if end of line or entity are hit
CMFunction<bool(CatEntity*, CatVector, CatVector)> TraceEnt {[](CatEntity*, CatVector, CatVector) -> bool {return true;}};

}
