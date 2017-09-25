 
/*
 *
 *	This file helps dynamicly create a menu tree for the menu to use.
 *
 */

#include "menutree.hpp"

namespace gui { namespace menu {
	
// To store the layout of the menu and where everything is located
CMenuTree* CMenuRoot;

// Recurses through catenums input into it and maps it into the menu tree
void AddMenuTree(CMenuTree* menu_tree, CatEnum* cat_enum, int recursions, CatVar* cat_var) {
	// If we have reached the end of the catenums info, we can deposit our catvar here.
	if (recursions > cat_enum->size) {
		menu_tree->cat_children.push_back(cat_var);
		return;
	}
	std::string enum_name = cat_enum->Name(recursions);
	
	// If our branch has children, one could already be made that we want to go into
	// Check if the menu input has any children
	if (!menu_tree->children.empty()) {
		// Look through the children and if any have the name of one we might want to make, we can reuse the branch.
		for (CMenuTree* tree_branch : menu_tree->children) {
			if (tree_branch->name == enum_name) {
				// We found an already existing branch so we can jsut recurse into it.
				AddMenuTree(tree_branch, cat_enum, recursions + 1, cat_var);	// Recurse + 1 each time to keep trach of how far we went down the enum
			}
		}
	}
		
	// We dont have a branch already so we must make a new one and recurse into it.
	CMenuTree* sapling = new CMenuTree();
	sapling->name = enum_name;
	menu_tree->children.push_back(sapling);
	
	// We now have a new branch to map into!
	AddMenuTree(sapling, cat_enum, recursions + 1, cat_var);
}

// The function that constructs the menu tree
void BuildMenu() {
	if (CMenuRoot) delete CMenuRoot; // Delete the menu if we already have it
	CMenuRoot = new CMenuTree();
	for (CatVar* cat_var : CatVarList) {
		if (cat_var == nullptr) continue;
		
		CatEnum* tmp;
		tmp = cat_var->GetGUIEnum();
		
		if (tmp == nullptr) continue;
		AddMenuTree(CMenuRoot, tmp, 0, cat_var);
	}
}

// Use for when you wish to request the menu tree
CMenuTree* GetMenuTree() {
		
	// If we already have a tree, return it
	if (CMenuRoot) return CMenuRoot;
	BuildMenu();
	return CMenuRoot;
}


// Deletes a tree as well as any children
void DeleteTreeTree(CMenuTree* tree) {
	if (!tree) return;
	
	// If there are no children, we our var and return, else we recurse
	if (tree->children.empty()) {
		delete tree;
		return;
	}
	
	// Recurse and delete children
	for(CMenuTree* sub_tree : tree->children) {
		if (!sub_tree) continue;
		DeleteTreeTree(sub_tree);
	}
	// We are done deleting children, suicide is only option
	delete tree;
}

// Deconstructor in case of deletion.
CMenuTree::~CMenuTree() { 	
	
	// If something has children, we want to delete them... Recursivly... Fuck children lol
	if (!children.empty()) {
		
		// Delete everything in the tree
		DeleteTreeTree(this);
	}
}
	
}}