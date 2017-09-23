 
#include <X11/keysymdef.h>
#include <X11/Xlib.h>	// X11 lib for interacting with our session
#include <X11/Xutil.h>	// So we can see process names, and geometry of windows
#include <unordered_map>// map


#include "../../logging.h"
#include "../../util/iohelper.hpp" 		// For our process name
#include "../../util/stringhelpers.hpp" // format()
#include "../../managers/inputmgr.hpp" 	// For our Catkey enums

#include "xlibinput.hpp"

#define XK_leftarrow                     0x08fb /* U+2190 LEFTWARDS ARROW */
#define XK_uparrow                       0x08fc  /* U+2191 UPWARDS ARROW */
#define XK_rightarrow                    0x08fd  /* U+2192 RIGHTWARDS ARROW */
#define XK_downarrow                     0x08fe

Display* xAppDisplay = nullptr;
Window*  xAppWindow  = nullptr;

bool GetXDisplay() {
	
	// If we dont have a display, attempt to get one.
	if (!xAppDisplay){
		xAppDisplay = XOpenDisplay(nullptr); // Use the "DISPLAY" enviroment var for our display. This may not always be the case but its close enough!
		if (!xAppDisplay) return false; // check if we recieved a display
		logging::Info("Recieved XDisplay!");
	}
	return true;
}
	
// Searches for windows with a pid
bool SearchForWindow(Window w, std::string process_name) {
	if (!GetXDisplay()) return false; // Make sure we have a display
	
    // Get the process name for the current window
	XClassHint classhint;
	if (XGetClassHint(xAppDisplay, w, &classhint)) {
		if (classhint.res_class) {
			if (classhint.res_class == process_name) {
				xAppWindow = w;
				return true;
			}
		}
	}

    // Recurse into child windows.
	Window wRoot; Window wParent;
	Window *wChild; unsigned nChildren;
	if (0 != XQueryTree(xAppDisplay, w, &wRoot, &wParent, &wChild, &nChildren)) {
		for (unsigned i = 0; i < nChildren; i++) {
			if (SearchForWindow(wChild[i], process_name)) return true; // Since a child returned true, we return true to go up the chain
		}
	}
	return false;
}

bool GetXWindow() {
	if (!GetXDisplay()) return false;
	
	// If we dont have the game window, we try to find it here!
	if (!xAppWindow){
		
		// Attempt to get our window
		if (!SearchForWindow(XDefaultRootWindow(xAppDisplay), GetProcessName())) return false;
		
		if (!xAppWindow) return false;// Do we have game window?
		logging::Info(format("Recieved XWindow: ", xAppWindow).c_str());
	}
	return true;
}

// Stores potential conversions between xlib's keycodes and cathooks catvars. Add more as nessesary! /usr/include/X11/keysymdef.h
std::unordered_map<int, int> xlibToCatVar = {
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
	
	{XK_uparrow, CATKEY_UP}, {XK_leftarrow, CATKEY_LEFT}, // These wouldnt go through so i manually put them in
	{XK_downarrow, CATKEY_DOWN}, {XK_rightarrow, CATKEY_RIGHT},
	
	{XK_F1, CATKEY_F1}, {XK_F2, CATKEY_F2}, {XK_F3, CATKEY_F3},
	{XK_F4, CATKEY_F4}, {XK_F5, CATKEY_F5}, {XK_F6, CATKEY_F6},
	{XK_F7, CATKEY_F7}, {XK_F8, CATKEY_F8}, {XK_F9, CATKEY_F9},
	{XK_F10, CATKEY_F10}, {XK_F11, CATKEY_F11}, {XK_F12, CATKEY_F12},

	{XK_Pointer_Button4, CATKEY_MOUSE_4}, {XK_Pointer_Button5, CATKEY_MOUSE_5},
	{XK_Pointer_DfltBtnPrev, CATKEY_M_WHEEL_UP}, {XK_Pointer_DfltBtnNext, CATKEY_M_WHEEL_DOWN}
};

int ConvertButtons(std::string in_string) { // Convert xlibkeys to catkeys
		
	auto find = xlibToCatVar.find(in_string.c_str());		//Search the map for our keycode
	if(find != xlibToCatVar.end()) return 0;
	
	return find->second;
}

// Request this to update the input system on button, mouse, and window info
void Refresh(CCatUserInp *input_class) {
	
	// Ensure we have a window
	if (!GetXWindow()) return;
	
	// Get window bounds
	Window root_return; int rel_x; int rel_y; unsigned int border; unsigned int depth; int tmpx; int tmpy; 
	if (XGetGeometry(xAppDisplay, xAppWindow, &root_return, &rel_x, &rel_y, &tmpx, &tmpy, &border, &depth)) {
		input_class->boundsx = tmpx;
		input_class->boundsy = tmpy;
	}
	
	// Update mouse position
	input_class->mouse_on_screen = false;
	Window child_return; int root_x; int root_y; unsigned int mask_return; 
	if (XQueryPointer(xAppDisplay, xAppWindow, &root_return, &child_return, &root_x, &root_y, &tmpx, &tmpy, &mask_return)) { // only update the cursor if this returns true
		if (!(tmpx <= 0 || tmpy <= 0 || tmpx >= input_class->boundsx || tmpy >= input_class->boundsy)) { // Clamp positions to the window
			input_class->mousex = tmpx;
			input_class->mousey = tmpy;
			input_class->mouse_on_screen = true;				
		}
		// We did a pointer query so check our buttons too!
		if (mask_return & (Button1Mask)) input_class->stored_pressed.insert({{CATKEY_MOUSE_1}, {true}});
		if (mask_return & (Button2Mask)) input_class->stored_pressed.insert({{CATKEY_MOUSE_2}, {true}});
		if (mask_return & (Button3Mask)) input_class->stored_pressed.insert({{CATKEY_MOUSE_3}, {true}});
		//if (mask_return & (Button4Mask)) input_class->stored_pressed.insert(CATKEY_MOUSE_4, true); // Doesnt work
		//if (mask_return & (Button5Mask)) input_class->stored_pressed.insert(CATKEY_MOUSE_5, true); 
	}
	
	// Find depressed keys and save them to the stored map
	char keys[32];
	XQueryKeymap(xAppDisplay, keys);
	for( const auto& current : xlibToCatVar ) {	// Recurse through the map looking for depressed keys
		// Sue me i dare you
		//if (keys & (current.first)) input_class->stored_pressed.insert({{current.second}, {true}}); // Commented so i can compile
	}
}



