
/*
 *
 * Man... I really want to die
 *
 */

#pragma once

#include <string>
#include <sstream>

#include "../../framework/drawing.hpp"

inline std::string WordWrap(std::string& in, int max, int font = draw::default_font, int font_size = draw::default_font_size) {
	std::stringstream result, line, wordstream, next;
	std::string word;
	char ch;
	for (int i = 0; i < in.size(); i++) {
		ch = in.at(i);
		if (ch == ' ' || ch == '\n') {
			word = wordstream.str();
			//logging::Info("got word: '%s'", word.c_str());
			wordstream.str("");
			auto size = draw::GetStringLength((line.str() + word).c_str() ,draw::default_font, draw::default_font_size);
			if (size.first >= max) {
				//logging::Info("wrapping: '%s'", line.str().c_str());
				result << line.str() << '\n';
				line.str("");
			}
			line << word << ch;
		} else {
			wordstream << ch;
		}
	}
	word = wordstream.str();
	wordstream.str("");
	auto size = draw::GetStringLength((line.str() + word).c_str() ,draw::default_font, draw::default_font_size);
	if (size.first >= max) {
		result << line.str() << '\n';
		line.str(word);
	} else {
		line << word;
	}
	result << line.str();
	return result.str();
}
