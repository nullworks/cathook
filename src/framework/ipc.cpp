
/*
 *
 * Nekohooks "ipc" isnt an ipc in the traditional since but it still allows us to communicate between processes...
 *
 *
 */

#include <chrono> // For time so we can sleep
#include <thread> // Threads are useful!
#include <fstream>
#include <queue>

#include "../util/logging.hpp"
#include "../util/strings.hpp"

#include "ipc.hpp"

namespace ipc {
const char ipc_path[] = P_tmpdir "/neko_ipc.tmp";
// Ipc stream class
class IpcStream {
public:
  IpcStream() {
    // Start a thread for the ipc
    g_CatLogging.log("IPC: Starting Thread");
    std::thread ipc_thread([&](){this->ipc_run();});
    ipc_thread.detach();
  }
  void IssueCommand(std::string in) {
    //g_CatLogging.log("IPC: Issuing command: \"%s\"", in.c_str());
    in += "\n";
    std::fstream ipc_stream(ipc_path, std::ios_base::app);
    ipc_stream.seekg(std::ios_base::end);
    ipc_stream.write(in.c_str(), in.size());
    ipc_stream.close();
  }
private:
  bool shutdown = false;
  std::vector<std::string> ipc_members;
  std::queue<std::string> queued_commands; // Init with ping to let ourself know about other clients
  void ipc_run() {
    // Make sure ipc file exists
    /*std::ifstream exist_test(ipc_path);
    if (!exist_test) {
      g_CatLogging.log("IPC: Creating log");
      std::fstream make_file(ipc_path, std::ios_base::out | std::ios_base::trunc);
      make_file.write("tmp\n", 4);
      make_file.close();
    } else
      exist_test.close();

    // Setup ipc stream to trail
    std::fstream ipc_stream(ipc_path);
    ipc_stream.seekg(std::ios_base::end);

    // Ping to get members on ipc
    IssueCommand("neko ping");
    queued_commands.push("neko ping");
    // Sleep to get replys from other members
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    auto last_ping = std::chrono::steady_clock::now();

    // Main loop
    g_CatLogging.log("IPC: Running Thread: %i", std::this_thread::get_id());
    std::string ipc_name;
    while (!shutdown) {
      // Simple func to check if we are at the end of the stream
      auto AtEnd = [&]() -> bool {
        auto pos = ipc_stream.tellg();
        ipc_stream.seekg(std::ios_base::end);
        if (pos == ipc_stream.tellg())
          return true;
        ipc_stream.seekg(pos);
        return false;
      };
      // Try to read from ipc file
      if (!AtEnd()) {
        std::string command;
        auto time_start = std::chrono::steady_clock::now(); // to time out just in case ;)
        while(true) {
          // Check is we should timeout
          if (std::chrono::steady_clock::now() - time_start > std::chrono::milliseconds(100)) {
            g_CatLogging.log("IPC: Timeout on Command: %s", command.c_str());
            break;
          // is we are at the end, we should recurse to wait for more info
          } else if (AtEnd()) {
            continue;
          // Get a char from the stream and determine whether to add it as a command to the queue
          } else {
            auto tmp = ipc_stream.get();
            if (tmp != '\n')
              command.push_back(tmp);
            else {
              queued_commands.push(command);
              g_CatLogging.log("IPC: Queued Command: %s", command.c_str());
              break;
            }
          }
        }
      // Without reading, run commands
      } else if (!queued_commands.empty()) {
        // Seperating strings makes processing stuff really easy
        auto tmp = sepstr(queued_commands.front());
        g_CatLogging.log("IPC: Command: \"%s\"", queued_commands.front().c_str());
        // Ensure we have a header for the command, this ensures that its an ipc command and not some jibberish
        if (tmp.at(0) != "neko") {
          g_CatLogging.log("IPC: Missing header on command: %s", tmp.at(0));
        // EVERYTHING BELOW IS PING LOGIC
        } else if (tmp.at(1) == "ping") {
          // If we dont have a name, we cant reply yet, so we wait for others to respond then run through this gain
          if (ipc_name.empty()) {
            g_CatLogging.log("IPC: Pinging to get name!");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            queued_commands.push("neko ping");
            ipc_name = "undefined";
          } else {
            // If we just set the name to undef from above due to no name, we look for a name that hasnt been used yet
            if (ipc_name == "undefined") {
              int i = 0;
              while ([&]() -> bool {
                for (const auto& ii : ipc_members) {
                  if (ii == std::string("neko_") + std::to_string(i)) {
                    return true;
                  }
                }
                return false;
              }()) i++;
              ipc_name = std::string("neko_") + std::to_string(i);
              g_CatLogging.log("IPC: Got name: \"%s\"", ipc_name.c_str());
            // if we didnt have a name then we dont clear as we now know the members
            // Otherwise we clear this normally as members are responding after this
            } else
              ipc_members.clear();
            // but we also respond to the ping with our own name, we should have this now and forever
            IssueCommand("neko ping_r " + ipc_name);
            last_ping = std::chrono::steady_clock::now();
          }
        // Responce to ping, should give us members
        } else if (tmp.at(1) == "ping_r") {
          ipc_members.push_back(tmp.at(2));
        // If command is sent to us
        } else if (tmp.at(1) == ipc_name) {

        }
        queued_commands.pop();
        std::this_thread::sleep_for(std::chrono::milliseconds(25)); // Pause after running a command
      // Ping keeper
      } else if (std::chrono::steady_clock::now() - last_ping > std::chrono::seconds(3)) {
        last_ping = std::chrono::steady_clock::now();
        IssueCommand("neko ping");
      }
    }
    // Shutdown
    g_CatLogging.log("IPC: Thread \"%i\" shutting down!", std::this_thread::get_id());
    ipc_stream.close();*/
  }
public:
  ~IpcStream() {
    g_CatLogging.log("IPC: Issuing shutdown to IPC thread!");
    shutdown = true;
  }
};

}

ipc::IpcStream g_IpcStream;
