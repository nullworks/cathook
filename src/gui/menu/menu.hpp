
/*
 *
 * 	This is the menu constructor for the gui. This build a menu our of elements for us to use.
 *
 */

#include "../base/CBaseWindow.hpp"
#include "../gui.hpp"

namespace gui { namespace menu {

class CMenu : public base::CBaseWindow {
public:
  std::vector<int> menu_position = {0};
  //virtual void OnMousePress();
};

extern CMenu pMenu;
void Init();

}}
