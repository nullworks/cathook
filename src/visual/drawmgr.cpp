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
#include <glez/glez.hpp>
#include <glez/record.hpp>
#include <settings/Bool.hpp>
#include <menu/GuiInterface.hpp>
#include "common.hpp"
#include "visual/drawing.hpp"
#include "hack.hpp"

static settings::Bool info_text{ "hack-info.enable", "true" };
static settings::Bool info_text_min{ "hack-info.minimal", "false" };

void render_cheat_visuals()
{
    {
        PROF_SECTION(BeginCheatVisuals);
        BeginCheatVisuals();
    }
    {
        PROF_SECTION(DrawCheatVisuals);
        DrawCheatVisuals();
    }
    {
        PROF_SECTION(EndCheatVisuals);
        EndCheatVisuals();
    }
}

glez::record::Record bufferA{};
glez::record::Record bufferB{};

glez::record::Record *buffers[] = { &bufferA, &bufferB };
int currentBuffer               = 0;

void BeginCheatVisuals()
{
    buffers[currentBuffer]->begin();
    ResetStrings();
}

std::mutex drawing_mutex;

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
            AddSideString("cathook by nullifiedcat", colors::RainbowCurrent());
            if (!info_text_min)
            {
                AddSideString(hack::GetVersion(),
                              GUIColor()); // github commit and date
                AddSideString(hack::GetType(), GUIColor()); //  Compile type
#if ENABLE_GUI
                AddSideString(
                    "Press 'INSERT' or 'F11' key to open/close cheat menu.",
                    GUIColor());
                AddSideString("Use mouse to navigate in menu.", GUIColor());
#endif
                if (!g_IEngine->IsInGame()
#if ENABLE_GUI
/*
|| g_pGUI->Visible()
*/
#endif
                )
                {
                    // FIXME
                    /*name_s = *force_name;
                    if (name_s.length() < 3)
                        name_s = "*Not Set*";
                    reason_s   = disconnect_reason.GetString();
                    if (reason_s.length() < 3)
                        reason_s = "*Not Set*";
                    AddSideString(""); // foolish
                    AddSideString(format("Custom Name: ", name_s), GUIColor());
                    AddSideString(
                        format("Custom Disconnect Reason: ", reason_s),
                        GUIColor());*/
                }
            }
        }
    }
    if (spectator_target)
    {
        AddCenterString("Press SPACE to stop spectating");
    }
    if (CE_GOOD(g_pLocalPlayer->entity) && !g_Settings.bInvalid)
    {
        PROF_SECTION(PT_total_hacks);
        {
            PROF_SECTION(DRAW_aimbot);
            hacks::shared::aimbot::DrawText();
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
            PROF_SECTION(DRAW_lightesp);
            hacks::shared::lightesp::draw();
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
#if ENABLE_GUI
    {
        PROF_SECTION(DRAW_GUI);
        gui::draw();
    }
#endif
}

void EndCheatVisuals()
{
    buffers[currentBuffer]->end();
    currentBuffer = !currentBuffer;
}

void DrawCache()
{
    buffers[!currentBuffer]->replay();
}
