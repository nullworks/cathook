#pragma once

#include "core/profiler.hpp"
#include "functional"
#include <array>

namespace EC
{

enum ec_types
{
    /* Note: engine prediction is run on this kind of CreateMove */
    CreateMove = 0,
    /* This kind of CreateMove will run earlier than all CreateMove events
     * and guranteed to run before EnginePrediction
     */
    CreateMove_NoEnginePred,
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

typedef std::function<void()> EventFunction;
void Register(enum ec_types type, const EventFunction &function, const std::string &name, enum ec_priority priority = average);
void Unregister(enum ec_types type, const std::string &name);
void run(enum ec_types type);
} // namespace EC
