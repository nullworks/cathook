/*
 * Schema.cpp
 *
 *  Created on: May 21, 2017
 *      Author: nullifiedcat
 */

#include "Schema.hpp"


#include "../../helpers.h"
#include "../../cvwrapper.h"
#include "../../logging.h"

namespace menu { namespace im {

CatVar* FindCatVar(const std::string name) {
	for (auto var : CatVarList()) {
		if (var->name == name) return var;
	}
	logging::Info("can't find %s", name.c_str());
	throw std::runtime_error("can't find catvar " + name);
}

ListEntry_Dummy* FromJson(nlohmann::json json) {
	if (json.is_string()) {
		const std::string var_name = json;
		ListEntry_Dummy* cv;
		try {
			cv = (ListEntry_Dummy*)(new ListEntry_Variable(*FindCatVar(var_name)));
		} catch (std::runtime_error& er) {
			logging::Info("[Error] %s", er.what());
			cv = (ListEntry_Dummy*)(new ListEntry_Label(format("[", var_name, "]")));
		}
		return cv;
	} else if (json.is_object()) {
#ifndef IPC_ENABLED
		if (json.find("data") != json.end()) {
			if (json["data"] == "ipc") {
				return nullptr;
			}
		}
#endif
		if (json.find("type") == json.end()) {
			logging::Info("[Warning] JSON object has no type!");
			return (ListEntry_Dummy*)(new ListEntry_Label("malformed object"));
		}
		if (json["type"] == "label") {
			std::string label = json["text"];
			return (ListEntry_Dummy*)(new ListEntry_Label(label));
		} else if (json["type"] == "var") {
			ListEntry_Variable* cv = new ListEntry_Variable(*FindCatVar(json["var"]));
			cv->data = json["data"];
			return (ListEntry_Dummy*)cv;
		} else if (json["type"] == "list") {
			const auto& list = json["list"];
			ListEntry_List* ll = new ListEntry_List();
			if (json.find("data") != json.end()) {
				ll->data = json["data"];
			}
			ll->name = json["name"];
			if (!list.is_array()) {
				logging::Info("List is not an array!");
				throw std::runtime_error("list is not an array");
			}
			for (const auto& item : list) {
				auto it = FromJson(item);
				if (it)
					ll->entries.push_back(std::auto_ptr<ListEntry_Dummy>(it));
			}
			return (ListEntry_Dummy*)ll;
		}
	}
	return (ListEntry_Dummy*)(new ListEntry_Label("malformed object"));
}

void PopulateList(nlohmann::json json) {
	try {
		if (!json.is_array()) {
			throw std::runtime_error("list is not an array");
		}
		for (const auto& item : json) {
			ListEntry_Dummy* entry = FromJson(item);
			if (entry) {
				if (entry->type != SUBLIST) {
					throw std::runtime_error("main sublist is not a list!");
				}
				main_list_array.push_back((ListEntry_List*)entry);
			}
		}
	} catch (std::exception& ex) {
		logging::Info("LIST POPULATION FATAL ERROR: %s", ex.what());
	}
}

std::vector<ListEntry_List*> main_list_array;

std::map<CatVar*, bool> map_present {};

void TraverseList(const ListEntry_List* list) {
	for (const auto& x : list->entries) {
		switch (x.get()->type) {
		case LABEL:
			//logging::Info("Label: %s", ((ListEntry_Label*)(x.get()))->text.c_str());
			break;
		case VARIABLE:
			map_present[&(((ListEntry_Variable*)(x.get()))->var)] = true;
			//logging::Info("Variable: %s", ((ListEntry_Variable*)(x.get()))->var.name.c_str());
			break;
		case SUBLIST:
			TraverseList((ListEntry_List*)x.get());
		}
	}
}

CatCommand reloadscheme("reloadscheme", "Reload Scheme", []() {
	main_list_array.clear();
	try {
		std::ifstream in(DATA_PATH "/menu.json", std::ios::in);
		nlohmann::json jo("[]");
		if (in.good()) {
			jo = jo.parse(in);
			in.close();
		}
		PopulateList(jo);
		logging::Info("Done populating list.. traversing now!");
		map_present.clear();
		for (const auto& i : main_list_array) {
			TraverseList(i);
		}
		for (const auto& v : CatVarList()) {
			if (map_present.find(v) == map_present.end()) {
				logging::Info("%s is not present in the GUI!", v->name.c_str());
			}
		}
		//TraverseList(&main_list);
	} catch (std::exception& ex) {
		logging::Info("Error: %s", ex.what());
	}
});

}}
