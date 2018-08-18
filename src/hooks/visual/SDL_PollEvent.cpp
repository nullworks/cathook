/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <menu/GuiInterface.hpp>
#include "HookedMethods.hpp"

namespace hooked_methods
{

DEFINE_HOOKED_METHOD(SDL_PollEvent, int, SDL_Event *event)
{
    auto ret = original::SDL_PollEvent(event);
#if ENABLE_GUI
    if (gui::handleSdlEvent(event))
        return 0;
#endif
    return ret;
}
} // namespace hooked_methods