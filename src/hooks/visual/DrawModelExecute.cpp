/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <MiscTemporary.hpp>
#include <settings/Bool.hpp>
#include "HookedMethods.hpp"

static settings::Bool no_arms{ "remove.arms", "false" };
static settings::Bool no_hats{ "remove.hats", "false" };

namespace hooked_methods
{

DEFINE_HOOKED_METHOD(DrawModelExecute, void, IVModelRender *this_,
                     const DrawModelState_t &state,
                     const ModelRenderInfo_t &info, matrix3x4_t *bone)
{
    if (!isHackActive())
        return;

    if (!(spectator_target || no_arms || no_hats ||
          (clean_screenshots && g_IEngine->IsTakingScreenshot()) ||
          CE_BAD(LOCAL_E) || !LOCAL_E->m_bAlivePlayer()))
    {
        return original::DrawModelExecute(this_, state, info, bone);
    }

    PROF_SECTION(DrawModelExecute);

    if (no_arms || no_hats)
    {
        if (info.pModel)
        {
            const char *name = g_IModelInfo->GetModelName(info.pModel);
            if (name)
            {
                std::string sname = name;
                if (no_arms && sname.find("arms") != std::string::npos)
                {
                    return;
                }
                else if (no_hats &&
                         sname.find("player/items") != std::string::npos)
                {
                    return;
                }
            }
        }
    }

    IClientUnknown *unk = info.pRenderable->GetIClientUnknown();
    if (unk)
    {
        IClientEntity *ent = unk->GetIClientEntity();
        if (ent)
        {
            if (ent->entindex() == spectator_target)
            {
                return;
            }
        }
        if (ent && !effect_chams::g_EffectChams.drawing &&
            effect_chams::g_EffectChams.ShouldRenderChams(ent))
        {
            return;
        }
    }

    return original::DrawModelExecute(this_, state, info, bone);
}
} // namespace hooked_methods
