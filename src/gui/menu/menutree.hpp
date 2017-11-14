
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
	CMenuTree(const char* string = "") name(string) {}
	~CMenuTree() { for (CMenuTree& tree : children) delete tree; } // Deconstructor to prevent memory leak

	void AddTree(const CatVar& cat_var, int recursions = 0);
	const char* name;
	std::vector<CMenuTree> children;
	std::vector<const CatVar&> 	cat_children;	// Nyaa~ :3
};

// Use to get our menu data
const CMenuTree& GetMenuTree();

}}
