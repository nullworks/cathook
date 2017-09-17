/*
 * SkinChanger.hpp
 *
 *  Created on: May 4, 2017
 *      Author: nullifiedcat
 */

#ifndef HACKS_SKINCHANGER_HPP_
#define HACKS_SKINCHANGER_HPP_

#include "../common.h"

namespace hacks { namespace tf2 { namespace skinchanger {

class CAttributeList;
class CAttribute;

typedef void* ItemSchemaPtr_t;
typedef void* AttributeDefinitionPtr_t;

// FIXME move to separate header

typedef ItemSchemaPtr_t(*ItemSystem_t)(void);
typedef void*(*SetRuntimeAttributeValue_t)(CAttributeList*, AttributeDefinitionPtr_t, float);
typedef AttributeDefinitionPtr_t(*GetAttributeDefinition_t)(ItemSchemaPtr_t, int);
ItemSchemaPtr_t GetItemSchema(void);

extern const char* sig_GetItemSchema;
extern const char* sig_GetAttributeDefinition;
extern const char* sig_SetRuntimeAttributeValue;
extern ItemSystem_t ItemSystemFn;
extern GetAttributeDefinition_t GetAttributeDefinitionFn;
extern SetRuntimeAttributeValue_t SetRuntimeAttributeValueFn;

// TOTALLY NOT A PASTE.
// Seriously tho, it's modified at least.
// Credits: blackfire62

struct attribute_s {
	uint16_t defidx;
	float value;
};

class CAttribute {
public:
	CAttribute(uint16_t iAttributeDefinitionIndex, float flValue);
public:
	void* vtable;
	uint16_t defidx;
	float value;
	unsigned int pad01;
};

class CAttributeList {
public:
	CAttributeList();
	float GetAttribute(int defindex);
	void SetAttribute(int index, float value);
	void RemoveAttribute(int index);
public:
	uint32_t unknown;
	CUtlVector<CAttribute, CUtlMemory<CAttribute>> m_Attributes;
};

enum class Attributes {
	loot_rarity = 2022,
	is_australium_item = 2027,
	item_style_override = 542,
	sheen = 2014,
	killstreak_tier = 2025,
	set_item_texture_wear = 725
};

enum class UnusualEffects {
	HOT = 701,
	ISOTOPE,
	COOL,
	ENERGY_ORB
};

enum class Sheens {
	TEAM_SHINE = 1,
	DEADLY_DAFFODIL,
	MANNDARIN,
	MEAN_GREEN,
	AGONIZING_EMERALD,
	VILLAINOUS_VIOLET,
	HOT_ROD
};

struct patched_weapon_cookie {
	patched_weapon_cookie(int entity);
	void Update(int entity);
	bool Check();
public:
	int eidx { 0 };
	int defidx { 0 };
	int eclass { 0 };
	int attrs { 0 };
	bool valid { false };
};

struct def_attribute_modifier {
	bool Default() const;
	void Apply(int entity);
	void Set(int id, float value);
	void Remove(int id);
	int defidx { 0 };
	int defidx_redirect { 0 };
	std::vector<attribute_s> modifiers { };
};

extern std::unordered_map<int, def_attribute_modifier> modifier_map;
extern patched_weapon_cookie cookie;
//extern std::unordered_map<int, patched_weapon_cookie> cookie_map;

def_attribute_modifier& GetModifier(int idx);
//patched_weapon_cookie& GetCookie(int idx);

constexpr unsigned SERIALIZE_VERSION = 1;
void Save(std::string filename);
void Load(std::string filename, bool merge = false);

void InvalidateCookie();
void FrameStageNotify(int stage);
void DrawText();

}}}

#endif /* HACKS_SKINCHANGER_HPP_ */
