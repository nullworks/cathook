
/*
 *
 * 	This is the menu constructor for the gui. This build a menu our of elements for us to use.
 *
 */

#include <vector>

#include "../gui.hpp"
#include "../../managers/inputmgr.hpp"		// So we get input
#include "../../util/catvars.hpp"	// Obs we need it
#include "../base/elements.hpp"		// Include elements for building a gui
#include "menutree.hpp"				// This holds our menu information for us to build from

#include "menu.hpp"

// menus aspect ratio, dont judge
#define aspectx 13
#define aspecty 7

namespace gui { namespace menu {

// User vars
CatEnum gui_catenum({"Visuals", "Gui"});
CatVarBool menu_enabled(gui_catenum, "menu_visible", true, "Show Menu", "Should menu be visible?");
CatVarInt menu_x(gui_catenum, "menu_x", -20000, "Menu Location X", "Part of the position of the menu");
CatVarInt menu_y(gui_catenum, "menu_y", -20000, "Menu Location Y", "Part of the position of the menu");
CatVarInt menu_scale(gui_catenum, "menu_y", 100, "Menu Scaling", "Allows you to scale the menu.\n100 = 100% Scaling", 250);

// The root of cathooks menu
CBaseWidget* main_menu = nullptr;
	
// Used to store "where in the menu tree" we are
std::vector<int> menu_location;

// Used as a indicator of how much of the menu was used
static int menu_height_used = 0;
	
// We need a string pointer for top buttons
static std::string* string_top_ptr = new std::string("Top");

// Creates our top bars and it returns the menu tree it ends off on.
CMenuTree* CreateMenuBar(CMenuTree* menu_tree, int recursions) {
		
	// Make our button bar for us to use
	CBaseWidget* button_bar = element::ButtonBarCreate(main_menu);
	
	// If this is our first recursion, we can do some special things for the top bar
	if (recursions == 0) {
		button_bar->height = main_menu->width * 0.077; // 7.7:1 aspect ratio is good for top bar 
		button_bar->extra_ints[3] = 1;	// Allow the bar to stretch.
		
	// This isnt the top bar so we can do other stuff.
	} else {
		button_bar->height = main_menu->width * 0.05; // lower trees use 15:1
		button_bar->extra_ints[3] = 0;	// Dont allow the bar to stretch.
	}
	
	// Set the sizes that werent already set
	button_bar->rootx = 0;
	button_bar->width = main_menu->width;
	
	button_bar->rooty = menu_height_used;
	menu_height_used += button_bar->height * 0.5;	// Add to our used height. Devide by half fixes it somehow...
	
	// Add new buttons to the Bar 
	for (CMenuTree* tree_branch : menu_tree->children) {
		CBaseWidget* button = element::ButtonCreate(button_bar);
		button->name = tree_branch->name;
		button->extra_ints[0] = 1;	// Set the font
		button->extra_ints[1] = 24;	// Set the size
	}
	
	// If the location tree is more than we recursed, then we have to recurse again
	if (!menu_location.empty() && menu_location.size() > recursions) {
		// Check if the menu item exists
		if (menu_tree->children.size() >= menu_location[recursions])
			// Recurse
			CreateMenuBar(menu_tree->children[menu_location[recursions]], recursions + 1);
			return;
	}
	// We have finished recursing, return our menu branch.
}
	
// Clears the things owned by the menu and rebuilds it
void ReConstruct() {
	
	// Check if the main menu is created
	if (!main_menu) {
		main_menu = gui::element::RootCreate();
	}
	
	// Create a good size to the screen
	main_menu->height = (CatUserInp.boundsy * 0.45F); // Make it 38% of the screen, and scale with scale var
	main_menu->width = (main_menu->height / aspecty) * aspectx;	// Extrapolate with the aspect ratio
		
	// Movement handler.
	if (main_menu->performed_last) {
		menu_x = main_menu->rootx;
		menu_y = main_menu->rooty;
	} else {
		// Get a center if menu is at default position
		if (menu_x == -20000 || menu_y == -20000) { 
			menu_x = (CatUserInp.boundsx * 0.5) - (main_menu->width * 0.5);
			menu_y = (CatUserInp.boundsy * 0.5) - (main_menu->height * 0.5);
		}
		// Set root to catvars
		main_menu->rootx = menu_x;
		main_menu->rooty = menu_y;
	}
	
	// Delete existing elements
	if (!main_menu->child_widgets.empty()) {
		for (CBaseWidget* widget : main_menu->child_widgets) {
			DeleteWidgetTree(widget);
		}
	}
	
	// Construct Menu bar Tree
	CMenuTree* menu_tree = GetMenuTree();
	menu_height_used = 0;
	CMenuTree* current_branch = CreateMenuBar(menu_tree, 0);
		
	// Check if we dont have a position, if so, try to use default
	/*if (menu_location.empty()) {
		menu_location.push_back(0);	// Just use the first...
		
	}*/
	main_menu->color = GUIColor();
}

// This gets run on draw
void Draw() {
	
	// First init
	if (!main_menu) {
		ReConstruct();
	}
}
}}