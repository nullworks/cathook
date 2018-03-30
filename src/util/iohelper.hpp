
/*
 *
 *	Stuff to interact with the filesystem goes here!
 *
 */

#pragma once

#include <cstdio> // FILE*, fopen()
#include <string>
#include <vector>

namespace io {

// Get the name of the process
std::string GetProcessName();

// Get a general location to read and write files
std::string GetSaveLocation();

// Get systems tmpdir
std::string GetTmpDir();

// Below actualy deals with filesystem

// Because windows is bad, we need to use backslashes
constexpr char GetDirSeperator() {
  #if defined(_WIN32) // Only windows uses this awkward scheme
    return '\\';
  #endif
  return '/';
}

// Creates directory for a file
void CreateDirectorys(std::string path);

// Easier reading and writing of files
std::vector<std::string> ReadFile(const std::string& path);
void WriteFile(const std::string& path, std::vector<std::string> to_write);

}

// A packed file is a group of chars that make up an object that can be unpacked to be used in cathook
// First convert your file to a char array, then input it into the constructor.
// When you want to use it, it will unpack the file into a tmp directory.
// When unpacked, you can get the path and file handle for use of the file.
// Keep in mind, if more than one of the file exists, It will reuse the existing file.
/*class PackedFile {
public:
	// To construct this, include a file with a char array of your file in binary and input it here
	PackedFile(const void* _file_start, size_t _file_size);
	~PackedFile();

	std::string name = ""; // path to the unpackedfile
	FILE* file_handle = nullptr; // File handle to the temperary file
	const void* file_start; // Starting point for file in binary
	const size_t file_size;
};*/
