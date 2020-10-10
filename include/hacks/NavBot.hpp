#pragma once

#include <array>
#include <stdint.h>

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
    outofbounds,
    engineer
};

enum engineer_task : uint8_t
{
    nothing = 0,
    // Build a new building
    goto_build_spot,
    // Go to an existing building
    goto_building,
    build_building,
    // Originally were going to be added seperately, but we already have autorepair and upgrade seperately
    // upgrade_building,
    // repair_building,
    upgradeorrepair_building,
    // Well time to just run at people and gun them (Rip old name: YEEEEEEEEEEEEEEHAW)
    staynear_engineer
};

extern engineer_task current_engineer_task;

struct Task
{
    task id;
    int priority;
    Task(task id)
    {
        this->id = id;
        priority = id == none ? 0 : 5;
    }
    Task(task id, int priority)
    {
        this->id       = id;
        this->priority = priority;
    }
    operator task()
    {
        return id;
    }
};

constexpr std::array<task, 3> blocking_tasks{ followbot, outofbounds, engineer };
extern Task current_task;
} // namespace task
struct bot_class_config
{
    float min;
    float preferred;
    float max;
};
} // namespace hacks::tf2::NavBot
