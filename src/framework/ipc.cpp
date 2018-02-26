/*
 *
 * Nekohooks "ipc" isnt an ipc in the traditional since but it still allows us to communicate between processes...
 *
 *
 */

#include <chrono> // For time so we can sleep
#include <thread> // Threads are useful!
#include <vector>
//#include <queue> // todo
#ifdef __linux__
  #include <fcntl.h> // flags for shm_open()
  #include <sys/stat.h> // S_IRWXU, S_IRWXG, S_IRWXO
  #include <errno.h>
  #include <string.h> // error string
  #include <sys/mman.h> // mmap
#elif
  #error "IPC ISNT MULTIPLAT"
#endif
#include "../util/logging.hpp"

namespace ipc {

enum ipc_message_state {
  OPEN, // Message is open for writing
  LOCKED, // Message is being written to
  SENT // Message has been written and is open to the recipient
};
struct IpcMessage {
  int state = ipc_message_state::OPEN;
  std::chrono::steady_clock::time_point time_sent;
  char author[32];
  char recipient[32];
  char payload[1024]; // This could be anything
};
// This is what the entire Ipc mapped memory should be
struct IpcContent {
  char members[32][32];
  std::chrono::steady_clock::time_point member_last_ping[32];
  IpcMessage message_pool[128];
};

// Ipc stream class
class IpcStream {
public:
  const char* pool_name;
  IpcStream(const char* _pool_name) : pool_name(_pool_name) {
    // Try to get access to the memory pool
    int shm_res = shm_open(pool_name, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    bool first_peer = false;
    if (shm_res == -1) {
      if (errno == ENOENT) { // The memory isnt mapped, we will do so ourselves.
        shm_res = shm_open(pool_name, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
        first_peer = true;
      }
      if (shm_res == -1) { // check if we still have error from above, or above wasnt run
        char buffer[1024];
        const char* error_string = strerror_r(errno, buffer, sizeof(buffer));
        g_CatLogging.log("IPC: Fatal shm_open error: %s", error_string);
      }
    }
    if (shm_res == -1) {
      g_CatLogging.log("IPC: Fatal shm_open error, panicing and stopping!");
      return;
    }

    // Map the memory pool
    shared_mem_addr = (IpcContent*)mmap(NULL, sizeof(IpcContent), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_32BIT, shm_res, 0);
    if (shared_mem_addr == MAP_FAILED) {
      g_CatLogging.log("IPC: Fatal mmap error, panicing and stopping!");
      return;
    }

    // If we are the first peers, we must setup everything ourselves
    if (first_peer) {
      g_CatLogging.log("IPC: Setting up ipc space!");
      memset(shared_mem_addr, 0, sizeof(IpcContent));
      *shared_mem_addr = IpcContent();
    }

    // Start a thread for the ipc
    g_CatLogging.log("IPC: Starting Thread");
    std::thread ipc_thread(&IpcStream::thread_loop, this);
    ipc_thread.detach();
  }
  IpcContent* shared_mem_addr = nullptr;
private:
  bool shutdown = false;
  void thread_loop() {

    // Main loop
    //while (!shutdown) {
      // Loop sleep
      std::this_thread::sleep_for(std::chrono::milliseconds(75));
    //}
    // Shutdown
    g_CatLogging.log("IPC: Thread shutting down!");
  }
public:
  ~IpcStream() {
    g_CatLogging.log("IPC: Issuing shutdown to IPC thread!");
    shutdown = true;
    shm_unlink(pool_name);
  }
};

}
