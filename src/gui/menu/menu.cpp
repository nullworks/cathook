
/*
 *
 * 	This is the menu constructor for the gui. This build a menu our of elements for us to use.
 *
 */

#include "../../util/catvars.hpp"	// CatMenuRoot
#include "../../util/logging.hpp"
#include "../gui.hpp"

#include "../base/CBaseTitleBar.hpp"
#include "menu.hpp"
#include "menubutton.hpp"
#include <sstream>

// menus aspect ratio, dont judge
#define aspectx 13
#define aspecty 7

namespace gui { namespace menu {

base::CBaseWindow pMenu("Menu Root");

void populateMenu(base::CBaseContainer *parent, CatMenuTree &branch, unsigned int depth=0){
  for(auto& child:branch.children){
    CMenuButton *button=new CMenuButton(child.name.c_str());
    parent->AddChild(button);
    CMenu *childMenu = new CMenu(child.name.c_str());
    button->SetChild(childMenu);
    populateMenu(childMenu, child, depth+1);
  }
  for(auto child:branch.cat_children){
    CMenuButton *button=new CMenuButton(child->name.c_str());
    parent->AddChild(button);
  }
}
CMenu::CMenu(const char * str):CBaseWindow(str){
  hover_is_focus=true;
  position_mode = base::FLOATING;
}

void CMenu::Update() {
  offset = std::make_pair(parent->size.first,0);
  CBaseWindow::Update();
}

void Init() {
  g_pGui.AddChild(&pMenu);
  base::CBaseTitleBar *title=new base::CBaseTitleBar("Menu");
  pMenu.AddChild(title);
  populateMenu(&pMenu,CatMenuRoot);
  
  //std::stringstream ss;
  //CatMenuRoot.Stringify(ss);
  //g_CatLogging.log("%s",ss.str().c_str());
}

}}
