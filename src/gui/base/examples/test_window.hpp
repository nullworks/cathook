
/*
 *
 *  Run init() to get a test window
 *
 */

#pragma once

#include "../../gui.hpp"
#include "../CBaseWindow.hpp"

class CExampleWindow {

}

CExampleWindow* example_window = nullptr;

void Init() {
  if (example_window) return;
  example_window = new CExampleWindow("Example Window", g_pGui);
}
