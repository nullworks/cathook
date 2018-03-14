
/*
 *
 * This file is for giving access to main xlib stuff
 *
 *
 */

#include <cstring>
#include <X11/Xutil.h>		// So we can see process names, and geometry of windows

#include "../../util/logging.hpp"			// Logging is always cool!
#include "../../util/iohelper.hpp" 		// For our process name

#include "xlib.hpp"

namespace xlib {

Display* xAppDisplay;
Window xAppWindow;

bool GetXDisplay() {

 // If we dont have a display, attempt to get one.
 if (!xAppDisplay){
 	xAppDisplay = XOpenDisplay(nullptr); // Use the "DISPLAY" enviroment var for our display. This may not always be the case but its close enough!
 	if (!xAppDisplay) return false; // check if we recieved a display
 	g_CatLogging.log("Xlib Input: Recieved XDisplay!");
 }
 return true;
}

 // Searches for windows with a pid
static bool SearchForWindow(Window w = XDefaultRootWindow(xAppDisplay), const char* process_name = io::GetProcessName().c_str()) {
  if (!GetXDisplay()) return false; // Make sure we have a display

  // Get the process name for the current window
  XClassHint classhint;
  if (XGetClassHint(xAppDisplay, w, &classhint)) {
 	  if (classhint.res_class) {
      // Check if the process name of the window matches the one we are in
 		  if (!strcmp(classhint.res_class, process_name)) {
 			  xAppWindow = w;
 			  return true;
 		  }
    }
  }

  // Recurse into child windows.
 	Window wRoot, wParent; Window* wChild; unsigned nChildren;
 	if (XQueryTree(xAppDisplay, w, &wRoot, &wParent, &wChild, &nChildren) != 0) {
 		for (unsigned i = 0; i < nChildren; i++) {
 			if (SearchForWindow(wChild[i], process_name)) return true; // Since a child returned true, we return true to go up the chain
 		}
 	}
 	return false;
 }

bool GetXWindow() {
  // We need display to get a window
  if (!GetXDisplay()) return false;

 	// If we dont have the game window, we try to find it here!
 	if (!xAppWindow) {
 		// Attempt to get our window
 		if (!SearchForWindow()) return false;
 		g_CatLogging.log("Xlib Input: Recieved XWindow: 0x%x", xAppWindow);
 	}
 	return true;
}

}
