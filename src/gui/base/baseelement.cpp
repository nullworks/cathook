 
/*
 *
 *	A base widget to build your other types around.
 *
 */

#include "divider.hpp"


namespace gui { namespace element {

CBaseWidget::CBaseWidget(CRoot* root_parent, int layer, void(*draw)(const CBaseWidget*, rgba_t&)) 
	: root_parent(root_parent), layer(layer), draw(draw){ 					
	CBaseWidgetList().push_back(this); // When creating this, i want to push it to the list
}
	
std::vector<CBaseWidget*>& CBaseWidgetList();
}}