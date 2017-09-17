/*
 * sdl.cpp
 *
 *  Created on: May 19, 2017
 *      Author: nullifiedcat
 */

#include "hookedmethods.h"
#include "../common.h"
#include "../hack.h"

#include <link.h>

#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include "../ftrender.hpp"

SDL_PollEvent_t* SDL_PollEvent_loc { nullptr };
SDL_GL_SwapWindow_t* SDL_GL_SwapWindow_loc { nullptr };
SDL_PollEvent_t SDL_PollEvent_o { nullptr };
SDL_GL_SwapWindow_t SDL_GL_SwapWindow_o { nullptr };

int SDL_PollEvent_hook(SDL_Event* event) {
	int retval = SDL_PollEvent_o(event);
	if (event && (event->key.keysym.sym & ~SDLK_SCANCODE_MASK) < 512) {
		ImGui_ImplSdl_ProcessEvent(event);
	}
	return retval;
}

#include <GL/gl.h>
#include "../atlas.hpp"
#include "../drawgl.hpp"
#include "../gui/im/Im.hpp"

void SDL_GL_SwapWindow_hook(SDL_Window* window) {
	static SDL_GLContext ctx_tf2 = SDL_GL_GetCurrentContext();
	static SDL_GLContext ctx_imgui = nullptr;
	static SDL_GLContext ctx_text = nullptr;
	if (!disable_visuals) {
		PROF_SECTION(DRAW_cheat);
		if (!ctx_imgui) {
			ctx_imgui = SDL_GL_CreateContext(window);
			ImGui_ImplSdl_Init(window);
			ctx_text = SDL_GL_CreateContext(window);
			FTGL_Init();
			textures::Init();
			drawgl::Initialize();
		}

		if (!cathook) {
			SDL_GL_MakeCurrent(window, ctx_tf2);
			SDL_GL_SwapWindow_o(window);
			return;
		}

		SDL_GL_MakeCurrent(window, ctx_text);
		{
			std::lock_guard<std::mutex> draw_lock(drawing_mutex);
			drawgl::PreRender();
			{
				PROF_SECTION(DRAW_gl);
				drawgl::Render();
			}

			{
				PROF_SECTION(DRAW_freetype);
				FTGL_Render();
			}

			drawgl::PostRender();
		}
		SDL_GL_MakeCurrent(window, ctx_imgui);
		{
			PROF_SECTION(DRAW_imgui);
			ImGui_ImplSdl_NewFrame(window);
			menu::im::Render();
			ImGui::Render();
		}
	}
	{
		PROF_SECTION(DRAW_valve);
		SDL_GL_MakeCurrent(window, ctx_tf2);
		SDL_GL_SwapWindow_o(window);
	}
}

void DoSDLHooking() {
	SDL_GL_SwapWindow_loc = reinterpret_cast<SDL_GL_SwapWindow_t*>(sharedobj::libsdl().Pointer(0xFD648));
	SDL_PollEvent_loc = reinterpret_cast<SDL_PollEvent_t*>(sharedobj::libsdl().Pointer(0xFCF64));

	SDL_GL_SwapWindow_o = *SDL_GL_SwapWindow_loc;
	SDL_PollEvent_o = *SDL_PollEvent_loc;

	/*char patch_1[4];
	char patch_2[4];
	*((unsigned*)patch_1) = (unsigned)SDL_GL_SwapWindow_hook;
	*((unsigned*)patch_2) = (unsigned)SDL_PollEvent_hook;

	Patch(SDL_GL_SwapWindow_loc, patch_1, 4);
	Patch(SDL_PollEvent_loc, patch_2, 4);*/

	*SDL_GL_SwapWindow_loc = SDL_GL_SwapWindow_hook;
	*SDL_PollEvent_loc = SDL_PollEvent_hook;
}

void DoSDLUnhooking() {
	*SDL_GL_SwapWindow_loc = SDL_GL_SwapWindow_o;
	*SDL_PollEvent_loc = SDL_PollEvent_o;
}
