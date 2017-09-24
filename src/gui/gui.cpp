
/*
 *
 *	This handles init and calling for draw from the drawmgr
 *
 */

#include "../util/catvars.hpp" 	// Catvars

#include "gui.hpp"


CatEnum menu_gui({ "Visuals", "GUI" });
CatVarColor CatGUIColor(menu_gui, "gui_color", rgba_t(255, 0, 240, 255), "GUI Color", "Controls the color of the gui");	


namespace gui {
	
rgba_t GUIColor() {
	return (rgba_t)CatGUIColor;
}
	
}




// TODO
/*make color from CatEntity function								-Done
make userinput handler / make modules give us info
Make gui
make gui have color selector thing


look at ncc gui to find out how it converted its rainbow

look at logging for time info


google hina logi




first, get how im gonna scale it, then make root.
make button objects, make a gui builder to use catvars and link them to the buttons.
*/