

/*
 *
 *	Do io stuff here
 *
 */

#include <fstream> // ifstream

#ifdef __linux__
  #include <stack>
  #include <sys/stat.h> // mkdir()S_IRWXU | S_IRWXG | S_IRWXO
  #include <libgen.h> // dirname()
  #include <errno.h> // errors
  #include <unistd.h> // Linux username and paths
  #include <pwd.h>
#endif

#include "logging.hpp"
#include "strings.hpp" // substr()

#include "iohelper.hpp"

// Uhhhhh, TODO!!!!
/*PackedFile::PackedFile(const void* _file_start, size_t _file_size) : file_start(_file_start), file_size(_file_size)  {

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
}*/

namespace io {

// TODO, remake this, its one of my first implimentations of ifstream, could be much better
// Gets name of process using unix goodies
std::string GetProcessName() {
  #ifdef __linux__
  	// Open /proc/self/status to get our process name
  	std::ifstream proc_status("/proc/self/status");
  	if (!proc_status.is_open()) return "unknown";

  	// Get the first line
  	char proc_name[32];
  	proc_status.getline(proc_name, sizeof(proc_name)); // We should only need the first line as unix keeps "Name: " on first
    proc_status.close();
  	// Seperate "Name: " from our string
  	substr(proc_name, proc_name, 6, sizeof(proc_name) - 6); // this isnt as reliable as regex but it gets the job done and quick

  	// Return the static char array with the process name in it
  	return proc_name;
  #else
    #warning "Cannot get process name"
  #endif
}

void CreateDirectorys(std::string path) {
  // TODO, impliment a way to remove the file if present from the string path so we dont make a directory named as a file meant to be used
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
	#else
    #warning "Cannot Create directories"
  #endif
	return;
}

// Save location
std::string GetSaveLocation() {
  std::string ret;
  #if defined(__linux__)
    ret = std::string(getpwuid(getuid())->pw_dir) + "/.config/nekohook/";
  #else
    #warning "Cant get save location for nekohook files"
  #endif
  return ret;
}

std::vector<std::string> ReadFile(const std::string& path) {
  std::vector<std::string> ret;
  try {
    // Open the file
  	std::ifstream read_stream(path);
    if (!read_stream) {
      g_CatLogging.log("Couldnt open \"%s\"", path.c_str());
      return ret;
    }

  	// Recurse through the lines of the file
  	while (!read_stream.eof()) {
  		// Get our line and push to ret
  		char buffer[512];
  		read_stream.getline(buffer, sizeof(buffer));
      ret.push_back(buffer);
    }
    read_stream.close();
  } catch (const std::ios_base::failure& e) {
    g_CatLogging.log("Couldnt Read file \"%s\": %s", path.c_str(), e.what());
  }
  return ret;
}

void WriteFile(const std::string& path, std::vector<std::string> to_write) {
  try {
    // Create the file stream
    std::fstream write_stream(path, std::ios::out | std::ios_base::trunc);
    if (!write_stream.is_open()) {
      g_CatLogging.log("Couldnt open \"%s\"", path.c_str());
      return;
    }
    // We dont use an iterator so we can tell where we are in the loop
    for (int i = 0; i < to_write.size(); i++) {
      if (i != to_write.size() - 1)
        to_write.at(i) += "\n";
      write_stream.write(to_write.at(i).c_str(), to_write.at(i).size());
    }
    write_stream.close();
  } catch (const std::ios_base::failure& e) {
    g_CatLogging.log("Couldnt write file \"%s\": %s", path.c_str(), e.what());
  }
}

// An asynchronous file trail, im bored so im continuing this in case of need in future
/*class FileTrail {
public:
  FileTrail(const char* _path, CMFunction<void(std::string)> _callback, bool _thread = true)
    : path(_path), callback(_callback), thread(_thread) {
    // We should test if we can open the file
    std::ifstream test(_path);
    if(!test) {
      g_CatLogging.log("Couldnt open file for trailing \"%s\"", _path);
      return;
    }
    if (_thread) {
      g_CatLogging.log("Starting thread for file trail of \"%s\"", _path);
      std::thread trail_thread(&FileTrail::thread_loop, this);
    }
  }
  ~FileTrail(){
    if (thread) {
      stop = true;
      while(stop)
        std::this_thread::sleep_for(std::duration::seconds(1));
    }
  }
private:
  const char* path;
  const CMFunction<void(std::string)> callback;
  std::streampos cur_pos;
  // for threading
  bool thread = false;
  bool stop = false;
  void thread_loop() {
    while(!stop) {
      thread_tick();
      std::this_thread::sleep_for(std::duration::seconds(1));
    }
    stop = false;
  }
public:
  // If you chose not to thread, you must run the loop yourself, its kinda expensive which is why it is threaded
  void thread_tick() {
    std::ifstream read_stream(path);
    if (!read_stream) {
      stop = true; // try to stop the thread gently if possible
      return;
    }
    ipc_stream.seekg(cur_pos);
    ipc_stream.clear();
    char buffer[2048];
    ipc_stream.getline(buffer, sizeof(buffer));
    if (!ipc_stream.fail() && strlen(buffer) > 0) {
      queued_commands.push(buffer);
      cur_pos = ipc_stream.tellg();
      std::this_thread::sleep_for(std::chrono::milliseconds(75));
      continue;
    }
  }
};*/

}
