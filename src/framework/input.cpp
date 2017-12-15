
/*
 *
 *	An input system for the cheat
 *
 *
 *
 */

#include "input.hpp"

namespace input {

// Used to store depressed keys
bool pressed_buttons[CATKEY_COUNT];

// Mouse info
std::pair<int, int> mouse(0, 0);

// Screen height and width
std::pair<int, int> bounds(0, 0);

// Holds our cat keys as strings
const char* key_names[] = {

	" ",
	"0", "1", "2",
	"3", "4", "5",
	"6", "7", "8",
	"9", "A", "B",
	"C", "D", "E",
	"F", "G", "H",
	"I", "J", "K",
	"L", "M", "N",
	"O", "P", "Q",
	"R", "S", "T",
	"U", "V", "W",
	"X", "Y", "Z",

	"ESCAPE",
	"LBRACKET", "RBRACKET",
	"SEMICOLON", "APOSTROPHE",
	"BACKQUOTE", "COMMA",
	"PERIOD", "SLASH",
	"BACKSLASH", "MINUS",
	"EQUAL", "ENTER",
	"SPACE", "BACKSPACE",
	"TAB", "CAPSLOCK",

	"INSERT", "DELETE",
	"HOME", "END",
	"PAGEUP", "PAGEDOWN",

	"LSHIFT", "RSHIFT",
	"LALT", "RALT",
	"LCONTROL", "RCONTROL",

	"PAD_0", "PAD_1", "PAD_2",
	"PAD_3", "PAD_4", "PAD_5",
	"PAD_6", "PAD_7", "PAD_8",
	"PAD_9",

	"PAD_DIVIDE", "PAD_MULTIPLY",
	"PAD_MINUS", "PAD_PLUS",
	"PAD_ENTER", "PAD_DECIMAL",

	"UP", "LEFT",
	"DOWN", "RIGHT",

	"F1", "F2", "F3",
	"F4", "F5", "F6",
	"F7", "F8", "F9",
	"F10", "F11", "F12",

	"MOUSE_1", "MOUSE_2",
	"MOUSE_3", "MOUSE_4",
	"MOUSE_5",
	"M_WHEEL_UP", "M_WHEEL_DOWN"
};

}
