
/*
 *
 *	A Root of a gui
 *
 */

#include "../../managers/inputmgr.hpp"		// So we get input
#include "../../managers/drawmodulemgr.hpp" // So we can draw

#include "root.hpp"

// TODO, make root as an base element

namespace gui { namespace element {
	
static int mouseoffsetx;
static int mouseoffsety;
	
void RootDraw(const CBaseWidget* base_widget) {
	if (base_widget == nullptr || !base_widget->visible) return;
	drawmgr::RectFilled(base_widget->rootx - 1, base_widget->rooty - 1, base_widget->width - 1, base_widget->height - 1, rgba_t(25, 25, 25, base_widget->color.a * 0.75));// Nice transparent dark gray inner box, 75% opacity
	drawmgr::Rect(base_widget->rootx, base_widget->rooty, base_widget->width, base_widget->height, base_widget->color);
}
	
bool RootHandleUi(CBaseWidget* base_widget) {
	if (base_widget == nullptr || !base_widget->visible) return false;
	if (CatUserInp.IsKeyPressed(CATKEY_MOUSE_1)) {	// Check for m1
		if (!base_widget->performed_last) {
			if (!(CatUserInp.mousex > base_widget->rootx && CatUserInp.mousey > base_widget->rooty && CatUserInp.mousex < base_widget->rootx + base_widget->width && CatUserInp.mousey < base_widget->rooty + base_widget->height)) return false; // check if in bounds
			mouseoffsetx = base_widget->rootx - CatUserInp.mousex;
			mouseoffsety = base_widget->rooty - CatUserInp.mousey;
			base_widget->performed_last = true;
			PushOnTop(base_widget);
		} else {
			base_widget->rootx = CatUserInp.mousex + mouseoffsetx;
			base_widget->rooty = CatUserInp.mousey + mouseoffsety;
		}
		return true;
	}
	base_widget->performed_last = false;
	return false; // TODO, make happen
}
	
CBaseWidget* RootCreate() {
	CBaseWidget* tmp = new CBaseWidget();
	tmp->usrinput = RootHandleUi;
	tmp->draw = RootDraw;
	return tmp;
}
}}