/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include "SDLHooks.hpp"
#include "HookedMethods.hpp"

namespace sdl_hooks
{

SDL_Window *window{ nullptr };

namespace pointers
{
hooked_methods::types::SDL_GL_SwapWindow *SDL_GL_SwapWindow{ nullptr };
hooked_methods::types::SDL_PollEvent *SDL_PollEvent{ nullptr };
} // namespace pointers

void applySdlHooks()
{
    pointers::SDL_GL_SwapWindow =
        reinterpret_cast<hooked_methods::types::SDL_GL_SwapWindow *>(
            sharedobj::libsdl().Pointer(0xFD648));

    hooked_methods::original::SDL_GL_SwapWindow = *pointers::SDL_GL_SwapWindow;
    *pointers::SDL_GL_SwapWindow = hooked_methods::methods::SDL_GL_SwapWindow;

    pointers::SDL_PollEvent =
        reinterpret_cast<hooked_methods::types::SDL_PollEvent *>(
            sharedobj::libsdl().Pointer(0xFCF64));

    hooked_methods::original::SDL_PollEvent = *pointers::SDL_PollEvent;
    *pointers::SDL_PollEvent = hooked_methods::methods::SDL_PollEvent;
}

void cleanSdlHooks()
{
    *pointers::SDL_GL_SwapWindow = hooked_methods::original::SDL_GL_SwapWindow;
}
} // namespace sdl_hooks