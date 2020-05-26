#pragma once
#include "common.hpp"

namespace hacks::tf2::backtrack
{
class CIncomingSequence
{
public:
    int inreliablestate;
    int sequencenr;
    float curtime;

    CIncomingSequence(int instate, int seqnr, float time)
    {
        inreliablestate = instate;
        sequencenr      = seqnr;
        curtime         = time;
    }
};

class hitboxData
{
public:
    Vector center{ 0.0f, 0.0f, 0.0f };
    Vector min{ 0.0f, 0.0f, 0.0f };
    Vector max{ 0.0f, 0.0f, 0.0f };
};

class BacktrackData
{
public:
    int tickcount{};

    std::array<hitboxData, 18> hitboxes{};
    Vector m_vecOrigin{};
    Vector m_vecAngles{};

    Vector m_vecMins{};
    Vector m_vecMaxs{};

    float m_flSimulationTime{};
    bool has_updated{};

    std::vector<matrix3x4_t> bones{};
};

// Stuff that has to be accessible from outside, mostly functions

extern settings::Float latency;
extern settings::Int bt_slots;
#if ENABLE_VISUALS
extern settings::Boolean chams;
extern settings::Int chams_ticks;
extern settings::Rgba chams_color;
extern settings::Boolean chams_solid;
#endif

// Check if backtrack is enabled
extern bool isBacktrackEnabled;
#if ENABLE_VISUALS
// Drawing Backtrack chams
extern bool isDrawing;
#endif
// Event callbacks
void CreateMove();
void CreateMoveLate();
#if ENABLE_VISUALS
void Draw();
#endif
void LevelShutdown();

void adjustPing(INetChannel *);
void updateDatagram();
void resetData(int);
bool isGoodTick(BacktrackData &);
bool defaultTickFilter(CachedEntity *, BacktrackData);
bool defaultEntFilter(CachedEntity *);

// Various functions for getting backtrack ticks
std::vector<BacktrackData> getGoodTicks(int);
std::optional<BacktrackData> getBestTick(CachedEntity *, std::function<bool(CachedEntity *, BacktrackData &, std::optional<BacktrackData> &)>);
std::optional<BacktrackData> getClosestEntTick(CachedEntity *, Vector, std::function<bool(CachedEntity *, BacktrackData)>);
std::optional<std::pair<CachedEntity *, BacktrackData>> getClosestTick(Vector, std::function<bool(CachedEntity *)>, std::function<bool(CachedEntity *, BacktrackData)>);

void SetBacktrackData(CachedEntity *ent, BacktrackData);
} // namespace hacks::tf2::backtrack
