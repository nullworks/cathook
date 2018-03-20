/*
 *
 * Nekohooks "ipc" isnt an ipc in the traditional since but it still allows us to communicate between processes...
 *
 *
 */

#include <thread> // Threads are useful!
#include <vector>
//#include <queue> // todo
#ifdef __linux__
  #include <fcntl.h> // flags for shm_open()
  #include <sys/stat.h> // S_IRWXU, S_IRWXG, S_IRWXO
  #include <errno.h>
  #include <string.h> // error string
  #include <sys/mman.h> // mmap
#endif

#include "../util/chrono.hpp" // for the cat timer, and so we can sleep in thread
#include "../util/functions.hpp" // ThreadedLoop
#include "../util/logging.hpp" // Logging is good u noob

// TODO: clean this shit up, it looks horrible!
// TODO: Windows support?

// Increase as needed, we just need enough space to handle
#define MAX_IPC_MEMBERS 32
#define MAX_IPC_MESSAGE_CAP (MAX_IPC_MEMBERS * MAX_IPC_MEMBERS / 8)

namespace ipc {

// IPC Memory space

enum ipc_state {
  OPEN, // Item is open to the network
  LOCKED, // Item is locked
  RECIPIENT_LOCKED // Item is open to recipients
};
struct IpcSlot { // A slot, open, closed, simple as that
  int state = ipc_state::OPEN;
  CatTimer time;
};
struct IpcMessage : public IpcSlot { // A message, it is the recipients job to recieve it in time and reset it
  int author; // number in array
  int recipient;
  char command[64]; // A command that tells the recipient what to do with the payload
  char payload[1024]; // This could be anything
};
struct IpcMember : public IpcSlot {
  char name[64] = "";
};
// This is what the entire Ipc mapped memory should be
struct IpcContent {
  IpcMember members[MAX_IPC_MEMBERS];
  IpcMessage message_pool[MAX_IPC_MESSAGE_CAP];
};

// IPC Driver

// Ipc stream class
class IpcStream {
private:
  // Constructor/Descructor needed stuff
  const char* pool_name;
  // Ipc stuff
  IpcContent* IpcMemSpace = nullptr; // A pointer to the ipc space
  int ipc_pos = -1; // our position in the ipc

  // Setup of shared memory

public:
  IpcStream(const char* _pool_name) : pool_name(_pool_name) {
    #ifdef __linux__ // Linux only sadly, gotta find a way to do this in windows
    // Try to get access to the memory pool
    int shm_res = shm_open(this->pool_name, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
    bool first_peer = false;
    if (shm_res == -1) {
      if (errno == ENOENT) { // The memory isnt mapped, we will do so ourselves.
        shm_res = shm_open(this->pool_name, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
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
    this->IpcMemSpace = (IpcContent*)mmap(NULL, sizeof(IpcContent), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_32BIT, shm_res, 0);
    if (this->IpcMemSpace == MAP_FAILED) {
      g_CatLogging.log("IPC: Fatal mmap error, panicing and stopping!");
      return;
    }

    // If we are the first peers, we must setup everything ourselves
    if (first_peer) {
      g_CatLogging.log("IPC: Setting up ipc space!");
      memset(this->IpcMemSpace, 0, sizeof(IpcContent));
      *this->IpcMemSpace = IpcContent();
    }

    // Setup a member slot for us
    this->ipc_pos = GetOpenSlot(this->IpcMemSpace->members, MAX_IPC_MEMBERS);
    if (this->ipc_pos != -1) {
      // Set ping so we dont get instantly rejected by peers, then set the state to locked so we can setup
      this->IpcMemSpace->members[this->ipc_pos].time.Reset();
      this->IpcMemSpace->members[this->ipc_pos].state = ipc_state::LOCKED;
      for (int i = 0; i < MAX_IPC_MEMBERS; i++) {
        std::string test = "neko_" + std::to_string(i);
        if (!MemberExists(test)) {
          strcpy(this->IpcMemSpace->members[this->ipc_pos].name, test.c_str());
          break;
        }
      }
      if (strlen(this->IpcMemSpace->members[this->ipc_pos].name) != 0){
        g_CatLogging.log("IPC: Found name: %s!", this->IpcMemSpace->members[this->ipc_pos].name);
      } else {
        g_CatLogging.log("IPC: Cant find suitible name!");
        return;
      }
    } else {
      g_CatLogging.log("IPC: No open slot to register in!");
      return;
    }

    // Set us up with a slot and name
    // Start a thread for the ipc
    g_CatLogging.log("IPC: Starting Thread");
    std::thread ipc_thread(&IpcStream::thread_loop, this);
    ipc_thread.detach();
    #else
      #pragma message ("IPC DISABLED")
    #endif
  }

  // Setters/Getters

public:
  std::vector<std::string> GetMembers() {
    std::vector<std::string> ret;
    if (!this->IpcMemSpace)// Saftey net
      return ret;
    for (auto i: this->IpcMemSpace->members)
      ret.push_back(i.name);
    return ret;
  }
  std::string GetIpcName() {
    if (!this->IpcMemSpace || ipc_pos == -1) return std::string();
    return this->IpcMemSpace->members[ipc_pos].name;
  }
  bool MemberExists(const std::string& test_name) {
    for (auto i: this->IpcMemSpace->members)
      if (i.name == test_name)
        return true;
    return false;
  }
private:
  // Internal cleanup of the ipc space, in the case of a member not being nice >:(
  void IpcCleanup(){
    // For the cleanup, we look for anything not open and check the timer on it
    // We clean thing more than 3 seconds of idle
    for (auto i : this->IpcMemSpace->members)
      if (i.state != ipc_state::OPEN && i.time.CheckTime(std::chrono::seconds(3)))
        i.state = ipc_state::OPEN;
    for (auto i : this->IpcMemSpace->message_pool)
      if (i.state != ipc_state::OPEN && i.time.CheckTime(std::chrono::seconds(3)))
        i.state = ipc_state::OPEN;
  }
  // Internal use for finding open slots, be sure to give it the right size to prevent segfaults
  inline int GetOpenSlot(IpcSlot* slot_array, int size){
    for (int i = 0; i < size; i++)
      if (slot_array[i].state == ipc_state::OPEN)
        return i;
    return -1;
  }

  // Threading, TODO: replace with ThreadedLoop

private:
  enum {RUNNING, STOP, HALTED};
  int state = HALTED;
  CatTimer cleanup;
  void thread_loop() {

    // Main loop
    this->state = RUNNING;
    while (this->state != STOP) {

      // Ping keeper
      this->IpcMemSpace->members[this->ipc_pos].time.Reset();

      // Find new messages sent to us
      for (auto i : this->IpcMemSpace->message_pool) {
        if (i.state == ipc_state::RECIPIENT_LOCKED) {
          if (i.recipient == this->ipc_pos) {
            // TODO, make system to handle this
          }
        }
      }

      // Cleanup Crew, we clean every 15 seconds... why, i dont know /shrug
      if (cleanup.ResetAfter(std::chrono::seconds(15)))
        this->IpcCleanup();

      // Loop sleep
      std::this_thread::sleep_for(std::chrono::milliseconds(75));
    }
    g_CatLogging.log("IPC: Thread recieved shutdown!");
    this->state = HALTED;
  }
public:
  ~IpcStream() {
    g_CatLogging.log("IPC: IPC shutting down!");
    this->state = STOP;
    // We hang the thread deconstructing this until the loop thread ends to prevent segfaulting
    while(this->state != HALTED)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    shm_unlink(this->pool_name);
  }
};

}
