/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include "websocketclient.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

// Reference implementation of a cheat-agnostic nullnexus client
class NullNexus
{
public:
    // TF2 Server info + steamid, used for authenticating with other nullnexus users
    struct TF2Server
    {
        bool connected;
        std::string ip;
        std::string port;
        std::string steamid;
        int server_spawn_count;

        bool operator==(TF2Server &other)
        {
            return connected == other.connected && ip == other.ip && port == other.port && steamid == other.steamid && server_spawn_count == other.server_spawn_count;
        }
        TF2Server(bool connected = false, std::string ip = "", std::string port = "", std::string steamid = "", int server_spawn_count = -1) : connected(connected), ip(ip), port(port), steamid(steamid), server_spawn_count(server_spawn_count)
        {
        }
    };

    struct UserSettings
    {
        std::optional<std::string> username;
        std::optional<int> colour;
        std::optional<TF2Server> tf2server;
    };

private:
    std::unique_ptr<WebSocketClient> ws;
    // Are settings set up yet?
    bool settings_set = false;
    UserSettings settings;

    // Callbacks
    std::optional<std::function<bool(boost::property_tree::ptree tree)>> callback_custom;
    std::optional<std::function<void(std::string username, std::string message, int colour)>> callback_chat;
    std::optional<std::function<void(std::vector<std::string> steamids)>> callback_authedplayers;

    bool sendAuthenticatedMessage(bool reliable, std::string type, boost::property_tree::ptree &child)
    {
        if (!ws)
            return false;
        boost::property_tree::ptree pt;
        // Basic data
        pt.put("username", *settings.username);
        pt.put("type", type);

        // Data exclusive to this request
        pt.put_child("data", child);

        std::ostringstream buf;
        write_json(buf, pt, false);
        return ws->sendMessage(buf.str(), reliable);
    }

    void handleMessage_chat(boost::property_tree::ptree &tree)
    {
        auto data = tree.get_child("data");
        if (callback_chat)
            (*callback_chat)(data.get<std::string>("user"), data.get<std::string>("msg"), data.get<int>("colour"));
    }

    void handleMessage_authedplayers(boost::property_tree::ptree &tree)
    {
        if (!callback_authedplayers)
            return;
        std::vector<std::string> steamids;
        for (auto &item : tree.get_child("data"))
            steamids.push_back(item.second.get<std::string>("steamid"));
        (*callback_authedplayers)(steamids);
    }

    void handleMessage(std::string msg)
    {
        try
        {
            // Parse message
            std::istringstream iss(msg);
            boost::property_tree::ptree pt;
            boost::property_tree::read_json(iss, pt);

            if (callback_custom)
                // If the custom callback handled this message, we should stop
                if ((*callback_custom)(pt))
                    return;

            std::string type = pt.get<std::string>("type");
            if (type == "chat")
                handleMessage_chat(pt);
            else if (type == "authedplayers")
                handleMessage_authedplayers(pt);
        }
        catch (...)
        {
        }
    }
    void setCustomHeaders()
    {
        std::vector<std::pair<std::string, std::string>> headers = { { "nullnexus_colour", std::to_string(*settings.colour) } };
        if (settings.tf2server && settings.tf2server->connected)
        {
            headers.push_back({ "nullnexus_server_ip", settings.tf2server->ip });
            headers.push_back({ "nullnexus_server_port", settings.tf2server->port });
            headers.push_back({ "nullnexus_server_steamid", settings.tf2server->steamid });
            headers.push_back({ "nullnexus_server_server_spawn_count", std::to_string(settings.tf2server->server_spawn_count) });
        }
        if (ws)
            ws->setCustomHeaders(headers);
    }

public:
    // Change some setting
    void changeData(UserSettings newsettings = UserSettings())
    {
        settings_set = true;
        {
            if ((!settings.username && !newsettings.username) || (newsettings.username && *newsettings.username == "anon"))
                settings.username = "Anon-" + std::to_string(rand() % 9000 + 1000);
            else if (!settings.username || (newsettings.username && *newsettings.username != *settings.username))
                settings.username = *newsettings.username;
        }
        // RNG colour generator
        if (!newsettings.colour && !settings.colour)
        {
            int r              = (rand() % 255 + 255) / 2;
            int g              = (rand() % 255 + 255) / 2;
            int b              = (rand() % 255 + 255) / 2;
            newsettings.colour = (r << 16) + (g << 8) + b;
        }
        boost::property_tree::ptree pt;
        // Set colour
        if (newsettings.colour && *newsettings.colour != *settings.colour)
        {
            settings.colour = *newsettings.colour;
            pt.put("colour", *settings.colour);
        }
        // Send info about current server to nullnexus instance
        if (newsettings.tf2server && (!settings.tf2server || !(*newsettings.tf2server == *settings.tf2server)))
        {
            boost::property_tree::ptree pt_server;
            pt_server.put("connected", newsettings.tf2server->connected);
            if (newsettings.tf2server->connected)
            {
                pt_server.put("ip", newsettings.tf2server->ip);
                pt_server.put("port", newsettings.tf2server->port);
                pt_server.put("steamid", newsettings.tf2server->steamid);
                pt_server.put("server_spawn_count", std::to_string(settings.tf2server->server_spawn_count));
            }
            settings.tf2server = *newsettings.tf2server;
            pt.put_child("server", pt_server);
        }
        if (pt.size())
        {
            sendAuthenticatedMessage(false, "dataupdate", pt);
            // In case of reconnect
            setCustomHeaders();
        }
    }
    void disconnect()
    {
        if (ws)
            ws->stop();
    }
    void reconnect(bool async = false)
    {
        if (ws)
            ws->stop();
        ws->start(async);
    }
    // Connect to a specific server
    void connect(std::string host = "localhost", std::string port = "3000", std::string endpoint = "/api/v1/client", bool async = false)
    {
        if (!settings_set)
            changeData();
        if (ws)
            ws.reset();
        ws = std::make_unique<WebSocketClient>(host, port, endpoint, std::bind(&NullNexus::handleMessage, this, std::placeholders::_1));
        setCustomHeaders();
        ws->start(async);
    }
#ifdef __linux__
    void connectunix(std::string socket = "/tmp/nullnexus.sock", std::string endpoint = "/api/v1/client", bool async = false)
    {
        if (!settings_set)
            changeData();
        if (ws)
            ws.reset();
        ws = std::make_unique<WebSocketClient>(socket, endpoint, std::bind(&NullNexus::handleMessage, this, std::placeholders::_1));
        setCustomHeaders();
        ws->start(async);
    }
#endif
    // Send a chat message
    bool sendChat(std::string message, std::string location = "public")
    {
        if (message.size() > 128 || message.size() < 1)
            return false;
        if (std::count_if(message.begin(), message.end(), [](unsigned char c) { return !std::isprint(c); }) != 0)
            return false;

        boost::property_tree::ptree pt;
        pt.put("msg", message);
        pt.put("loc", location);
        return sendAuthenticatedMessage(false, "chat", pt);
    }
    // Add a handler that overrides all other handlers implemented by this class.
    // Return true if your handler handled the message, false if you want the class to handle it.
    void setHandlerCustom(std::function<bool(boost::property_tree::ptree tree)> handler)
    {
        callback_custom = handler;
    }
    // Handle chat messages
    void setHandlerChat(std::function<void(std::string username, std::string message, int colour)> handler)
    {
        callback_chat = handler;
    }
    // Handle authedplayers messages
    void setHandlerAuthedplayers(std::function<void(std::vector<std::string>)> handler)
    {
        callback_authedplayers = handler;
    }
};
