
#include "../../util/catvars.hpp"
#include "../base/elements.hpp"		// Include elements for building a gui

#include "menu.hpp"

namespace gui { namespace menu {

// aspect ratio, i like 11:7
const int aspectx = 11;
const int aspecty = 7;

// The root of cathooks menu
//gui::element::CRoot cathook_menu();
gui::element::CBaseWidget* main_menu = nullptr;
	
	
void Construct() {
	if (!main_menu) main_menu = new gui::element::CBaseWidget();
	main_menu->widthx = 400;
	main_menu->widthy = 250;	
	
}
void Init() {

	
	
}
}}