
#include <string>

#include "../../util/colors.hpp"

#define MAX_SIDESTRINGS 32

namespace gui { namespace sidestrings {
	
class HudStrings {
public:
	int string_count = 0;
	colors::ColoredString strings[MAX_SIDESTRINGS];
	void AddString(const std::string&, const rgba_t&);
};
	
extern HudStrings side_strings; 

void DefaultSideStrings();
	
void DrawSideStrings();
	
void BuildTopString();
}}