
/*
 *
 *  Run init() to get a test window
 *
 */

#pragma once

#include "../../gui.hpp"
#include "../CBaseWindow.hpp"
#include "../CBaseTitleBar.hpp"
#include "../CBaseInfoBox.hpp"

namespace gui {
using namespace base;

class CExampleWindow : public CBaseWindow {
public:
CExampleWindow() : CBaseWindow("example_window") {
  AddChild(new CBaseTitleBar("Example Window"));
  //AddChild(new CBaseInfoBox("info", "This is an example window!"));
}

};

CExampleWindow* example_window = nullptr;

void ShowTestWindow() {
  if (example_window) return;
  example_window = new CExampleWindow();
  g_pGui.AddChild(example_window);
}

}
