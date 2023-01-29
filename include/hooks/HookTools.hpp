#pragma once

#include "core/profiler.hpp"
#include <array>

namespace EC
{

enum ec_types
{
    /* Note: engine prediction is run on this kind of CreateMove */
    CreateMove = 0,
    /* Note: this is the CreateMove one layer higher, and should only be used for things that mess with command number*/
    CreateMoveLate,
    /* This kind of CreateMove will run earlier than "CreateMove" events
     * and guranteed to run before EnginePrediction
     * This is NEEDED for any kind of movement
     */
    CreateMove_NoEnginePred,
    /* Note: this is still CreateMove, just ran before original is called, needed in some cases like changing tickcount before original gets called*/
    CreateMoveEarly,
    /* Note: This will replace the normal CreateMove when we are warping. Used for performance purposes. */
    CreateMoveWarp,
#if ENABLE_VISUALS
    Draw,
#endif
    Paint,
    LevelInit,
    FirstCM,
    LevelShutdown,
    Shutdown,
    /* Append new event above this line. Line below declares amount of events */
    EcTypesSize
};

enum ec_priority
{
    very_early = -2,
    early,
    average,
    late,
    very_late
};

typedef void (*EventFunction)();
void Register(enum ec_types type, const EventFunction &function, const std::string &name, enum ec_priority priority = average);
void Unregister(enum ec_types type, const std::string &name);
void run(enum ec_types type);
} // namespace EC
