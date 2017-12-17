
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
CMFunction<bool(const CatVector&, const CatVector&)> TraceLine {[](const CatVector&, const CatVector&) -> bool { return true; }};

// Trace line to get entity
CMFunction<const CatEntity*(const CatVector&, const CatVector&)> TraceLineToEnt {[](const CatVector&, const CatVector&) -> const CatEntity* {return nullptr;}};

// Trace a line, returns true if end of line or entity are hit
CMFunction<bool(const CatEntity&, const CatVector&, const CatVector&)> TraceEnt {[](const CatEntity&, const CatVector&, const CatVector&) -> bool {return true;}};

}
