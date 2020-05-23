#include "HookedMethods.hpp"
#include "MiscTemporary.hpp"
#include "CatBot.hpp"
#include "interfaces.hpp"
#include "ipc.hpp"
namespace hooked_methods
{

DEFINE_HOOKED_METHOD(ClientCmd_Unrestricted, void, IVEngineClient013* _this, const char* command)
{
    if (hacks::shared::catbot::abandon_if_ipc_bots_gte) {
        std::string command_str(command);
        
        if (ipc::peer && command_str.length() >= 20 && command_str.substr(0, 7) == "connect") {
            unsigned count_ipc = 0;

            if (command_str.substr(command_str.length() - 11, 11) == "matchmaking") {
                std::string server_ip = command_str.substr(8, command_str.length() - 20);

                auto &peer_mem = ipc::peer->memory;

                for (unsigned i = 0; i < cat_ipc::max_peers; i++) {
                    if (i != ipc::peer->client_id && !peer_mem->peer_data[i].free && peer_mem->peer_user_data[i].connected && peer_mem->peer_user_data[i].ingame.server) {

                        std::string remote_server_ip(peer_mem->peer_user_data[i].ingame.server);
                        if (remote_server_ip.compare(server_ip) == 0) {
                            count_ipc++;
                        }
                    }
                }
            }

            if (count_ipc > 0 && count_ipc >= (int)hacks::shared::catbot::abandon_if_ipc_bots_gte)
            {
                // if (hacks::shared::catbot::auto-requeue)
                tfmm::startQueue();
                return;
            }
        }
    }

    original::ClientCmd_Unrestricted(_this, command);
}
} // namespace hooked_methods
