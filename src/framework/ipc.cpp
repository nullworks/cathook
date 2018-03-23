/*
 *
 * Nekohooks "ipc" isnt an ipc in the traditional since but it still allows us to communicate between processes...
 *
 *
 */

#include <unordered_map>
#if defined(__linux__)
  #include <fcntl.h> // flags for shm_open()
  #include <sys/stat.h> // S_IRWXU, S_IRWXG, S_IRWXO
  #include <errno.h>
  #include <string.h> // error string
  #include <sys/mman.h> // mmap
  #include <unistd.h> // ftruncate
#elif defined(_WIN32)
  #include <windows.h>
#endif

#include "../util/logging.hpp" // Logging is good u noob
#include "console.hpp"

#include "ipc.hpp"

// TODO: clean this shit up, it looks horrible!
// TODO: Windows support?

namespace ipc {

// The big boi
IpcStream* g_IpcStream = nullptr;

// Command handler
static std::unordered_map<std::string, IpcCommand*> IpcCommandMap;
IpcCommand::IpcCommand(const char* name, void(*_com_callback)(const IpcMessage* message)) : com_callback(_com_callback) {
  IpcCommandMap.insert({name, this});
}

// Stock ipc commands
static IpcCommand ipc_exec("exec", [](const IpcMessage* message){
  CallCommand((const char*)message->payload);
});

// Stock CatCommands to handle ipc
static CatCommand exec_all("ipc_exec_all", [](std::vector<std::string> args) {
  if (args.empty()) {
    g_CatLogging.log("Missing args");
    return;
  }
  if (!g_IpcStream) {
    g_CatLogging.log("IPC isnt connected");
    return;
  }
  // we send size + 1 to include end char
  g_IpcStream->SendAll("exec", (void*)args.at(0).c_str(), args.at(0).size() + 1);
});

// Stock CatCommands to handle ipc
static CatCommand connect("ipc_connect", [](std::vector<std::string> args) {
  if (g_IpcStream) {
    g_CatLogging.log("IPC already connected");
    return;
  }
  // If no args, use default network
  std::string tmp;
  if (args.empty())
    tmp = "neko_ipc";
  else
    tmp = args.at(0);
  // Connect to ipc
  g_IpcStream = new IpcStream(tmp.c_str());
});

static CatCommand disconnect("ipc_disconnect", [](std::vector<std::string> args) {
  if (!g_IpcStream) {
    g_CatLogging.log("IPC isnt connected");
    return;
  }
  // Destruct ipc stream and set to null
  delete g_IpcStream;
  g_IpcStream = nullptr;
});

static CatCommand list("ipc_list", [](std::vector<std::string> args) {
  if (!g_IpcStream) {
    g_CatLogging.log("IPC isnt connected");
    return;
  }
  auto tmp = g_IpcStream->GetMembers();
  for (const auto& i : tmp)
    g_CatLogging.log("Found IPC member: %s", i.c_str());
});

// Constructor responsibilities, constructing the shared memory pool and starting the thread if successful
IpcStream::IpcStream(const char* _pool_name) : pool_name(_pool_name) {
  #ifdef __linux__ // Linux only sadly, gotta find a way to do this in windows
    // Try to get access to the memory pool
    int shm_res = shm_open(this->pool_name, O_RDWR | O_CREAT, S_IRWXU | S_IRWXG | S_IRWXO);
    if (shm_res == -1) {
      char buffer[1024];
      const char* error_string = strerror_r(errno, buffer, sizeof(buffer));
      g_CatLogging.log("IPC: Fatal shm_open error: \"%s\", panicing and stopping!", error_string);
      return;
    }

    // truncate size, this is required for memory to be read/writable, as mmap needs the handle to have a size beforehand
    if (ftruncate(shm_res, sizeof(IpcContent)) == -1){
  	  char buffer[1024];
      const char* error_string = strerror_r(errno, buffer, sizeof(buffer));
      g_CatLogging.log("IPC: ftruncate recieved a unknown error... halp: %s\n", error_string);
  	  return;
    }

    // Map the memory pool
    this->IpcMemSpace = (IpcContent*)mmap(NULL, sizeof(IpcContent), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_32BIT, shm_res, 0);
    if (this->IpcMemSpace == MAP_FAILED) {
      g_CatLogging.log("IPC: Fatal mmap error, panicing and stopping!");
      return;
    }
  #elif defined(_WIN32)
    #pragma message ("Windows IPC is in an experimental state")
    // This is just from windows documentation, this needs sometesting to get working
    // Get memory handle
    HANDLE hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(IpcContent), this->pool_name);
    if (hMapFile == NULL) {
      g_CatLogging.log("IPC: Cannot create file mapping: \"%s\", panicing and stopping!", GetLastError());
      return;
    }
    // Open for viewing
    this->IpcMemSpace = (IpcContent*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(IpcContent));
    if (this->IpcMemSpace == NULL) {
      g_CatLogging.log("IPC: Cannot get MapViewOfFile: \"%s\", panicing and stopping!", GetLastError());
      return;
    }
  #else
    #pragma message ("IPC DISABLED")
    g_CatLogging.log("IPC: IPC disabled, wrong platform!");
    return;
  #endif
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
      // Allow peers to see us
      this->IpcMemSpace->members[this->ipc_pos].state = ipc_state::RECIPIENT_LOCKED;
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
}

IpcStream::~IpcStream() {
  #ifdef __linux__
    g_CatLogging.log("IPC: IPC shutting down!");
    this->state = STOP;
    // We hang the thread deconstructing this until the loop thread ends to prevent segfaulting
    while(this->state != HALTED)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // Unmap the shared memory
    munmap(this->IpcMemSpace, sizeof(IpcContent));
    shm_unlink(this->pool_name);
  #endif
}

void IpcStream::thread_loop() {
  // Main loop
  this->state = RUNNING;
  while (this->state != STOP) {

    // Ping keeper
    this->IpcMemSpace->members[this->ipc_pos].time.Reset();

    // Find new messages sent to us
    for (auto& i : this->IpcMemSpace->message_pool) {
      if (i.state == ipc_state::RECIPIENT_LOCKED) {
        if (i.recipient == this->ipc_pos) {
          auto find = IpcCommandMap.find(i.command);
          if (find != IpcCommandMap.end()) {
            g_CatLogging.log("IPC: Recieved Command: %s!", i.command);
            // Run the command we found
            (*find->second)(&i);
          } else
            g_CatLogging.log("IPC: Recieved Unknown Command: %s!", i.command);
          // Reset the message status since we recieved it
          i.state = ipc_state::OPEN;
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



bool IpcStream::SendMessage(const std::string& recipient, const char* command, const void* payload, size_t size) {
  if (recipient == "all") {
    this->SendAll(command, payload, size);
    return true;
  }
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
    if (i == this->ipc_pos) continue; // dont send one to yourself dummy
    if (this->IpcMemSpace->members[i].state != ipc_state::RECIPIENT_LOCKED) continue;
    this->SendMessage(i, command, payload, size);
  }
}

}
