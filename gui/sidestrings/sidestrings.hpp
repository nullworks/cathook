
#include <string>
#include "../../util/colors.hpp"

class HudStrings {
public:
	std::string string[32];
	rgba_t color[32];
	int count;
	void AddString(const std::string&, const rgba_t&);
} 
extern HudStringsside_strings; 

void DefaultSideStrings();
	
void DrawSideStrings();