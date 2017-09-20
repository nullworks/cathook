


#include "drawmodulemgr.hpp"



namespace drawmgr {
	
void DrawTick();
	
class HudStrings {
public:
	std::string string[32];
	rgba_t color[32];
	int count;
	void AddString(const std::string&, const rgba_t&);
	void Reset();
} 
extern HudStringsside_strings;
}

