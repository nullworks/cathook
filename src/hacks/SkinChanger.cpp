/*
 * SkinChanger.cpp
 *
 *  Created on: May 4, 2017
 *      Author: nullifiedcat
 */

#include "SkinChanger.hpp"
#include "../copypasted/CSignature.h"

#include <sys/dir.h>
#include <sys/stat.h>

namespace hacks { namespace tf2 { namespace skinchanger {

// Because fuck you, that's why.
const char* sig_GetAttributeDefinition = "55 89 E5 57 56 53 83 EC 6C C7 45 9C 00 00 00 00 8B 75 08 C7 45 A4 00 00 00 00 8B 45 0C C6 45 A8 00 C6 45 A9 00 C6 45 AA 00 8B BE B0 01 00 00 C6 45 AB 00 C6 45 B4 00 C7 45 B8 00 00 00 00 C7 45 BC 02 00 00 00 83 FF FF C7 45 C0 00 00 00 00 C7 45 C4 00 00 00 00 C7 45 C8 00 00 00 00 C7 45 CC 00 00 00 00 C7 45 D0 00 00 00 00 C6 45 D4 00 C6 45 D5 00 C7 45 D8 FF FF FF FF C7 45 DC 00 00 00 00 89 45 98 0F 84 86 01 00 00 8B 86 A4 01 00 00 EB 21";
const char* sig_SetRuntimeAttributeValue = "55 89 E5 57 56 53 83 EC 3C 8B 5D 08 8B 4B 10 85 C9 7E 33 8B 75 0C 8B 43 04 0F B7 7E 04 66 3B 78 04 0F 84 CA 00 00 00 83 C0 10 31 D2 EB 11 66 90 89 C6 83 C0 10 66 39 78 F4 0F 84 B9 00 00 00";
const char* sig_GetItemSchema = "55 89 E5 57 56 53 83 EC 1C 8B 1D ? ? ? ? 85 DB 89 D8 74 0B 83 C4 1C 5B 5E 5F 5D C3";

ItemSystem_t ItemSystem { nullptr };
GetAttributeDefinition_t GetAttributeDefinitionFn { nullptr };
SetRuntimeAttributeValue_t SetRuntimeAttributeValueFn { nullptr };

ItemSchemaPtr_t GetItemSchema(void) {
	if (!ItemSystem) {
		ItemSystem = (ItemSystem_t)gSignatures.GetClientSignature((char*)sig_GetItemSchema);
	}
	return (void*)((uint32_t)(ItemSystem()) + 4);
}

CAttribute::CAttribute(uint16_t iAttributeDefinitionIndex, float flValue) {
	defidx = iAttributeDefinitionIndex;
	value = flValue;
}


float CAttributeList::GetAttribute(int defindex) {
	for (int i = 0; i < m_Attributes.Count(); i++) {
		const auto& a = m_Attributes[i];
		if (a.defidx == defindex) {
			return a.value;
		}
	}
	return 0.0f;
}

void CAttributeList::RemoveAttribute(int index) {
	for (int i = 0; i < m_Attributes.Count(); i++) {
		const auto& a = m_Attributes[i];
		if (a.defidx == index) {
			m_Attributes.Remove(i);
			return;
		}
	}
}

CAttributeList::CAttributeList() {}

void CAttributeList::SetAttribute(int index, float value) {
	static ItemSchemaPtr_t schema = GetItemSchema();
	AttributeDefinitionPtr_t attrib = GetAttributeDefinitionFn(schema, index);
	SetRuntimeAttributeValueFn(this, attrib, value);
	// The code below actually is unused now - but I'll keep it just in case!
	// Let's check if attribute exists already. We don't want dupes.
	/*for (int i = 0; i < m_Attributes.Count(); i++) {
		auto& a = m_Attributes[i];
		if (a.defidx == index) {
			a.value = value;
			return;
		}
	}

	if (m_Attributes.Count() > 14)
		return;

	m_Attributes.AddToTail({ index, value });
	*/
}

static CatVar enabled(CV_SWITCH, "skinchanger", "0", "Skin Changer");
static CatCommand set_attr("skinchanger_set", "Set Attribute", [](const CCommand& args) {
	unsigned attrid = strtoul(args.Arg(1), nullptr, 10);
	float attrv = strtof(args.Arg(2), nullptr);
	GetModifier(CE_INT(LOCAL_W, netvar.iItemDefinitionIndex)).Set(attrid, attrv);
	InvalidateCookie();
});
static CatCommand remove_attr("skinchanger_remove", "Remove attribute", [](const CCommand& args) {
	unsigned attrid = strtoul(args.Arg(1), nullptr, 10);
	GetModifier(CE_INT(LOCAL_W, netvar.iItemDefinitionIndex)).Remove(attrid);
	InvalidateCookie();
});
static CatCommand set_redirect("skinchanger_redirect", "Set Redirect", [](const CCommand& args) {
	unsigned redirect = strtoul(args.Arg(1), nullptr, 10);
	GetModifier(CE_INT(LOCAL_W, netvar.iItemDefinitionIndex)).defidx_redirect = redirect;
	InvalidateCookie();
});
static CatCommand dump_attrs("skinchanger_debug_attrs", "Dump attributes", []() {
	CAttributeList* list = (CAttributeList*)((uintptr_t)(RAW_ENT(LOCAL_W)) + netvar.AttributeList);
	logging::Info("ATTRIBUTE LIST: %i", list->m_Attributes.Size());
	for (int i = 0; i < list->m_Attributes.Size(); i++) {
		logging::Info("%i %.2f", list->m_Attributes[i].defidx, list->m_Attributes[i].value);
	}
});

static CatCommand list_redirects("skinchanger_redirect_list", "Dump redirects", []() {
	for (const auto& mod : modifier_map) {
		if (mod.second.defidx_redirect) {
			logging::Info("%d -> %d", mod.first, mod.second.defidx_redirect);
		}
	}
});
static CatCommand save("skinchanger_save", "Save", [](const CCommand& args) {
	std::string filename = "skinchanger";
	if (args.ArgC() > 1) {
		filename = args.Arg(1);
	}
	Save(filename);
});
static CatCommand load("skinchanger_load", "Load", [](const CCommand& args) {
	std::string filename = "skinchanger";
	if (args.ArgC() > 1) {
		filename = args.Arg(1);
	}
	Load(filename);
});
static CatCommand load_merge("skinchanger_load_merge", "Load with merge", [](const CCommand& args) {
	std::string filename = "skinchanger";
	if (args.ArgC() > 1) {
		filename = args.Arg(1);
	}
	Load(filename, true);
});
static CatCommand remove_redirect("skinchanger_remove_redirect", "Remove redirect", [](const CCommand& args) {
	unsigned redirectid = strtoul(args.Arg(1), nullptr, 10);
	GetModifier(redirectid).defidx_redirect = 0;
	logging::Info("Redirect removed");
	InvalidateCookie();
});
static CatCommand reset("skinchanger_reset", "Reset", []() {
	modifier_map.clear();
});

static CatCommand invalidate_cookies("skinchanger_bite_cookie", "Bite Cookie", InvalidateCookie);

void FrameStageNotify(int stage) {
	static int my_weapon, handle, eid, *weapon_list;
	static IClientEntity *entity, *my_weapon_ptr;
	static IClientEntity *last_weapon_out = nullptr;

	if (stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START) return;
	if (!enabled) return;
	if (CE_BAD(LOCAL_E)) return;

	if (!SetRuntimeAttributeValueFn) {
		SetRuntimeAttributeValueFn = (SetRuntimeAttributeValue_t)(gSignatures.GetClientSignature((char*)sig_SetRuntimeAttributeValue));
		logging::Info("SetRuntimeAttributeValue: 0x%08x", SetRuntimeAttributeValueFn);
	}
	if (!GetAttributeDefinitionFn) {
		GetAttributeDefinitionFn = (GetAttributeDefinition_t)(gSignatures.GetClientSignature((char*)sig_GetAttributeDefinition));
		logging::Info("GetAttributeDefinition: 0x%08x", GetAttributeDefinitionFn);
	}

	weapon_list = (int*)((unsigned)(RAW_ENT(LOCAL_E)) + netvar.hMyWeapons);
	my_weapon = CE_INT(g_pLocalPlayer->entity, netvar.hActiveWeapon);
	my_weapon_ptr = g_IEntityList->GetClientEntity(my_weapon & 0xFFF);
	if (!my_weapon_ptr) return;
	if (!vfunc<bool(*)(IClientEntity*)>(my_weapon_ptr, 190, 0)(my_weapon_ptr)) return;
	for (int i = 0; i < 4; i++) {
		handle = weapon_list[i];
		eid = handle & 0xFFF;
		if (eid < 32 || eid > HIGHEST_ENTITY) continue;
		//logging::Info("eid, %i", eid);
		entity = g_IEntityList->GetClientEntity(eid);
		if (!entity) continue;
		// TODO IsBaseCombatWeapon
		// or TODO PlatformOffset
		if (!vfunc<bool(*)(IClientEntity*)>(entity, 190, 0)(entity)) continue;
		if ((my_weapon_ptr != last_weapon_out) || !cookie.Check()) {
			GetModifier(NET_INT(entity, netvar.iItemDefinitionIndex)).Apply(eid);
		}
	}
	if ((my_weapon_ptr != last_weapon_out) || !cookie.Check())
		cookie.Update(my_weapon & 0xFFF);
	last_weapon_out = my_weapon_ptr;
}

static CatVar show_debug_info(CV_SWITCH, "skinchanger_debug", "0", "Debug Skinchanger");

void DrawText() {
	CAttributeList *list;

	if (!enabled) return;
	if (!show_debug_info) return;
	if (CE_GOOD(LOCAL_W)) {
		AddSideString(format("dIDX: ", CE_INT(LOCAL_W, netvar.iItemDefinitionIndex)));
		list = (CAttributeList*)((uintptr_t)(RAW_ENT(LOCAL_W)) + netvar.AttributeList);
		for (int i = 0; i < list->m_Attributes.Size(); i++) {
			AddSideString(format('[', i, "] ", list->m_Attributes[i].defidx, ": ", list->m_Attributes[i].value));
		}
	}
}

#define BINARY_FILE_WRITE(handle, data) handle.write(reinterpret_cast<const char*>(&data), sizeof(data))
#define BINARY_FILE_READ(handle, data) handle.read(reinterpret_cast<char*>(&data), sizeof(data))

void Save(std::string filename) {
	DIR* cathook_directory = opendir(DATA_PATH "/skinchanger");
	if (!cathook_directory) {
		logging::Info("Skinchanger directory doesn't exist, creating one!");
		mkdir(DATA_PATH "/skinchanger", S_IRWXU | S_IRWXG);
	} else closedir(cathook_directory);
	try {
		std::ofstream file(DATA_PATH "/skinchanger/" + filename, std::ios::out | std::ios::binary);
		BINARY_FILE_WRITE(file, SERIALIZE_VERSION);
		size_t size = modifier_map.size();
		BINARY_FILE_WRITE(file, size);
		for (const auto& item : modifier_map) {
			BINARY_FILE_WRITE(file, item.first);
			// modifier data isn't a POD (it contains a vector), we can't BINARY_WRITE it completely.
			BINARY_FILE_WRITE(file, item.second.defidx_redirect);
			const auto& modifiers = item.second.modifiers;
			size_t modifier_count = modifiers.size();
			BINARY_FILE_WRITE(file, modifier_count);
			// this code is a bit tricky - I'm treating vector as an array
			if (modifier_count) {
				file.write(reinterpret_cast<const char*>(modifiers.data()), modifier_count * sizeof(attribute_s));
			}
		}
		file.close();
		logging::Info("Writing successful");
	} catch (std::exception& e) {
		logging::Info("Writing unsuccessful: %s", e.what());
	}
}

void Load(std::string filename, bool merge) {
	DIR* cathook_directory = opendir(DATA_PATH "/skinchanger");
	if (!cathook_directory) {
		logging::Info("Skinchanger directory doesn't exist, creating one!");
		mkdir(DATA_PATH "/skinchanger", S_IRWXU | S_IRWXG);
	} else closedir(cathook_directory);
	try {
		std::ifstream file(DATA_PATH "/skinchanger/" + filename, std::ios::in | std::ios::binary);
		unsigned file_serialize = 0;
		BINARY_FILE_READ(file, file_serialize);
		if (file_serialize != SERIALIZE_VERSION) {
			logging::Info("Outdated/corrupted SkinChanger file! Cannot load this.");
			file.close();
			return;
		}
		size_t size = 0;
		BINARY_FILE_READ(file, size);
		logging::Info("Reading %i entries...", size);
		if (!merge)
			modifier_map.clear();
		for (int i = 0; i < size; i++) {
			int defindex;
			BINARY_FILE_READ(file, defindex);
			size_t count;
			def_attribute_modifier modifier;
			BINARY_FILE_READ(file, modifier.defidx_redirect);
			BINARY_FILE_READ(file, count);
			modifier.modifiers.resize(count);
			file.read(reinterpret_cast<char*>(modifier.modifiers.data()), sizeof(attribute_s) * count);
			if (!merge) {
				modifier_map.insert(std::make_pair(defindex, std::move(modifier)));
			} else {
				if (!modifier.Default()) {
					modifier_map[defindex] = modifier;
				}
			}
		}
		file.close();
		logging::Info("Reading successful! Result: %i entries.", modifier_map.size());
	} catch (std::exception& e) {
		logging::Info("Reading unsuccessful: %s", e.what());
	}
}

void def_attribute_modifier::Set(int id, float value) {
	for (auto& i : modifiers) {
		if (i.defidx == id) {
			i.value = value;
			return;
		}
	}
	if (modifiers.size() > 13) {
		logging::Info("Woah there, that's too many! Remove some.");
		return;
	}
	modifiers.push_back(attribute_s { id, value });
	logging::Info("Added new attribute: %i %.2f (%i)", id, value, modifiers.size());
}

void InvalidateCookie() {
	cookie.valid = false;
}

patched_weapon_cookie::patched_weapon_cookie(int entity) {}

void patched_weapon_cookie::Update(int entity) {
	IClientEntity *ent;
	CAttributeList *list;

	ent = g_IEntityList->GetClientEntity(entity);
	if (!ent || ent->IsDormant()) return;
	if (show_debug_info)
		logging::Info("Updating cookie for %i", entity); // FIXME DEBUG LOGS!
	list = (CAttributeList*)((uintptr_t)ent + netvar.AttributeList);
	attrs = list->m_Attributes.Size();
	eidx = entity;
	defidx = NET_INT(ent, netvar.iItemDefinitionIndex);
	eclass = ent->GetClientClass()->m_ClassID;
	valid = true;
}

bool patched_weapon_cookie::Check() {
	IClientEntity *ent;
	CAttributeList *list;

	if (!valid) return false;
	ent = g_IEntityList->GetClientEntity(eidx);
	if (!ent || ent->IsDormant()) return false;
	list = (CAttributeList*)((uintptr_t)ent + netvar.AttributeList);
	if (attrs != list->m_Attributes.Size()) return false;
	if (eclass != ent->GetClientClass()->m_ClassID) return false;
	if (defidx != NET_INT(ent, netvar.iItemDefinitionIndex)) return false;
	return true;
}

void def_attribute_modifier::Remove(int id) {
	auto it = modifiers.begin();
	while (it != modifiers.end()) {
		if ((*it).defidx == id) {
			it = modifiers.erase(it);
		} else {
		   ++it;
		}
	}
}

bool def_attribute_modifier::Default() const {
	return defidx_redirect == 0 && modifiers.empty();
}

void def_attribute_modifier::Apply(int entity) {
	IClientEntity *ent;
	CAttributeList *list;

	ent = g_IEntityList->GetClientEntity(entity);
	if (!ent) return;
	if (!vfunc<bool(*)(IClientEntity*)>(ent, 190, 0)(ent)) return;
	if (defidx_redirect && NET_INT(ent, netvar.iItemDefinitionIndex) != defidx_redirect) {
		NET_INT(ent, netvar.iItemDefinitionIndex) = defidx_redirect;
		if (show_debug_info)
			logging::Info("Redirect -> %i", NET_INT(ent, netvar.iItemDefinitionIndex));
		GetModifier(defidx_redirect).Apply(entity);
		return;
	}
	list = (CAttributeList*)((uintptr_t)ent + netvar.AttributeList);
	for (const auto& mod : modifiers) {
		if (mod.defidx) {
			list->SetAttribute(mod.defidx, mod.value);
		}
	}
}

def_attribute_modifier& GetModifier(int idx) {
	try {
		return modifier_map.at(idx);
	} catch (std::out_of_range& oor) {
		modifier_map.emplace(idx, def_attribute_modifier{});
		return modifier_map.at(idx);
	}
}
// A map that maps an Item Definition Index to a modifier
std::unordered_map<int, def_attribute_modifier> modifier_map {};
// A map that maps an Entity Index to a cookie
//std::unordered_map<int, patched_weapon_cookie> cookie_map {};
patched_weapon_cookie cookie { 0 };

}}}
