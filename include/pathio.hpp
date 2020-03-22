/*
 * textfile.h
 *
 *  Created on: Jan 24, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include <string>
#include <vector>

namespace paths
{
std::string getDataPath(std::string subpath = "");
std::string getConfigPath();
} // namespace paths

class TextFile
{
public:
    TextFile();
    void Load(const std::string &filename);
    bool TryLoad(const std::string &filename);
    size_t LineCount() const;
    const std::string &Line(size_t id) const;

public:
    std::vector<std::string> lines;
};
