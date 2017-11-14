 
#include <string>

const char* GetProcessName();

// A packed file is a group of chars that make up an object that can be unpacked to be used in cathook
// First convert your file to a char array, then input it into the constructor.
// When you want to use it, it will unpack the file into a tmp directory.
// When unpacked, you can get the path and file handle for use of the file.
// Keep in mind, if more than one of the file exists, It will reuse the existing file.
class PackedFile {
public:
	// To construct this, include a file with a char array of your file in binary and input it here
	PackedFile(const char* name, const char* packed_file);
	~PackedFile();
	
private:
	const char* name;
	const char* packed_file;
	FILE* handle;
}