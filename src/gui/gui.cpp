
/*
 *
 *	This handles init and calling for draw from the drawmgr
 *
 */

#include "../util/catvars.hpp" 	// Catvars

#include "gui.hpp"


CatEnum menu_gui({ "Visuals", "GUI" });
CatVarColor CatGUIColor(menu_gui, "gui_color", colors::pink, "GUI Color", "Controls the color of the gui");	


namespace gui {
	
rgba_t GUIColor() {
	return (rgba_t)CatGUIColor;
}
	
}

