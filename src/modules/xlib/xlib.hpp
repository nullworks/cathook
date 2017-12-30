
/*
 *
 *
 *
 *
 */

#include <X11/Xlib.h>		// X11 lib for interacting with our session and other useful things to do with windows and the display manager

namespace xlib {

extern Display* xAppDisplay;
extern Window xAppWindow;

// Check if we have display/window.
bool GetXDisplay();
bool GetXWindow();

}
