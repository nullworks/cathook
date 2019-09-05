#pragma once

#include "common.hpp"
namespace hacks::tf2::NavBot
{
bool init(bool first_cm);
namespace task
{
enum task : uint8_t
{
    none = 0,
    sniper_spot,
    stay_near,
    health,
    ammo,
    dispenser,
    followbot,
    outofbounds
};
struct Task
{
    task id;
    int priority;
    Task(task _id)
    {
        id       = _id;
        priority = _id == none ? 0 : 5;
    }
    Task(task _id, int _priority)
    {
        id       = _id;
        priority = _priority;
    }
    operator task()
    {
        return id;
    }
};
constexpr std::array<task, 2> blocking_tasks{ followbot, outofbounds };
extern Task current_task;
} // namespace task
struct bot_class_config
{
    float min;
    float preferred;
    float max;
};
} // namespace hacks::tf2::NavBot
