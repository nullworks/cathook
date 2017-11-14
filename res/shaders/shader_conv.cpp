
/*
 *
 *	A converter to output a shader file for open-gl as a char array.
 *	This is useful for intergrating files into projects without the need or worries with binary conversions.
 *
 *
 */

#include <iostream>
#include <string>
#include <sstream>
#include <string.h>

// To read from file
const char* open_gl_file_read(const char* filename) {
    FILE* file;
    char* buffer;
	size_t size;

    file = fopen(filename, "rb");
    if(!file) {
        printf("Unable to open file \"%s\".\n", filename);
		return nullptr;
    }
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);
    buffer = (char*)malloc((size+1) * sizeof(char*));
	fread(buffer, sizeof(char), size, file);
    buffer[size] = 0;
    fclose(file);
    return buffer;
}

// This is used to replace
std::string replace_chars(const std::string& str, const char* thing_to_replace, const char* thing_to_replace_with) {
	
}

// Our main entry point
int main(int argc, char** argv) {
	
	// Check for a possible file to convert
	if (argc < 2) {
		printf("usage: %s \"File to convert\" \"Output name\"\n", argv[0]);
		return 0;
	}
	
	// Get output name
	std::string output_file;
	if (argc > 2)
		output_file = argv[2];
	else
		output_file = argv[1];
	output_file += ".h";
	
	// Read from our source file and get a char array
	const char* file_as_char = open_gl_file_read(argv[1]);
	if (!file_as_char) return 0;
	
	// Start our file with a nice header
	FILE* output_handle = fopen(output_file.c_str(), "w");
	fprintf(output_handle, "%s",
			"\n"
			"/*\n"
			" *\n"
			" * Generated file\n"
			" *\n"
			" *\n"
			" */\n"
			"\n"
		    "const char* "  "[] = {\n\t"); // TODO, make the name be the file name

	// Recurse through our file handle printing
	for (int i = 0; i < strlen(file_as_char); i++) {
		
		// Get our hex value
		const char& a_byte = file_as_char[i];
		std::stringstream ss;
		ss << std::hex << (int)a_byte;
		
		// Print to file
		fprintf(output_handle, "0x%s", ss.str().c_str());
		if (i != strlen(file_as_char) - 1) fprintf(output_handle, ", ");
		
		// Newline tab
		if (i > 0 && i % 16 == 0)
			fprintf(output_handle, "\n\t", a_byte);
	}
	
	// Output our ending
	fprintf(output_handle, "\n};\n");
	return 0;
}
	



