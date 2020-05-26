#include "common.hpp"
#include "teamroundtimer.hpp"

int CTeamRoundTimer::GetSetupTimeLength()
{
    IClientEntity *ent;
    ent = g_IEntityList->GetClientEntity(entity);
    if (!ent || ent->GetClientClass()->m_ClassID != CL_CLASS(CTeamRoundTimer))
        return -1;
    return NET_INT(ent, netvar.m_nSetupTimeLength);
};

round_states CTeamRoundTimer::GetRoundState()
{
    IClientEntity *ent;
    ent = g_IEntityList->GetClientEntity(entity);
    if (!ent || ent->GetClientClass()->m_ClassID != CL_CLASS(CTeamRoundTimer))
        return RT_STATE_NORMAL;
    int state = NET_INT(ent, netvar.m_nState);
    return state == 1 ? RT_STATE_NORMAL : RT_STATE_SETUP;
};

void CTeamRoundTimer::Update()
{
    IClientEntity *ent;

    entity = 0;
    for (int i = 0; i <= HIGHEST_ENTITY; i++)
    {
        ent = g_IEntityList->GetClientEntity(i);
        if (ent && ent->GetClientClass()->m_ClassID == CL_CLASS(CTeamRoundTimer))
        {
            entity = i;
            return;
        }
    }
}
CTeamRoundTimer *g_pTeamRoundTimer{ nullptr };

static InitRoutine init_trt([]() {
    EC::Register(
        EC::CreateMove, []() { g_pTeamRoundTimer->Update(); }, "update_teamroundtimer", EC::early);
});
