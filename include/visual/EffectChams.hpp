/*
 * EffectChams.hpp
 *
 *  Created on: Apr 16, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include "common.hpp"
#include "sdk/ScreenSpaceEffects.h"
#include "core/sdk.hpp"

namespace effect_chams
{

class EffectChams : public IScreenSpaceEffect
{
public:
    virtual void Init();
    inline virtual void Shutdown(){};

    inline virtual void SetParameters(KeyValues *params){};

    virtual void Render(int x, int y, int w, int h);

    inline virtual void Enable(bool bEnable)
    {
        enabled = bEnable;
    };
    inline virtual bool IsEnabled()
    {
        return enabled;
    };

    void SetEntityColor(CachedEntity *ent, rgba_t color);
    rgba_t ChamsColor(IClientEntity *entity);
    bool ShouldRenderChams(IClientEntity *entity);
    void RenderChams(IClientEntity *entity);
    void BeginRenderChams();
    void EndRenderChams();
    void RenderChamsRecursive(IClientEntity *entity);

public:
    bool init{ false };
    bool drawing{ false };
    bool enabled;
    float orig_modulation[3];
    CMaterialReference mat_unlit;
    CMaterialReference mat_unlit_z;
    CMaterialReference mat_lit;
    CMaterialReference mat_lit_z;
};

extern EffectChams g_EffectChams;
extern CScreenSpaceEffectRegistration *g_pEffectChams;
} // namespace effect_chams
