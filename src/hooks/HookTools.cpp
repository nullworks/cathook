#include "common.hpp"
#include "HookTools.hpp"

namespace EC
{

struct EventCallbackData
{
    explicit EventCallbackData(const EventFunction &function, std::string name, enum ec_priority priority) : function{ function }, priority{ int(priority) }, section(name), event_name{ name }
    {
    }
    EventFunction function;
    int priority;
    ProfilerSection section;
    std::string event_name;
};

static std::vector<EventCallbackData> events[ec_types::EcTypesSize];
CatCommand evt_print("debug_print_events", "Print EC events",
                     []()
                     {
                         for (int i = 0; i < int(ec_types::EcTypesSize); ++i)
                         {
                             logging::Info("%d events:", i);

                             for (auto it = events[i].begin(); it != events[i].end(); ++it)
                                 logging::Info("%s", it->event_name.c_str());
                             logging::Info("");
                         }
                     });

void Register(enum ec_types type, const EventFunction &function, const std::string &name, enum ec_priority priority)
{
    events[type].emplace_back(function, name, priority);
    // Order vector to always keep priorities correct
    std::sort(events[type].begin(), events[type].end(), [](EventCallbackData &a, EventCallbackData &b) { return a.priority < b.priority; });
}

void Unregister(enum ec_types type, const std::string &name)
{
    auto &e = events[type];
    for (auto it = e.begin(); it != e.end(); ++it)
        if (it->event_name == name)
        {
            e.erase(it);
            break;
        }
}

void run(ec_types type)
{
    auto &vector = events[type];
    for (auto &i : vector)
    {
#if ENABLE_PROFILER
        volatile ProfilerNode node(i.section);
#endif
        i.function();
    }
}

} // namespace EC
