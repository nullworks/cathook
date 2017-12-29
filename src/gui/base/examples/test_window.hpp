
/*
 *
 *  Run init() to get a test window
 *
 */

#pragma once

#include "../../gui.hpp"
#include "../CBaseWindow.hpp"

namespace gui {
using namespace base;

class CExampleWindow : CBaseWindow {
public:
CExampleWindow() {
  AddChild(new CBaseTitleBar("Example Window"));
}

};

CExampleWindow* example_window = nullptr;

void Init() {
  if (example_window) return;
  example_window = new CExampleWindow();
}
