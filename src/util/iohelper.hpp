
/*
 *
 *	Stuff to interact with the filesystem goes here!
 *
 */

#pragma once

#include <cstdio> // FILE*, fopen()
#include <string>

const char* GetProcessName();

// Creates directory for a file
void CreateDirectorys(std::string path);

// A packed file is a group of chars that make up an object that can be unpacked to be used in cathook
// First convert your file to a char array, then input it into the constructor.
// When you want to use it, it will unpack the file into a tmp directory.
// When unpacked, you can get the path and file handle for use of the file.
// Keep in mind, if more than one of the file exists, It will reuse the existing file.
class PackedFile {
public:
	// To construct this, include a file with a char array of your file in binary and input it here
	PackedFile(const void* _file_start, size_t _file_size);
	~PackedFile();

	std::string name = ""; // path to the unpackedfile
	FILE* file_handle = nullptr; // File handle to the temperary file
	const void* file_start; // Starting point for file in binary
	const size_t file_size;
};
