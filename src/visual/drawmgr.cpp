/*
 * drawmgr.cpp
 *
 *  Created on: May 22, 2017
 *      Author: nullifiedcat
 */

#include <MiscTemporary.hpp>
#include <hacks/Misc.hpp>
#include <hacks/Aimbot.hpp>
#include <hacks/hacklist.hpp>
#include "common.hpp"
#include "visual/drawing.hpp"
#include "hack.hpp"

void render_cheat_visuals()
{
    {
        PROF_SECTION(BeginCheatVisuals);
        BeginCheatVisuals();
    }
    //    xoverlay_draw_rect(300, 300, 100, 100, xoverlay_rgba(200, 100, 100,
    //    255));
    // draw_api::draw_string(100, 100, XORSTR("Testing"), fonts::main_font,
    // colors::white);
    //    static draw_api::font_handle_t fh = draw_api::create_font(DATA_PATH
    //    XORSTR("/fonts/tf2build.ttf"), 14); xoverlay_draw_string(100, 100,
    //    XORSTR("TestingSTR"), fh.handle, *reinterpret_cast<const xoverlay_rgba_t
    //    *>(&colors::white), 0, 0);
    // xoverlay_draw_string_with_outline(100, 20, XORSTR("Testing2"), )
    {
        PROF_SECTION(DrawCheatVisuals);
        DrawCheatVisuals();
    }
    {
        PROF_SECTION(EndCheatVisuals);
        EndCheatVisuals();
    }
}

void BeginCheatVisuals()
{
    /*#if RENDERING_ENGINE_OPENGL
        std::lock_guard<std::mutex> draw_lock(drawing_mutex);
    #endif*/
    draw_api::draw_begin();
    ResetStrings();
}

std::mutex drawing_mutex;

static CatVar info_text(CV_SWITCH, XORSTR("info"), XORSTR("1"), XORSTR("Show info"),
                        XORSTR("Show cathook version in top left corner"));
static CatVar info_text_min(CV_SWITCH, XORSTR("info_min"), XORSTR("0"), XORSTR("Show minimal info"),
                            XORSTR("Only show cathook title in top left corner"));
static CatVar enable_logo(CV_SWITCH, XORSTR("nullcore_mode_logo"), XORSTR("1"),
                          XORSTR("Enable Nullcore watermark"), XORSTR(""));

void DrawCheatVisuals()
{
    /*#if RENDERING_ENGINE_OPENGL
        std::lock_guard<std::mutex> draw_lock(drawing_mutex);
    #endif*/
    {
        PROF_SECTION(DRAW_misc);
        hacks::shared::misc::DrawText();
    }
    {
        PROF_SECTION(DRAW_info);
        std::string name_s, reason_s;
        PROF_SECTION(PT_info_text);
        if (info_text)
        {
            AddSideString(XORSTR("cathook by nullifiedcat"), colors::RainbowCurrent());
            if (!info_text_min)
            {
                AddSideString(hack::GetVersion(),
                              GUIColor()); // github commit and date
                AddSideString(hack::GetType(), GUIColor()); //  Compile type
#if ENABLE_GUI
                AddSideString(
                    XORSTR("Press 'INSERT' or 'F11' key to open/close cheat menu."),
                    GUIColor());
                AddSideString(XORSTR("Use mouse to navigate in menu."), GUIColor());
#endif
                if (!g_IEngine->IsInGame()
#if ENABLE_GUI
/*
|| g_pGUI->Visible()
*/
#endif
                        )
                {
                    name_s = force_name.GetString();
                    if (name_s.length() < 3)
                        name_s = XORSTR("*Not Set*");
                    reason_s   = disconnect_reason.GetString();
                    if (reason_s.length() < 3)
                        reason_s = XORSTR("*Not Set*");
                    AddSideString(XORSTR("")); // foolish
                    AddSideString(format(XORSTR("Custom Name: "), name_s), GUIColor());
                    AddSideString(
                        format(XORSTR("Custom Disconnect Reason: "), reason_s),
                        GUIColor());
                }
            }
        }
    }
    if (spectator_target)
    {
        AddCenterString(XORSTR("Press SPACE to stop spectating"));
    }
    if (CE_GOOD(g_pLocalPlayer->entity) && !g_Settings.bInvalid)
    {
        PROF_SECTION(PT_total_hacks);
        {
            PROF_SECTION(DRAW_aimbot);
            hacks::shared::aimbot::DrawText();
        }
        {
            PROF_SECTION(DRAW_lagexploit)
            hacks::shared::lagexploit::Draw();
        }
        IF_GAME(IsTF2())
        {
            PROF_SECTION(DRAW_skinchanger);
            hacks::tf2::skinchanger::DrawText();
        }
#ifndef FEATURE_RADAR_DISABLED
        IF_GAME(IsTF())
        {
            PROF_SECTION(DRAW_radar);
            hacks::tf::radar::Draw();
        }
#endif
        IF_GAME(IsTF())
        {
            PROF_SECTION(DRAW_autoreflect);
            hacks::tf::autoreflect::Draw();
        }
        IF_GAME(IsTF2())
        {
            PROF_SECTION(DRAW_backtracc);
            hacks::shared::backtrack::Draw();
        }
        IF_GAME(IsTF2())
        {
            PROF_SECTION(DRAW_healarrows);
            hacks::tf2::healarrow::Draw();
        }
        {
            PROF_SECTION(DRAW_walkbot);
            hacks::shared::walkbot::Draw();
        }
        IF_GAME(IsTF())
        {
            PROF_SECTION(PT_antidisguise);
            hacks::tf2::antidisguise::Draw();
        }
        IF_GAME(IsTF())
        {
            PROF_SECTION(PT_spyalert);
            hacks::tf::spyalert::Draw();
        }
#if ENABLE_IPC
        IF_GAME(IsTF()) hacks::shared::followbot::DrawTick();
#endif
        {
            PROF_SECTION(DRAW_esp);
            hacks::shared::esp::Draw();
        }
        IF_GAME(IsTF2())
        {
            criticals::draw();
        }
#ifndef FEATURE_FIDGET_SPINNER_ENABLED
        DrawSpinner();
#endif
        Prediction_PaintTraverse();
    }
    {
        PROF_SECTION(DRAW_strings);
        DrawStrings();
    }
}

void EndCheatVisuals()
{
    draw_api::draw_end();
}
