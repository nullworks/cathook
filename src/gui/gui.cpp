
/*
 * GUI.cpp
 *
 *  Created on: Jan 25, 2017
 *      Author: nullifiedcat
 */

#include "gui.hpp"

namespace gui {
CatEnum gui_menu({"GUI"}); // Menu locator for esp settings
CatVarKey mousekey(gui_menu, "gui_mousekey", CATKEY_CAPSLOCK, "GUI Mouse Toggle Button", "Freezes player look & unlocks mouse. (Keyboard shortcuts always work, ignorant of this setting)");
CatVarKey activatekey(gui_menu, "gui_activatekey", CATKEY_RIGHT, "GUI Activate Button", "Actuates. Presses buttons, opens submenus, etc.");
CatVarKey backkey(gui_menu, "gui_backkey", CATKEY_LEFT, "GUI Back-out Button", "Escapes. Closes submenus, deselects containers, etc.");
CatVarKey nextkey(gui_menu, "gui_nextkey", CATKEY_DOWN, "GUI Next Button", "Next element in container.");
CatVarKey prevkey(gui_menu, "gui_prevkey", CATKEY_UP, "GUI Prev Button", "Previous element in container.");
CatVarKey field_leftkey(gui_menu, "gui_field_lefttkey", CATKEY_LEFT, "GUI Field Left Button", "In a text field, moves cursor left.");
CatVarKey field_rightkey(gui_menu, "gui_field_rightkey", CATKEY_RIGHT, "GUI Field Right Button", "In a text field, moves cursor right.");
CatVarKey field_escapekey(gui_menu, "gui_field_escapekey", CATKEY_ENTER, "GUI Field Finish Button", "In a text field, releases focus.");
base::CBaseContainer g_pGui("Root");
}
