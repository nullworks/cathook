

#include "inputmgr.hpp"

// Input a keycode to determine whether key is pressed
bool CCatUserInp::IsKeyPressed(int keycode) {
	
	auto find = stored_pressed.find(keycode);		//Search the map for our keycode
	if(find != stored_pressed.end()) return true;	// If we found an item, key is pressed
	return false;
}

void CCatUserInp::Refresh() {
	stored_pressed.clear(); // Clear out the map
	if(!refresh_defined) return;
	StoredRefresh(this);	
}

void CCatUserInp::InitRefresh(void (*func)(CCatUserInp*)) {
	refresh_defined = true;
	StoredRefresh = func;
}
CCatUserInp	CatUserInp;

namespace CatKeys {
// Returns a nice string depending on the catkey input
// Bad way of doing this but idk at this point. Can always change it in the future
std::string GetKeyName(int in_key) {
		
	switch(in_key) {
	case CATKEY_NONE: return " ";
	case CATKEY_0: return "0";
	case CATKEY_1: return "1";
	case CATKEY_2: return "2";
	case CATKEY_3: return "3";
	case CATKEY_4: return "4";
	case CATKEY_5: return "5";
	case CATKEY_6: return "6";
	case CATKEY_7: return "7";
	case CATKEY_8: return "8";
	case CATKEY_9: return "9";
	case CATKEY_A: return "A";
	case CATKEY_B: return "B";
	case CATKEY_C: return "C";
	case CATKEY_D: return "D";
	case CATKEY_E: return "E";
	case CATKEY_F: return "F";
	case CATKEY_G: return "G";
	case CATKEY_H: return "H";
	case CATKEY_I: return "I";
	case CATKEY_J: return "J";
	case CATKEY_K: return "K";
	case CATKEY_L: return "L";
	case CATKEY_M: return "M";
	case CATKEY_N: return "N";
	case CATKEY_O: return "O";
	case CATKEY_P: return "P";
	case CATKEY_Q: return "Q";
	case CATKEY_R: return "R";
	case CATKEY_S: return "S";
	case CATKEY_T: return "T";
	case CATKEY_U: return "U";
	case CATKEY_V: return "V";
	case CATKEY_W: return "W";
	case CATKEY_X: return "X";
	case CATKEY_Y: return "Y";
	case CATKEY_Z: return "Z";
	
	case CATKEY_LBRACKET: return "LBRACKET";
	case CATKEY_RBRACKET: return "RBRACKET";
	case CATKEY_SEMICOLON: return "SEMICOLON";
	case CATKEY_APOSTROPHE: return "APOSTROPHE";
	case CATKEY_BACKQUOTE: return "BACKQUOTE";
	case CATKEY_COMMA: return "COMMA";
	case CATKEY_PERIOD: return "PERIOD";
	case CATKEY_SLASH: return "SLASH";
	case CATKEY_BACKSLASH: return "BACKSLASH";
	case CATKEY_MINUS: return "MINUS";
	case CATKEY_EQUAL: return "EQUAL";
	case CATKEY_ENTER: return "ENTER";
	case CATKEY_SPACE: return "SPACE";
	case CATKEY_BACKSPACE: return "BACKSPACE";
	case CATKEY_TAB: return "TAB";
	case CATKEY_CAPSLOCK: return "CAPSLOCK";		
	
	case CATKEY_INSERT: return "INSERT";
	case CATKEY_DELETE: return "DELETE";
	case CATKEY_HOME: return "HOME";
	case CATKEY_END: return "END";
	case CATKEY_PAGEUP: return "PAGEUP";
	case CATKEY_PAGEDOWN: return "PAGEDOWN";
	
	case CATKEY_LSHIFT: return "LSHIFT";
	case CATKEY_RSHIFT: return "RSHIFT";
	case CATKEY_LALT: return "LALT";
	case CATKEY_RALT: return "RALT";
	case CATKEY_LCONTROL: return "LCONTROL";
	case CATKEY_RCONTROL: return "RCONTROL";

	case CATKEY_PAD_0: return "PAD_0";
	case CATKEY_PAD_1: return "PAD_1";
	case CATKEY_PAD_2: return "PAD_2";
	case CATKEY_PAD_3: return "PAD_3";
	case CATKEY_PAD_4: return "PAD_4";
	case CATKEY_PAD_5: return "PAD_5";
	case CATKEY_PAD_6: return "PAD_6";
	case CATKEY_PAD_7: return "PAD_7";
	case CATKEY_PAD_8: return "PAD_8";
	case CATKEY_PAD_9: return "PAD_9";
	
	case CATKEY_PAD_DIVIDE: return "PAD_DIVIDE";
	case CATKEY_PAD_MULTIPLY: return "PAD_MULTIPLY";
	case CATKEY_PAD_MINUS: return "PAD_MINUS";
	case CATKEY_PAD_PLUS: return "PAD_PLUS";
	case CATKEY_PAD_ENTER: return "PAD_ENTER";
	case CATKEY_PAD_DECIMAL: return "PAD_DECIMAL";
	
	case CATKEY_UP: return "UP";
	case CATKEY_LEFT: return "LEFT";
	case CATKEY_DOWN: return "DOWN";
	case CATKEY_RIGHT: return "RIGHT";
	
	case CATKEY_F1: return "F1";
	case CATKEY_F2: return "F2";
	case CATKEY_F3: return "F3";
	case CATKEY_F4: return "F4";
	case CATKEY_F5: return "F5";
	case CATKEY_F6: return "F6";
	case CATKEY_F7: return "F7";
	case CATKEY_F8: return "F8";
	case CATKEY_F9: return "F9";
	case CATKEY_F10: return "F10";
	case CATKEY_F11: return "F11";
	case CATKEY_F12: return "F12";

	case CATKEY_MOUSE_1: return "MOUSE_1";
	case CATKEY_MOUSE_2: return "MOUSE_2";
	case CATKEY_MOUSE_3: return "MOUSE_3";
	case CATKEY_MOUSE_4: return "MOUSE_4";
	case CATKEY_MOUSE_5: return "MOUSE_5";
	case CATKEY_M_WHEEL_UP: return "M_WHEEL_UP";
	case CATKEY_M_WHEEL_DOWN: return "M_WHEEL_DOWN";
	}
	return " ";
}
}