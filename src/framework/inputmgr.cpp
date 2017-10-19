

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
	
// Holds our cat keys string names for easy access
// TODO, use an arry for this instead
const static std::unordered_map<int, char*> KeyNames = {
		
	{CATKEY_NONE, " "}, 
	{CATKEY_0, "0"}, {CATKEY_1, "1"}, {CATKEY_2, "2"}, 
	{CATKEY_3, "3"}, {CATKEY_4, "4"}, {CATKEY_5, "5"}, 
	{CATKEY_6, "6"}, {CATKEY_7, "7"}, {CATKEY_8, "8"}, 
	{CATKEY_9, "9"}, {CATKEY_A, "A"}, {CATKEY_B, "B"}, 
	{CATKEY_C, "C"}, {CATKEY_D, "D"}, {CATKEY_E, "E"}, 
	{CATKEY_F, "F"}, {CATKEY_G, "G"}, {CATKEY_H, "H"}, 
	{CATKEY_I, "I"}, {CATKEY_J, "J"}, {CATKEY_K, "K"}, 
	{CATKEY_L, "L"}, {CATKEY_M, "M"}, {CATKEY_N, "N"}, 
	{CATKEY_O, "O"}, {CATKEY_P, "P"}, {CATKEY_Q, "Q"}, 
	{CATKEY_R, "R"}, {CATKEY_S, "S"}, {CATKEY_T, "T"}, 
	{CATKEY_U, "U"}, {CATKEY_V, "V"}, {CATKEY_W, "W"}, 
	{CATKEY_X, "X"}, {CATKEY_Y, "Y"}, {CATKEY_Z, "Z"}, 
	
	{CATKEY_LBRACKET, "LBRACKET"}, {CATKEY_RBRACKET, "RBRACKET"}, 
	{CATKEY_SEMICOLON, "SEMICOLON"}, {CATKEY_APOSTROPHE, "APOSTROPHE"}, 
	{CATKEY_BACKQUOTE, "BACKQUOTE"}, {CATKEY_COMMA, "COMMA"}, 
	{CATKEY_PERIOD, "PERIOD"}, {CATKEY_SLASH, "SLASH"}, 
	{CATKEY_BACKSLASH, "BACKSLASH"}, {CATKEY_MINUS, "MINUS"}, 
	{CATKEY_EQUAL, "EQUAL"}, {CATKEY_ENTER, "ENTER"}, 
	{CATKEY_SPACE, "SPACE"}, {CATKEY_BACKSPACE, "BACKSPACE"}, 
	{CATKEY_TAB, "TAB"}, {CATKEY_CAPSLOCK, "CAPSLOCK"}, 		
	
	{CATKEY_INSERT, "INSERT"}, {CATKEY_DELETE, "DELETE"}, 
	{CATKEY_HOME, "HOME"}, {CATKEY_END, "END"}, 
	{CATKEY_PAGEUP, "PAGEUP"}, {CATKEY_PAGEDOWN, "PAGEDOWN"}, 
	
	{CATKEY_LSHIFT, "LSHIFT"}, {CATKEY_RSHIFT, "RSHIFT"}, 
	{CATKEY_LALT, "LALT"}, {CATKEY_RALT, "RALT"}, 
	{CATKEY_LCONTROL, "LCONTROL"}, {CATKEY_RCONTROL, "RCONTROL"}, 

	{CATKEY_PAD_0, "PAD_0"}, {CATKEY_PAD_1, "PAD_1"}, {CATKEY_PAD_2, "PAD_2"}, 
	{CATKEY_PAD_3, "PAD_3"}, {CATKEY_PAD_4, "PAD_4"}, {CATKEY_PAD_5, "PAD_5"}, 
	{CATKEY_PAD_6, "PAD_6"}, {CATKEY_PAD_7, "PAD_7"}, {CATKEY_PAD_8, "PAD_8"}, 
	{CATKEY_PAD_9, "PAD_9"}, 
	
	{CATKEY_PAD_DIVIDE, "PAD_DIVIDE"}, {CATKEY_PAD_MULTIPLY, "PAD_MULTIPLY"}, 
	{CATKEY_PAD_MINUS, "PAD_MINUS"}, {CATKEY_PAD_PLUS, "PAD_PLUS"}, 
	{CATKEY_PAD_ENTER, "PAD_ENTER"}, {CATKEY_PAD_DECIMAL, "PAD_DECIMAL"}, 
	
	{CATKEY_UP, "UP"}, {CATKEY_LEFT, "LEFT"}, 
	{CATKEY_DOWN, "DOWN"}, {CATKEY_RIGHT, "RIGHT"}, 
	
	{CATKEY_F1, "F1"}, {CATKEY_F2, "F2"}, {CATKEY_F3, "F3"}, 
	{CATKEY_F4, "F4"}, {CATKEY_F5, "F5"}, {CATKEY_F6, "F6"}, 
	{CATKEY_F7, "F7"}, {CATKEY_F8, "F8"}, {CATKEY_F9, "F9"}, 
	{CATKEY_F10, "F10"}, {CATKEY_F11, "F11"}, {CATKEY_F12, "F12"}, 

	{CATKEY_MOUSE_1, "MOUSE_1"}, {CATKEY_MOUSE_2, "MOUSE_2"}, 
	{CATKEY_MOUSE_3, "MOUSE_3"}, {CATKEY_MOUSE_4, "MOUSE_4"}, 
	{CATKEY_MOUSE_5, "MOUSE_5"},
	{CATKEY_M_WHEEL_UP, "M_WHEEL_UP"}, {CATKEY_M_WHEEL_DOWN, "M_WHEEL_DOWN"}
};
	
// Input a keycode to get the name of it
const char* GetKeyName(int keycode) {
	
	auto find = KeyNames.find(keycode);		//Search the map for our keycode
	if(find != KeyNames.end()) return find->second;	// If we found an item, return the name
	return " "; // No name, return air
}

}