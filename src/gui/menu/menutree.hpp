 
/*
 *
 *	This file helps with the menu tree
 *
 */

#include <vector>

#include "../../util/catvars.hpp"	// We are using catvars for many things

namespace gui { namespace menu {

// Used to store the menu listings
class CMenuTree {
public:
	~CMenuTree();	// Deconstructor to prevent memory leak
public:
	std::string name;
	std::vector<CMenuTree*> children;
	std::vector<CatVar*> 	cat_children;	// Nyaa~ :3
};
	
CMenuTree* GetMenuTree();
	
}}