/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#pragma once

struct SDL_Window;

namespace sdl_hooks
{

extern SDL_Window *window;

void applySdlHooks();
void cleanSdlHooks();
} // namespace sdl_hooks