

/*
 *
 *	Do io stuff here
 *
 */

#include <fstream> // ifstream

#include "stringhelpers.hpp" // substr()

#include "iohelper.hpp"

// TODO, remake this, its one of my first implimentations of ifstream, could be much better
// Gets name of process using unix goodies
const char* GetProcessName() {
	// Open /proc/self/status to get our process name
	std::ifstream proc_status("/proc/self/status");
	if (!proc_status.is_open()) return "unknown";

	// Get the first line
	static char proc_name[32];
	proc_status.getline(proc_name, sizeof(proc_name), '\n'); // We should only need the first line as unix keeps "Name: " on first

	// Seperate "Name: " from our string
	substr(proc_name, proc_name, 6, sizeof(proc_name) - 6); // this isnt as reliable as regex but it gets the job done and quick
	/*std::cmatch reg_result;
	if (!std::regex_search(line, reg_result, "(?<=Name:[\s]*)([\w]+[\s]*)*")) return "unknown";
	strcpy(proc_name, sm[0]);*/

	// Return the static char array with the process name in it
	return proc_name;
}

// Uhhhhh, TODO!!!!
PackedFile::PackedFile(const void* _file_start, size_t _file_size) : file_start(_file_start), file_size(_file_size)  {

	// Make a temp file to store the file
	char tmp[] = P_tmpdir "/neko.XXXXXX";
	mkstemp(tmp);
	name = tmp;
	// Open our file
	file_handle = fopen(tmp, "w");
	// Write our packed info to the tmp file
	fwrite(file_start , sizeof(char), (file_size - (size_t)_file_start) / (size_t)sizeof(void*), file_handle);
	fflush(file_handle);
}

PackedFile::~PackedFile() {
	fclose(file_handle);
}
