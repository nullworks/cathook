/*
  Created by Jenny White on 29.04.18.
  Copyright (c) 2018 nullworks. All rights reserved.
*/

#include <MiscTemporary.hpp>
#include <visual/SDLHooks.hpp>
#include <glez/draw.hpp>
#include "HookedMethods.hpp"
#include <SDL2/SDL_syswm.h>

static bool init{ false };
static bool init_wminfo{ false };
static SDL_SysWMinfo wminfo{};

int static_init_wminfo =
    (wminfo.version.major = 2, wminfo.version.minor = 0, 1);

typedef SDL_bool (*SDL_GetWindowWMInfo_t)(SDL_Window *window,
                                          SDL_SysWMinfo *info);
static SDL_GetWindowWMInfo_t GetWindowWMInfo = nullptr;
static SDL_GLContext tf2_sdl                 = nullptr;

namespace hooked_methods
{

DEFINE_HOOKED_METHOD(SDL_GL_SwapWindow, void, SDL_Window *window)
{
    if (!init_wminfo)
    {
        GetWindowWMInfo = *reinterpret_cast<SDL_GetWindowWMInfo_t *>(
            sharedobj::libsdl().Pointer(0xFD4D8));
        GetWindowWMInfo(window, &wminfo);
        init_wminfo = true;
    }
    if (!sdl_hooks::window)
        sdl_hooks::window = window;

    if (!tf2_sdl)
        tf2_sdl = SDL_GL_GetCurrentContext();

    if (isHackActive() && !disable_visuals)
    {
        PROF_SECTION(SWAPWINDOW_cathook);
        if (not init)
        {
            draw::InitGL();
            init = true;
        }
        draw::BeginGL();
        DrawCache();
        draw::EndGL();
    }
    {
        PROF_SECTION(SWAPWINDOW_tf2);
#if EXTERNAL_DRAWING
        SDL_GL_MakeCurrent(window, tf2_sdl);
#endif
        original::SDL_GL_SwapWindow(window);
        // glXMakeContextCurrent(wminfo.info.x11.display,
        // wminfo.info.x11.window,
        //                      wminfo.info.x11.window, tf2);
        // glXSwapBuffers(wminfo.info.x11.display, wminfo.info.x11.window);
    }
}
} // namespace hooked_methods
