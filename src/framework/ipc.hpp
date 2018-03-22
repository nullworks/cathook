
#include <thread> // Threads are useful!
#include <vector>

#include "../util/chrono.hpp" // for the cat timer, and so we can sleep in thread

// Increase as needed, we just need enough space to handle
#define MAX_IPC_MEMBERS 32
#define MAX_IPC_MESSAGE_CAP (MAX_IPC_MEMBERS * MAX_IPC_MEMBERS / 8)

namespace ipc {

// IPC Memory space

// State of a slot
enum ipc_state {
  OPEN, // Item is open to the network
  LOCKED, // Item is locked
  RECIPIENT_LOCKED // Item is open to recipients
};
// A slot, open, closed, simple as that
struct IpcSlot {
  int state = ipc_state::OPEN;
  CatTimer time;
};
// A message, it is the recipients job to recieve it in time and reset it
struct IpcMessage : public IpcSlot {
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
// Constructor/Descructor needed stuff
private:
  const char* pool_name;
  // Ipc stuff
  IpcContent* IpcMemSpace = nullptr; // A pointer to the ipc space
  int ipc_pos = -1; // our position in the ipc

// Setup of shared memory, Destructor, stops thread and unlinks shared memory
public:
  IpcStream(const char* _pool_name);
  ~IpcStream();

// Setters/Getters
public:
  // Used to get a list of members
  inline std::vector<std::string> GetMembers() {
    std::vector<std::string> ret;
    if (!this->IpcMemSpace)// Saftey net
      return ret;
    for (auto i: this->IpcMemSpace->members)
      ret.push_back(i.name);
    return ret;
  }
  // Returns the name used in ipc
  inline std::string GetIpcName() {
    if (!this->IpcMemSpace || ipc_pos == -1) return std::string();
    return this->IpcMemSpace->members[ipc_pos].name;
  }
  // Returns whether member exists
  inline bool MemberExists(const std::string& test_name) {
    return (FindMember(test_name) == -1) ? false : true;
  }
  // Returns the slot of the member, returns -1 on failure
  inline int FindMember(const std::string& test_name) {
    for (int i = 0; i < MAX_IPC_MEMBERS; i++)
      if (this->IpcMemSpace->members[i].name == test_name)
        return i;
    return -1;
  }
private:
  // Internal cleanup of the ipc space, in the case of a member not being nice >:(
  inline void IpcCleanup(){
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
  void thread_loop();
public:
  // Use to send messages through ipc, use member pos or by name, returns true on success
  bool SendMessage(std::string recipient, const char* command, const void* payload, size_t size);
  bool SendMessage(int recipient, const char* command, const void* payload, size_t size);
  void SendAll(const char* command, const void* payload, size_t size);
};

}
