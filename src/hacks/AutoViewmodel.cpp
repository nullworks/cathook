#include "common.hpp"

namespace hacks::tf2::autoviewmodel
{

static settings::Boolean auto_viewmodel_flipper{ "misc.auto-flip-viewmodel", "false" };

static void CreateMove()
{
    if (!auto_viewmodel_flipper)
        return;
    static auto cl_flipviewmodels = g_ICvar->FindVar("cl_flipviewmodels");
    if (!cl_flipviewmodels)
    {
        cl_flipviewmodels = g_ICvar->FindVar("cl_flipviewmodels");
        return;
    }
    static bool defaults = cl_flipviewmodels->GetBool();

    // Should we test if a flip would help?
    bool should_test_viewmodel = true;
    for (auto const &ent: entity_cache::player_cache)
    {
        
        if (CE_BAD(ent) || !ent->m_bEnemy() || !ent->m_bAlivePlayer() || ent == LOCAL_E)
            continue;
        auto eye   = g_pLocalPlayer->v_Eye;
        auto angle = GetAimAtAngles(eye, ent->m_vecOrigin());
        eye        = getShootPos(angle);

        if (IsVectorVisible(eye, ent->m_vecOrigin()))
        {
            should_test_viewmodel = false;
            break;
        }
    }
    bool keep_flipped = false;
    // Noone is visible, test if flipping helps
    if (should_test_viewmodel)
    {
        cl_flipviewmodels->SetValue(!cl_flipviewmodels->GetBool());
        for (auto const &ent: entity_cache::player_cache)
        {
            if (CE_BAD(ent) || !ent->m_bEnemy() || !ent->m_bAlivePlayer() || ent == LOCAL_E)
                continue;
            auto eye   = g_pLocalPlayer->v_Eye;
            auto angle = GetAimAtAngles(eye, ent->m_vecOrigin());
            eye        = getShootPos(angle);

            if (IsVectorVisible(eye, ent->m_vecOrigin()))
            {
                keep_flipped = true;
                break;
            }
        }
    }

    // Reset status as noone is in range
    if (!keep_flipped && should_test_viewmodel)
        cl_flipviewmodels->SetValue(defaults);
}

static InitRoutine init([]() { EC::Register(EC::CreateMove, CreateMove, "viewmodel_flip_cm"); });
} // namespace hacks::tf2::autoviewmodel
