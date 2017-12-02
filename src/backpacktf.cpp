/*
 * backpacktf.cpp
 *
 *  Created on: Jul 23, 2017
 *      Author: nullifiedcat
 */

#include "backpacktf.hpp"
#include "json.hpp"
#include "https_request.hpp"

#include "common.hpp"

#include <thread>
#include <queue>

namespace backpacktf {

std::unordered_map<unsigned, backpack_data_s> cache {};
std::queue<backpack_data_s*> pending_queue {};
std::mutex queue_mutex {};
std::mutex cache_mutex {};

std::string api_key_s = "";
bool valid_api_key = false;

CatVar enable_bptf(CV_SWITCH, "bptf_enable", "0", "Enable backpack.tf", "Enable backpack.tf integration\nYou have to set your API key in cat_bptf_key");
CatCommand api_key("bptf_key", "Set API Key", [](const CCommand& args) {
	api_key_s = args.ArgS();
	logging::Info("API key changed!");
	valid_api_key = false;
	if (api_key_s.length() != 24) {
		logging::Info("API key must be exactly 24 characters long");
		valid_api_key = false;
	} else {
		valid_api_key = true;
	}
});


void store_data(unsigned id, float value, bool no_value, bool outdated_value);

void processing_thread() {
	logging::Info("[bp.tf] Starting the thread");
	while (true) {
		if (enabled()) {
			try {
				std::vector<backpack_data_s*> batch {};
				int count = 0;
				{
					std::lock_guard<std::mutex> lock(queue_mutex);
					while (not pending_queue.empty() && ++count < 100) {
						batch.push_back(pending_queue.front());
						pending_queue.pop();
					}
				}
				if (count) {
					logging::Info("[bp.tf] Requesting data for %d users", count);
					std::string id_list = "";
					for (const auto& x : batch) {
						x->pending = false;
						id_list += format("[U:1:", x->id, "],");
					}
					// Remove trailing ','
					id_list = id_list.substr(0, id_list.length() - 1);
					std::string query = format("steamids=", id_list, "&key=", api_key_s);
					try {
						auto sock = https::RAII_HTTPS_Socket("backpack.tf");
						std::string response = sock.get("/api/users/info/v1?" + query);
						if (response.find("HTTP/1.1 200 OK\r\n") != 0) {
							size_t status = response.find("\r\n");
							throw std::runtime_error("Response isn't 200 OK! It's " + response.substr(0, status));
						}

						std::string body = response.substr(response.find("\r\n\r\n") + 4);

						try {
							nlohmann::json data = nlohmann::json::parse(body);
							nlohmann::json users = data["users"];
							std::lock_guard<std::mutex> lock(cache_mutex);
							for (auto it = users.begin(); it != users.end(); ++it) {
								unsigned userid = strtoul(it.key().substr(5).c_str(), nullptr, 10);
								try {
									unsigned userid = strtoul(it.key().substr(5).c_str(), nullptr, 10);
									const auto& v = it.value();
									if (not v.is_object()) {
										logging::Info("Data for %u (%s) is not an object!", userid, it.key().c_str());
										continue;
									}
									std::string name = v.at("name");
									logging::Info("Parsing data for user %u (%s)", userid, name.c_str());
									if (v.find("inventory") == v.end()) {
										store_data(userid, 0, true, false);
										continue;
									}
									const auto& inv = v.at("inventory").at("440");
									if (inv.find("value") == inv.end()) {
										store_data(userid, 0, true, false);
									} else {
										float value = float(inv["value"]);
										unsigned updated = unsigned(inv["updated"]);
										store_data(userid, value * REFINED_METAL_PRICE, false, (unsigned(time(0)) - updated > OUTDATED_AGE));
									}
								} catch (std::exception& ex) {
									logging::Info("Error while parsing user %s: %s", it.key().c_str(), ex.what());
								}
							}
						} catch (std::exception& e) {
							logging::Info("[bp.tf] Exception while parsing response: %s", e.what());
						}

					} catch (std::exception& e) {
						logging::Info("[bp.tf] HTTPS exception: %s", e.what());
					}
				}
			} catch (std::exception& e) {
				logging::Info("[bp.tf] Thread exception: %s", e.what());
			}
		}
		sleep(REQUEST_INTERVAL);
	}
}

void request_data(unsigned id) {
	if (cache[id].pending) return;
	cache[id].pending = true;
	{
		std::lock_guard<std::mutex> lock(queue_mutex);
		pending_queue.push(&cache[id]);
	}
}

bool enabled() {
	return enable_bptf && valid_api_key;
}

backpack_data_s& access_data(unsigned id) {
	try {
		return cache.at(id);
	} catch (std::out_of_range& oor) {
		cache.emplace(id, backpack_data_s {});
		cache.at(id).id = id;
		return cache.at(id);
	}
}

void store_data(unsigned id, float value, bool none, bool outdated) {
	auto& d = access_data(id);
	d.last_request = unsigned(time(0));
	d.bad = false;
	d.value = value;
	d.no_value = none;
	d.outdated_value = outdated;
	d.pending = false;
}

const backpack_data_s& get_data(unsigned id) {
	auto& d = access_data(id);
	if (d.bad || ((unsigned)time(0) - MAX_CACHE_AGE > cache[id].last_request)) {
		request_data(id);
	}
	return d;
}

std::thread& GetBackpackTFThread() {
	static std::thread thread(processing_thread);
	return thread;
}

void init() {
	GetBackpackTFThread();
}

}
