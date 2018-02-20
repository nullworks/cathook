

/*
 *
 *	Do io stuff here
 *
 */

#include <fstream> // ifstream
#ifdef __linux__
  #include <stack>
  #include <sys/stat.h> // mkdir()
  #include <libgen.h> // dirname()
  #include <errno.h> // errors
#endif

#include "logging.hpp"
#include "strings.hpp" // substr()

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
  proc_status.close();
	// Seperate "Name: " from our string
	substr(proc_name, proc_name, 6, sizeof(proc_name) - 6); // this isnt as reliable as regex but it gets the job done and quick

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

void CreateDirectorys(std::string path) {
	#ifdef __linux__
    // Stack to store parent directories
    std::stack<std::string> dir_stack;
    // Get our directories
		while (path != "." && path != "/") {
      dir_stack.push(path);
			char cut_path[512];
			strcpy(cut_path, path.c_str());
			dirname(cut_path);
			path = cut_path;
		}
    // Make the directories
    while (!dir_stack.empty()) {
			if (mkdir(dir_stack.top().c_str(), S_IRWXU | S_IRWXG | S_IRWXO)) {
				if (errno != EEXIST && errno != ENOENT) {
					char error_str[1024];
					g_CatLogging.log("Couldnt Create directory %s: %s", dir_stack.top().c_str(), strerror_r(errno, error_str, sizeof(error_str)));
				}
			} else
        g_CatLogging.log("Made Dir: \"%s\"", dir_stack.top().c_str());
      dir_stack.pop();
    }
	#endif
	return;
}
