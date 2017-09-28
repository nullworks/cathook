
/*
 *
 * 	This is the menu constructor for the gui. This build a menu our of elements for us to use.
 *
 */

#include <vector>

#include "../../logging.h"
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
CatEnum gui_catenum({"Visuals", "Menu"});
CatVarBool menu_enabled(gui_catenum, "menu_visible", false, "Show Menu", "Should menu be visible?");
CatVarInt menu_x(gui_catenum, "menu_x", -20000, "Menu Location X", "Part of the position of the menu");
CatVarInt menu_y(gui_catenum, "menu_y", -20000, "Menu Location Y", "Part of the position of the menu");
CatVarInt menu_scale(gui_catenum, "menu_y", 100, "Menu Scaling", "Allows you to scale the menu.\n100 = 100% Scaling", 250);
CatVarInt font(gui_catenum, "menu_font", 1, "Menu Font", "Choose the menu font");
CatVarInt font_size(gui_catenum, "menu_font_size", 19, "Menu Font Size", "Choose a size for the fonts", 40);

// The root of cathooks menu
CBaseWidget* main_menu = nullptr;
	
// Used to store "where in the menu tree" we are
std::vector<int> menu_location;

// Used as a indicator of how much of the menu was used
static int menu_height_used = 0;

// Creates our top bars and it returns the menu tree it ends off on.
CMenuTree* CreateMenuBar(CMenuTree* menu_tree, int recursions) {
		
	// Make our button bar for us to use
	CBaseWidget* button_bar = element::ButtonBarCreate(main_menu);
	
	// If this is our first recursion, we can do some special things for the top bar
	if (recursions == 0) {
		button_bar->height = main_menu->width * 0.077; // 7.7:1 aspect ratio is good for top bar 
		button_bar->extra_ints[3] = 1;	// Allow the bar to stretch.
		menu_height_used = 0;
		
	// This isnt the top bar so we can do other stuff.
	} else {
		button_bar->height = main_menu->width * 0.05; // lower trees use 15:1
		button_bar->extra_ints[3] = 0;	// Dont allow the bar to stretch.
	}
	
	button_bar->width = main_menu->width;
	button_bar->rooty = menu_height_used;
	menu_height_used += (button_bar->height * 0.5); // If this is the start, we set our used height here
	
	// Add new buttons to the Bar 
	int i = 0;
	for (CMenuTree* tree_branch : menu_tree->children) {
		CBaseWidget* button = element::ButtonCreate(button_bar);
		button->name = tree_branch->name;
		button->font = 1;	// Set the font
		button->font_size = 24;	// Set the size
		if (recursions > 0) {
			for (int ii = 0; ii < recursions; ii++) {
				button->menu_identifier.push_back(menu_location[ii]);
			}
		}
		button->menu_identifier.push_back(i);
		i++;
	}
	
	// If the location tree is more than we recursed, then we have to recurse again
	if (!menu_location.empty() && menu_location.size() > recursions) {
		// Check if the menu item exists
		if (menu_tree->children.size() >= menu_location[recursions]) {
			
			// Make a divider to seperate the top bar of the menu
			CBaseWidget* divider = element::DividerCreate(main_menu);
			divider->rooty = menu_height_used * 2 - 1;
			divider->width = main_menu->width;
			
			// Recurse
			CreateMenuBar(menu_tree->children[menu_location[recursions]], recursions + 1);
		}
	}
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
		
	// Movement handler, so that we can link it to a catvar
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
		main_menu->child_widgets.clear();
	}
	
	// Construct Menu bar Tree
	CMenuTree* menu_tree = GetMenuTree();	// This gets a fresh tree from the tree generator
	CMenuTree* current_branch = CreateMenuBar(menu_tree, 0);	// This returns us a menu_branch that contains catvars to put into the menu
	
	
	CBaseWidget* button = element::ButtonToggleCreate(main_menu);
	button->name = "Suidice";
	button->rootx = 5;
	button->rooty = menu_height_used * 2 + 5;
	button->font = font;
	button->font_size = font_size;
	static bool child_butte = false;
	button->child_bool = &child_butte;
}


	
	
// This gets run on draw
void Draw() {
	
	// First init
	if (main_menu == nullptr) {
		ReConstruct();
	}
	
	// Menu key handler
	static bool last_keystate = false;
	if (last_keystate && CatUserInp.IsKeyPressed(CATKEY_INSERT)) {
		menu_enabled = !menu_enabled;
	}
	last_keystate = CatUserInp.IsKeyPressed(CATKEY_INSERT);
	
	// update menu states
	main_menu->visible = menu_enabled;
	main_menu->color = GUIColor();
	
	// User input handler
	static bool last_preformed = false; // Useful var for input handler
	if (main_menu->child_widgets.empty()) return;
	// Here we should be looping through button bars, -1 for the element organizer
	for (CBaseWidget* base_widget : main_menu->child_widgets) { 
		if (!base_widget->child_widgets.empty()) {
			// Here, inside the button bar, we check for depressed buttons.
			for (CBaseWidget* widget : base_widget->child_widgets) { 
				if (widget->performed_last) { // We found a button that preformed
					if (!last_preformed) {	// We only want to do stuff if last time didnt go through to prevent "looping" from one click.
						last_preformed = true;
						// Swap the identifiers position with our current menu
						menu_location.swap(widget->menu_identifier);
						ReConstruct(); // We need to reconstruct
					}
					return; // Return to prevent last preformed from resetting
				}
			}
		}
	}
	last_preformed = false;
}
	
	
	
	
	
	
}}