/*
 * textfile.h
 *
 *  Created on: Jan 24, 2017
 *      Author: nullifiedcat
 */

#ifndef TEXTFILE_H_
#define TEXTFILE_H_

#include "beforecheaders.h"
#include <string>
#include <vector>
#include "aftercheaders.h"

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

#endif /* TEXTFILE_H_ */
