/*
 * GUI.cpp
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#include "menu/GUI.h"
#include "menu/IWidget.h"

#include "common.hpp"
#include "core/sdk.hpp"

#undef RootWindow

void GUIVisibleCallback(IConVar *var, const char *pOldValue, float flOldValue)
{
    g_IInputSystem->SetCursorPosition(draw::width / 2, draw::height / 2);
    g_ISurface->SetCursor(vgui::CursorCode::dc_none);
    // g_IMatSystemSurface->SetCursorAlwaysVisible(false);
    if (gui_visible)
    {
        g_ISurface->UnlockCursor();
        g_ISurface->SetCursorAlwaysVisible(true);
        // g_IMatSystemSurface->UnlockCursor();
    }
    else
    {
        g_ISurface->LockCursor();
        g_ISurface->SetCursorAlwaysVisible(false);
        // g_IMatSystemSurface->LockCursor();
    }
}

CatVar gui_visible(CV_SWITCH, XORSTR("gui_visible"), XORSTR("0"), XORSTR("GUI Active"),
                   XORSTR("GUI switch (bind it to a key!)"));
CatVar gui_draw_bounds(CV_SWITCH, XORSTR("gui_bounds"), XORSTR("0"), XORSTR("Draw Bounds"),
                       XORSTR("Draw GUI elements' bounding boxes"));
CatCommand debug_invalid(XORSTR("debug_invalid"), XORSTR("Display all invalid CatVars"),
                         []() { menu::ncc::List::ShowInvalidCatVars(); });
CatCommand debug_missing(XORSTR("debug_missing"), XORSTR("Display all missing CatVars"),
                         []() { menu::ncc::List::ShowMissingCatVars(); });

CatGUI::CatGUI()
{
    root_nullcore = nullptr;
}

CatGUI::~CatGUI()
{
    delete root_nullcore;
}

bool CatGUI::Visible()
{
    return gui_visible;
}

CatVar gui_color_r(CV_INT, XORSTR("gui_color_r"), XORSTR("15"), XORSTR("Main GUI color (red)"),
                   XORSTR("Defines red component of main gui color"));
CatVar gui_color_g(CV_INT, XORSTR("gui_color_g"), XORSTR("150"), XORSTR("Main GUI color (green)"),
                   XORSTR("Defines green component of main gui color"));
CatVar gui_color_b(CV_INT, XORSTR("gui_color_b"), XORSTR("150"), XORSTR("Main GUI color (blue)"),
                   XORSTR("Defines blue component of main gui color"));
CatVar gui_color_a(CV_INT, XORSTR("gui_color_a"), XORSTR("130"), XORSTR("GUI Background Opacity"),
                   XORSTR("Defines the opacity of the GUI background (no shit?)"));
static CatVar gui_rainbow(CV_SWITCH, XORSTR("gui_rainbow"), XORSTR("0"), XORSTR("Rainbow GUI"),
                          XORSTR("RGB all the things!!!"));

int NCGUIColor()
{
    return gui_rainbow ? colorsint::RainbowCurrent()
                       : colorsint::Create((int) gui_color_r, (int) gui_color_g,
                                           (int) gui_color_b, 255);
}

void CatGUI::Setup()
{
    menu::ncc::Init();
    menu::ncc::List::ShowInvalidCatVars();
    root_nullcore = menu::ncc::root;
    gui_visible.OnRegister([](CatVar *var) {
        var->convar->InstallChangeCallback(GUIVisibleCallback);
    });
}

void CatGUI::Update()
{
    try
    {
        CBaseWindow *root = dynamic_cast<CBaseWindow *>(root_nullcore);
        int new_scroll =
            g_IInputSystem->GetAnalogValue(AnalogCode_t::MOUSE_WHEEL);
        // logging::Info(XORSTR("scroll: %i"), new_scroll);
        if (last_scroll_value < new_scroll)
        {
            // Scrolled up
            m_bPressedState[ButtonCode_t::MOUSE_WHEEL_DOWN] = false;
            m_bPressedState[ButtonCode_t::MOUSE_WHEEL_UP]   = true;
        }
        else if (last_scroll_value > new_scroll)
        {
            // Scrolled down
            m_bPressedState[ButtonCode_t::MOUSE_WHEEL_DOWN] = true;
            m_bPressedState[ButtonCode_t::MOUSE_WHEEL_UP]   = false;
        }
        else
        {
            // Didn't scroll
            m_bPressedState[ButtonCode_t::MOUSE_WHEEL_DOWN] = false;
            m_bPressedState[ButtonCode_t::MOUSE_WHEEL_UP]   = false;
        }

        last_scroll_value = new_scroll;
        for (int i = 0; i < ButtonCode_t::BUTTON_CODE_COUNT; i++)
        {
            bool down = false, changed = false;
            ;
            if (i != ButtonCode_t::MOUSE_WHEEL_DOWN &&
                i != ButtonCode_t::MOUSE_WHEEL_UP)
            {
                down    = g_IInputSystem->IsButtonDown((ButtonCode_t)(i));
                changed = m_bPressedState[i] != down;
            }
            else
            {
                down    = m_bPressedState[i];
                changed = down;
            }
            if (changed && down)
                m_iPressedFrame[i] = g_GlobalVars->framecount;
            m_bPressedState[i]     = down;
            if (m_bKeysInit)
            {
                if (changed)
                {
                    // logging::Info(XORSTR("Key %i changed! Now %i."), i, down);
                    if (i == ButtonCode_t::MOUSE_LEFT)
                    {
                        if (Visible())
                        {
                            if (down)
                                root->OnMousePress();
                            else
                                root->OnMouseRelease();
                        }
                    }
                    else
                    {
                        if ((i == ButtonCode_t::KEY_INSERT ||
                             i == ButtonCode_t::KEY_F11) &&
                            down)
                        {
                            gui_visible = !gui_visible;
                        }
                        if (Visible())
                        {
                            if (down)
                                root->OnKeyPress((ButtonCode_t) i, false);
                            else
                                root->OnKeyRelease((ButtonCode_t) i);
                        }
                    }
                }
                else
                {
                    if (down)
                    {
                        int frame =
                            g_GlobalVars->framecount - m_iPressedFrame[i];
                        bool shouldrepeat = false;
                        if (frame)
                        {
                            if (frame > 150)
                            {
                                if (frame > 400)
                                {
                                    if (frame % 30 == 0)
                                        shouldrepeat = true;
                                }
                                else
                                {
                                    if (frame % 80 == 0)
                                        shouldrepeat = true;
                                }
                            }
                        }
                        if (Visible())
                        {
                            if (shouldrepeat)
                                root->OnKeyPress((ButtonCode_t) i, true);
                        }
                    }
                }
            }
        }

        int nx = g_IInputSystem->GetAnalogValue(AnalogCode_t::MOUSE_X);
        int ny = g_IInputSystem->GetAnalogValue(AnalogCode_t::MOUSE_Y);

        mouse_dx = nx - m_iMouseX;
        mouse_dy = ny - m_iMouseY;

        m_iMouseX = nx;
        m_iMouseY = ny;

        if (!m_bKeysInit)
            m_bKeysInit = 1;
        if (!root->IsVisible())
            root->Show();
        root->Update();
        root->Draw(0, 0);
        if (Visible())
        {
            draw::DrawRect(m_iMouseX - 5, m_iMouseY - 5, 10, 10,
                           colorsint::Transparent(colorsint::white));
            draw::OutlineRect(m_iMouseX - 5, m_iMouseY - 5, 10, 10,
                              NCGUIColor());
        }
        if (gui_draw_bounds)
        {
            root->DrawBounds(0, 0);
        }
    }
    catch (std::exception &ex)
    {
        logging::Info(XORSTR("ERROR: %s"), ex.what());
    }
}

bool CatGUI::ConsumesKey(ButtonCode_t key)
{
    CBaseWindow *root = dynamic_cast<CBaseWindow *>(root_nullcore);
    if (root->IsVisible())
        return root->ConsumesKey(key);
    else
        return false;
}

CatGUI *g_pGUI = 0;
