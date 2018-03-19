
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
  CMenu(const char *);
  std::vector<int> menu_position = {0};
  //virtual void OnMousePress();
  virtual void Update();
};

extern base::CBaseWindow pMenu;
void Init();

}}
