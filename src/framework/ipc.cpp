/*
 *
 * Nekohooks "ipc" isnt an ipc in the traditional since but it still allows us to communicate between processes...
 *
 *
 */

//#include <queue> // todo
#ifdef __linux__
  #include <fcntl.h> // flags for shm_open()
  #include <sys/stat.h> // S_IRWXU, S_IRWXG, S_IRWXO
  #include <errno.h>
  #include <string.h> // error string
  #include <sys/mman.h> // mmap
#endif

#include "../util/logging.hpp" // Logging is good u noob

#include "ipc.hpp"

// TODO: clean this shit up, it looks horrible!
// TODO: Windows support?

namespace ipc {

// Constructor responsibilities, constructing the shared memory pool and starting the thread if successful
IpcStream::IpcStream(const char* _pool_name) : pool_name(_pool_name) {
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
  this->IpcCleanup(); // cleanup to make sure we get unused slots
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

void IpcStream::thread_loop() {

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

IpcStream::~IpcStream() {
  g_CatLogging.log("IPC: IPC shutting down!");
  this->state = STOP;
  // We hang the thread deconstructing this until the loop thread ends to prevent segfaulting
  while(this->state != HALTED)
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  shm_unlink(this->pool_name);
}

bool IpcStream::SendMessage(std::string recipient, const char* command, const void* payload, size_t size) {
  auto tmp = this->FindMember(recipient);
  if (tmp == -1) return false;
  this->SendMessage(tmp, command, payload, size);
  return true;
}
bool IpcStream::SendMessage(int recipient, const char* command, const void* payload, size_t size) {
  auto tmp = this->GetOpenSlot(this->IpcMemSpace->message_pool, MAX_IPC_MESSAGE_CAP);
  if (tmp == -1) return false;
  // Reset ping and lock so we can use without issues
  this->IpcMemSpace->message_pool[tmp].time.Reset();
  this->IpcMemSpace->message_pool[tmp].state = ipc_state::LOCKED;
  // Address it
  this->IpcMemSpace->message_pool[tmp].author = this->ipc_pos;
  this->IpcMemSpace->message_pool[tmp].recipient = recipient;
  // Copy our payload into the message
  strcpy(this->IpcMemSpace->message_pool[tmp].command, command);
  memcpy(this->IpcMemSpace->message_pool[tmp].payload, payload, size);
  // Allow the recipient to read it
  this->IpcMemSpace->message_pool[tmp].state = ipc_state::RECIPIENT_LOCKED;
  return true;
}
void IpcStream::SendAll(const char* command, const void* payload, size_t size) {
  this->IpcCleanup();// Cleanup so we only get existing members
  for (int i = 0; i < MAX_IPC_MEMBERS; i++) {
    if (this->IpcMemSpace->members[i].state != ipc_state::RECIPIENT_LOCKED) continue;
    this->SendMessage(i, command, payload, size);
  }
}

}
