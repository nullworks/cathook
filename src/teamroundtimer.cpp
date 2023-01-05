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

    entity = 0;
    for (auto const &ent : entity_cache::valid_ents)
    {
        auto result_ent = ent->InternalEntity();
        if (ent && result_ent->GetClientClass()->m_ClassID == CL_CLASS(CTeamRoundTimer))
        {
            entity = ent->m_IDX;
            return;
        }
    }
}
CTeamRoundTimer *g_pTeamRoundTimer{ nullptr };

static InitRoutine init_trt(
    []()
    {
        EC::Register(
            EC::CreateMove, []() { g_pTeamRoundTimer->Update(); }, "update_teamroundtimer", EC::early);
    });
