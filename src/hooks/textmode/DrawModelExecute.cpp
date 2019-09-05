/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/
#include "HookedMethods.hpp"
#include "MiscTemporary.hpp"
namespace hooked_methods
{

DEFINE_HOOKED_METHOD(DrawModelExecute, void, IVModelRender *this_, const DrawModelState_t &state, const ModelRenderInfo_t &info, matrix3x4_t *bone)
{
    return;
}
} // namespace hooked_methods
