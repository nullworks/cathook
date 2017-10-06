
/*
 *
 *	This handles init and calling for draw from the drawmgr
 *
 */

#include "gui.hpp"

CatEnum menu_gui({ "Visuals", "GUI" });
CatVarColor CatGUIColor(menu_gui, "gui_color", colors::pink, "GUI Color", "Controls the color of the gui");	

namespace gui {

// These are externed
CatVarInt gui_font(menu_gui, "gui_font", 1, "Font", "Choose the global font");
CatVarInt gui_font_size(menu_gui, "gui_font_size", 19, "Font Size", "Choose a size for the fonts", 40);
	
rgba_t GUIColor() {
	return (rgba_t)CatGUIColor;
}
	
}

