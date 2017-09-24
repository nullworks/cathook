 

#ifndef ROOTBASEGUI
#define ROOTBASEGUI

#include <vector>
#include "../../util/colors.hpp"

namespace gui { namespace element {

class CRoot {
public:
	CRoot();
public:	
	
	bool visible = true;
	int rootx;
	int rooty;
	
	int widthx;
	int widthy;
	
	void draw(rgba_t color);
};
	
extern std::vector<CRoot*>& CRootList();
}}

#endif