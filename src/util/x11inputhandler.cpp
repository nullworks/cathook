 

#include <X11/Xlib.h>	// X11 lib for interacting with our session
#include <X11/Xutil.h>	// So we can see process names of windows


#include "iohelper.hpp" // For our process name
#include "../logging.h"

//http://www.xfree86.org/4.4.0/XQueryPointer.3.html

namespace InputHandler {
	
Display* xAppDisplay = nullptr;
Window*  xAppWindow  = nullptr;

bool GetXDisplay() {
	
	// If we dont have a display, attempt to get one.
	if (!xAppDisplay){
		xAppDisplay = XOpenDisplay(nullptr); // Use the "DISPLAY" enviroment var for our display. This may not always be the case but its close enough!
		if (!xAppDisplay) return false; // check if we recieved a display
		logging::Info("Recieved XDisplay...");
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
		logging::Info("Recieved XWindow...");
	}
	return true;
}
}