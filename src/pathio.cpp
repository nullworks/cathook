/*
 * textfile.cpp
 *
 *  Created on: Jan 24, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"

#include <stdio.h>

// Cached data path
std::optional<std::string> cached_data_path;

namespace paths
{
// Example: getDataPath("/foo") -> "/opt/bar/data/foo"
std::string getDataPath(std::string subpath)
{
    if (!cached_data_path)
    {
        if (std::getenv("CH_DATA_PATH"))
        {
            cached_data_path = std::getenv("CH_DATA_PATH");
        }
        else
        {
#if ENABLE_BINARYMODE
            std::string xdg_data_dir;
            if (std::getenv("XDG_DATA_HOME"))
                xdg_data_dir = std::getenv("XDG_DATA_HOME");
            else if (std::getenv("HOME"))
                xdg_data_dir = std::string(std::getenv("HOME")) + "/.local/share";
            if (xdg_data_dir.empty())
                cached_data_path = DATA_PATH;
            else
                cached_data_path = xdg_data_dir + "/cathook/data";
#else
            cached_data_path = DATA_PATH;
#endif
        }
    }
    return *cached_data_path + subpath;
}
std::string getConfigPath()
{
    return getDataPath("/configs");
}
} // namespace paths

// Textfile class functions
TextFile::TextFile() : lines{}
{
}

bool TextFile::TryLoad(const std::string &name)
{
    if (name.length() == 0)
        return false;
    std::string filename = format(DATA_PATH "/", name);
    std::ifstream file(filename, std::ios::in);
    if (!file)
    {
        return false;
    }
    lines.clear();
    for (std::string line; std::getline(file, line);)
    {
        if (*line.rbegin() == '\r')
            line.erase(line.length() - 1, 1);
        lines.push_back(line);
    }
    if (lines.size() > 0 && *lines.rbegin() == "\n")
        lines.pop_back();

    return true;
}

void TextFile::Load(const std::string &name)
{
    std::string filename = DATA_PATH "/" + name;
    std::ifstream file(filename, std::ios::in);
    if (file.bad())
    {
        logging::Info("Could not open the file: %s", filename.c_str());
        return;
    }
    lines.clear();
    for (std::string line; std::getline(file, line);)
    {
        if (*line.rbegin() == '\r')
            line.erase(line.length() - 1, 1);
        lines.push_back(line);
    }
}

size_t TextFile::LineCount() const
{
    return lines.size();
}

const std::string &TextFile::Line(size_t id) const
{
    return lines.at(id);
}
// Textfile class functions end
