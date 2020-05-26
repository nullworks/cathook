#pragma once
enum round_states
{
    RT_STATE_SETUP = 0,
    RT_STATE_NORMAL
};

class CTeamRoundTimer
{
public:
    int GetSetupTimeLength();
    round_states GetRoundState();
    void Update();

    int entity;
};
extern CTeamRoundTimer *g_pTeamRoundTimer;
