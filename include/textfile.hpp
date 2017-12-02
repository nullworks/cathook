/*
 * textfile.h
 *
 *  Created on: Jan 24, 2017
 *      Author: nullifiedcat
 */

#ifndef TEXTFILE_HPP_
#define TEXTFILE_HPP_

#include <aftercheaders.hpp>
#include <beforecheaders.hpp>
#include <string>
#include <vector>

class TextFile {
public:
	TextFile();
	void Load(std::string filename);
	bool TryLoad(std::string filename);
	size_t LineCount() const;
	const std::string& Line(size_t id) const;
public:
	std::vector<std::string> lines;
};

#endif /* TEXTFILE_HPP_ */
