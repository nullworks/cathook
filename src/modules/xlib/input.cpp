
/*
 *
 *	A libary for reading user input using the xlib sdk.
 *	All credits goto onee-chan :3
 *	I worked fucking hard on this >_>
 *
 *
 */

#include <X11/Xutil.h> // For xlibs keycodes, window geometry
#include <bitset> // For caching pressed keys

#include "../../framework/input.hpp" // To manipulate the input framework ofc
#include "../../framework/gameticks.hpp" 	// For init
#include "xlib.hpp" // so we can get window and display

#include "input.hpp"

// These arent defined by default so I do that here
#define XK_leftarrow                     0x08fb
#define XK_uparrow                       0x08fc
#define XK_rightarrow                    0x08fd
#define XK_downarrow                     0x08fe

namespace xlib {

//TODO: xlib - FastMap instead of this mess
// Stores potential conversions between xlib's keycodes and cathooks catkeyss. Add more as nessesary! /usr/include/X11/keysymdef.h
static const std::pair<int, CatKey> xlibToCatVar[] = {
	{XK_0, CATKEY_0}, {XK_1, CATKEY_1}, {XK_2, CATKEY_2},
	{XK_3, CATKEY_3}, {XK_4, CATKEY_4}, {XK_5, CATKEY_5},
	{XK_6, CATKEY_6}, {XK_7, CATKEY_7}, {XK_8, CATKEY_8},
	{XK_9, CATKEY_9}, {XK_A, CATKEY_A}, {XK_B, CATKEY_B},
	{XK_C, CATKEY_C}, {XK_D, CATKEY_D}, {XK_E, CATKEY_E},
	{XK_F, CATKEY_F}, {XK_G, CATKEY_G}, {XK_H, CATKEY_H},
	{XK_I, CATKEY_I}, {XK_J, CATKEY_J}, {XK_K, CATKEY_K},
	{XK_L, CATKEY_L}, {XK_M, CATKEY_M}, {XK_N, CATKEY_N},
	{XK_O, CATKEY_O}, {XK_P, CATKEY_P}, {XK_Q, CATKEY_Q},
	{XK_R, CATKEY_R}, {XK_S, CATKEY_S}, {XK_T, CATKEY_T},
	{XK_U, CATKEY_U}, {XK_V, CATKEY_V}, {XK_W, CATKEY_W},
	{XK_X, CATKEY_X}, {XK_Y, CATKEY_Y}, {XK_Z, CATKEY_Z},

	{XK_Escape, CATKEY_ESCAPE}, {XK_bracketleft, CATKEY_LBRACKET},
	{XK_bracketright, CATKEY_RBRACKET}, {XK_semicolon, CATKEY_SEMICOLON},
	{XK_apostrophe, CATKEY_APOSTROPHE}, {XK_apostrophe, CATKEY_COMMA},
	{XK_period, CATKEY_PERIOD}, {XK_slash, CATKEY_SLASH},
	{XK_backslash, CATKEY_BACKSLASH}, {XK_minus, CATKEY_MINUS},
	{XK_equal, CATKEY_EQUAL}, {XK_Return, CATKEY_ENTER},
	{XK_space, CATKEY_SPACE}, {XK_BackSpace, CATKEY_BACKSPACE},
	{XK_Tab, CATKEY_TAB}, {XK_Caps_Lock, CATKEY_CAPSLOCK},

	{XK_Insert, CATKEY_INSERT}, {XK_Delete, CATKEY_DELETE},
	{XK_Home, CATKEY_HOME}, {XK_End, CATKEY_END},
	{XK_Page_Up, CATKEY_PAGEUP}, {XK_Page_Down, CATKEY_PAGEDOWN},

	{XK_Shift_L, CATKEY_LSHIFT}, {XK_Shift_R, CATKEY_RSHIFT},
	{XK_Alt_L, CATKEY_LALT}, {XK_Alt_R, CATKEY_RALT},
	{XK_Control_L, CATKEY_LCONTROL}, {XK_Control_R, CATKEY_RCONTROL},

	{XK_KP_0, CATKEY_PAD_0}, {XK_KP_1, CATKEY_PAD_1}, {XK_KP_2, CATKEY_PAD_2},
	{XK_KP_3, CATKEY_PAD_3}, {XK_KP_4, CATKEY_PAD_4}, {XK_KP_5, CATKEY_PAD_5},
	{XK_KP_6, CATKEY_PAD_6}, {XK_KP_7, CATKEY_PAD_7}, {XK_KP_8, CATKEY_PAD_8},
	{XK_KP_9, CATKEY_PAD_9},

	{XK_KP_Divide, CATKEY_PAD_DIVIDE}, {XK_KP_Multiply, CATKEY_PAD_MULTIPLY},
	{XK_KP_Subtract, CATKEY_PAD_MINUS},	{XK_KP_Add, CATKEY_PAD_PLUS},
	{XK_KP_Enter, CATKEY_PAD_ENTER}, {XK_KP_Decimal, CATKEY_PAD_DECIMAL},

	{XK_uparrow, CATKEY_UP}, {XK_leftarrow, CATKEY_LEFT},
	{XK_downarrow, CATKEY_DOWN}, {XK_rightarrow, CATKEY_RIGHT},

	{XK_F1, CATKEY_F1}, {XK_F2, CATKEY_F2}, {XK_F3, CATKEY_F3},
	{XK_F4, CATKEY_F4}, {XK_F5, CATKEY_F5}, {XK_F6, CATKEY_F6},
	{XK_F7, CATKEY_F7}, {XK_F8, CATKEY_F8}, {XK_F9, CATKEY_F9},
	{XK_F10, CATKEY_F10}, {XK_F11, CATKEY_F11}, {XK_F12, CATKEY_F12},

	{XK_Pointer_DfltBtnPrev, CATKEY_M_WHEEL_UP}, {XK_Pointer_DfltBtnNext, CATKEY_M_WHEEL_DOWN}
};

// Used to store depressed keys
static std::bitset<CATKEY_COUNT> pressed_buttons;

// Mouse info
static std::pair<int, int> mouse(-1, -1);

// Screen height and width
static std::pair<int, int> bounds(-1, -1);

// Request this to update the input system on button, mouse, and window info
static void RefreshState() {

	// Ensure we have a window
	if (!GetXWindow()) return;

	// Get window bounds
	Window root_return; int rel_x, rel_y; unsigned int border, depth, boundsx, boundsy;
	if (XGetGeometry(xAppDisplay, xAppWindow, &root_return, &rel_x, &rel_y, &boundsx, &boundsy, &border, &depth)) {
		bounds = std::make_pair(boundsx, boundsy);
		input::bounds_event(bounds);
	}

	// Update mouse position
	Window child_return; int root_x, root_y, mousex, mousey; unsigned int mask_return;
	if (XQueryPointer(xAppDisplay, xAppWindow, &root_return, &child_return, &root_x, &root_y, &mousex, &mousey, &mask_return)) { // only update the cursor if this returns true
		if (mousex!=mouse.first||mousey!=mouse.second||!(mousex <= 0 || mousey <= 0 || mousex >= bounds.first || mousey >= bounds.second)) { // Clamp positions to the window
			mouse = std::make_pair(mousex, mousey);
			input::mouse_event(mouse);
		}
		// We did a pointer query so check our buttons too!
		bool s = (mask_return & (Button1Mask));
		if (s != pressed_buttons[CATKEY_MOUSE_1]){
			pressed_buttons[CATKEY_MOUSE_1] = s;
			input::key_event(CATKEY_MOUSE_1, s);
		}
		s = (mask_return & (Button2Mask));
		if (s != pressed_buttons[CATKEY_MOUSE_2]){
			pressed_buttons[CATKEY_MOUSE_2] = s;
			input::key_event(CATKEY_MOUSE_2, s);
		}
		s = (mask_return & (Button3Mask));
		if (s != pressed_buttons[CATKEY_MOUSE_3]){
			pressed_buttons[CATKEY_MOUSE_3] = s;
			input::key_event(CATKEY_MOUSE_3, s);
		}
	}

	// Find depressed keys and save them to the stored map
	char keys[32];
	XQueryKeymap(xAppDisplay, keys);
	// Recurse through the map looking for depressed keys
	for (const auto& current : xlibToCatVar) {

		// Get the keycode for the key we are looking for...
		int current_key = XKeysymToKeycode(xAppDisplay, current.first);

		// Use the keymap with bitwise logic to get state, oof this took forever to make
		bool s = (keys[current_key / 8] & (1 << (current_key % 8)));
		if (s != pressed_buttons[current.second]){
			pressed_buttons[current.second]=s;
			input::key_event(current.second, s);
		}
	}
}

static bool GetKey(CatKey k){
	return pressed_buttons[k];
}

static std::pair<int,int> GetMouse(){
	return mouse;
}

static std::pair<int,int> GetBounds(){
	return bounds;
}

// Simple init function to enable xlib input functionality
void InitInput() {
	drawmgr.REventBefore(RefreshState);
	input::GetKey = GetKey;
	input::GetMouse = GetMouse;
	input::GetBounds = GetBounds;
}

}
