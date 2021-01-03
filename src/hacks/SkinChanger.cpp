/*
 * SkinChanger.cpp
 *
 *  Created on: May 4, 2017
 *      Author: nullifiedcat
 */

#include "common.hpp"
#include "MiscTemporary.hpp"

#include <sys/dir.h>
#include <sys/stat.h>
#include <hacks/SkinChanger.hpp>
#include <settings/Bool.hpp>
#include <boost/functional/hash.hpp>

namespace hacks::tf2::skinchanger
{
static settings::Boolean enable{ "skinchanger.enable", "false" };
static settings::Boolean debug{ "skinchanger.debug", "false" };

// Because fuck you, that's why.
const char *sig_GetAttributeDefinition   = "55 89 E5 57 56 53 83 EC 6C C7 45 9C 00 00 00 00";
const char *sig_SetRuntimeAttributeValue = "55 89 E5 57 56 53 83 EC 3C 8B 5D 08 8B 4B 10 85 C9 7E 33";
const char *sig_GetItemSchema            = "55 89 E5 57 56 53 83 EC 1C 8B 1D ? ? ? ? 85 "
                                "DB 89 D8 74 0B 83 C4 1C 5B 5E 5F 5D C3";

ItemSystem_t ItemSystem{ nullptr };
GetAttributeDefinition_t GetAttributeDefinitionFn{ nullptr };
SetRuntimeAttributeValue_t SetRuntimeAttributeValueFn{ nullptr };

ItemSchemaPtr_t GetItemSchema(void)
{
    if (!ItemSystem)
    {
        ItemSystem = (ItemSystem_t) gSignatures.GetClientSignature((char *) sig_GetItemSchema);
    }
    return (void *) ((uint32_t)(ItemSystem()) + 4);
}

CAttribute::CAttribute(uint16_t iAttributeDefinitionIndex, float flValue)
{
    defidx = iAttributeDefinitionIndex;
    value  = flValue;
}

float CAttributeList::GetAttribute(int defindex)
{
    for (int i = 0; i < m_Attributes.Count(); i++)
    {
        const auto &a = m_Attributes[i];
        if (a.defidx == defindex)
        {
            return a.value;
        }
    }
    return 0.0f;
}

void CAttributeList::RemoveAttribute(int index)
{
    for (int i = 0; i < m_Attributes.Count(); i++)
    {
        const auto &a = m_Attributes[i];
        if (a.defidx == index)
        {
            m_Attributes.Remove(i);
            return;
        }
    }
}

CAttributeList::CAttributeList()
{
}

void CAttributeList::SetAttribute(int index, float value)
{
    static ItemSchemaPtr_t schema   = GetItemSchema();
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

static std::array<int, 46> australium_table{ 4, 7, 13, 14, 15, 16, 18, 19, 20, 21, 29, 36, 38, 45, 61, 132, 141, 194, 197, 200, 201, 202, 203, 205, 206, 207, 208, 211, 228, 424, 654, 658, 659, 662, 663, 664, 665, 669, 1000, 1004, 1006, 1007, 1078, 1082, 1085, 1149 };
static std::array<std::pair<int, int>, 12> redirects{ std::pair{ 264, 1071 }, std::pair{ 18, 205 }, std::pair{ 13, 200 }, std::pair{ 21, 208 }, std::pair{ 19, 206 }, std::pair{ 20, 207 }, std::pair{ 15, 202 }, std::pair{ 7, 197 }, std::pair{ 29, 211 }, std::pair{ 14, 201 }, std::pair{ 16, 203 }, std::pair{ 4, 194 } };
static CatCommand australize("australize", "Make everything australium", []() {
    enable = true;
    for (auto i : redirects)
        GetModifier(i.first).defidx_redirect = i.second;
    for (auto i : australium_table)
    {
        GetModifier(i).Set(2027, 1);
        GetModifier(i).Set(2022, 1);
        GetModifier(i).Set(542, 1);
    }
    InvalidateCookie();
});
static CatCommand set_attr("skinchanger_set", "Set Attribute", [](const CCommand &args) {
    if (args.ArgC() < 2)
    {
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Please Provide an Argument\n");
        return;
    }
    enable = true;
    try
    {
        if (CE_BAD(LOCAL_W))
            return;
        unsigned attrid = std::strtoul(args.Arg(1), nullptr, 10);
        float attrv     = std::strtof(args.Arg(2), nullptr);
        GetModifier(CE_INT(LOCAL_W, netvar.iItemDefinitionIndex)).Set(attrid, attrv);
        InvalidateCookie();
    }
    catch (const std::invalid_argument &)
    {
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Please pass a valid int\n");
    }
});
static CatCommand remove_attr("skinchanger_remove", "Remove attribute", [](const CCommand &args) {
    if (args.ArgC() < 2)
    {
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Please Provide an Argument\n");
        return;
    }
    try
    {
        if (CE_BAD(LOCAL_W))
            return;
        enable          = true;
        unsigned attrid = std::strtoul(args.Arg(1), nullptr, 10);
        GetModifier(CE_INT(LOCAL_W, netvar.iItemDefinitionIndex)).Remove(attrid);
        InvalidateCookie();
    }
    catch (const std::invalid_argument &)
    {
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Please pass a valid int\n");
    }
});
static CatCommand set_redirect("skinchanger_redirect", "Set Redirect", [](const CCommand &args) {
    if (args.ArgC() < 2)
    {
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Please Provide an Argument\n");
        return;
    }
    try
    {
        if (CE_BAD(LOCAL_W))
            return;
        enable                                                                    = true;
        unsigned redirect                                                         = std::strtoul(args.Arg(1), nullptr, 10);
        GetModifier(CE_INT(LOCAL_W, netvar.iItemDefinitionIndex)).defidx_redirect = redirect;
        InvalidateCookie();
    }
    catch (const std::invalid_argument &)
    {
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Please pass a valid int\n");
    }
});
static CatCommand dump_attrs("skinchanger_debug_attrs", "Dump attributes", []() {
    CAttributeList *list = (CAttributeList *) ((uintptr_t)(RAW_ENT(LOCAL_W)) + netvar.AttributeList);
    logging::Info("ATTRIBUTE LIST: %i", list->m_Attributes.Size());
    for (int i = 0; i < list->m_Attributes.Size(); i++)
    {
        logging::Info("%i %.2f", list->m_Attributes[i].defidx, list->m_Attributes[i].value);
    }
});

static CatCommand list_redirects("skinchanger_redirect_list", "Dump redirects", []() {
    for (const auto &mod : modifier_map)
    {
        if (mod.second.defidx_redirect)
        {
            g_ICvar->ConsoleColorPrintf(MENU_COLOR, "%d -> %d\n", mod.first, mod.second.defidx_redirect);
        }
    }
});
static CatCommand save("skinchanger_save", "Save", [](const CCommand &args) {
    std::string filename = "skinchanger";
    if (args.ArgC() > 1)
    {
        filename = args.Arg(1);
    }
    Save(filename);
});
static CatCommand load("skinchanger_load", "Load", [](const CCommand &args) {
    enable               = true;
    std::string filename = "skinchanger";
    if (args.ArgC() > 1)
    {
        filename = args.Arg(1);
    }
    Load(filename);
});
static CatCommand load_merge("skinchanger_load_merge", "Load with merge", [](const CCommand &args) {
    enable               = true;
    std::string filename = "skinchanger";
    if (args.ArgC() > 1)
    {
        filename = args.Arg(1);
    }
    Load(filename, true);
});
static CatCommand remove_redirect("skinchanger_remove_redirect", "Remove redirect", [](const CCommand &args) {
    if (args.ArgC() < 2)
    {
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Please Provide an argument\n");
    }
    try
    {
        if (CE_BAD(LOCAL_W))
            return;
        enable                                  = true;
        unsigned redirectid                     = std::strtoul(args.Arg(1), nullptr, 10);
        GetModifier(redirectid).defidx_redirect = 0;
        logging::Info("Redirect removed");
        InvalidateCookie();
    }
    catch (const std::invalid_argument &)
    {
        g_ICvar->ConsoleColorPrintf(MENU_COLOR, "Please supply a valid int\n");
    }
});
static CatCommand reset("skinchanger_reset", "Reset", []() { modifier_map.clear(); });

static CatCommand invalidate_cookies("skinchanger_bite_cookie", "Bite Cookie", InvalidateCookie);

void FrameStageNotify(int stage)
{
    static int my_weapon, handle, eid, *weapon_list;
    static IClientEntity *entity, *my_weapon_ptr;
    static IClientEntity *last_weapon_out = nullptr;

    if (stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
        return;
    if (!enable)
        return;
    if (CE_BAD(LOCAL_E))
        return;

    if (!SetRuntimeAttributeValueFn)
    {
        SetRuntimeAttributeValueFn = (SetRuntimeAttributeValue_t)(gSignatures.GetClientSignature((char *) sig_SetRuntimeAttributeValue));
        logging::Info("SetRuntimeAttributeValue: 0x%08x", SetRuntimeAttributeValueFn);
    }
    if (!GetAttributeDefinitionFn)
    {
        GetAttributeDefinitionFn = (GetAttributeDefinition_t)(gSignatures.GetClientSignature((char *) sig_GetAttributeDefinition));
        logging::Info("GetAttributeDefinition: 0x%08x", GetAttributeDefinitionFn);
    }

    weapon_list   = (int *) ((unsigned) (RAW_ENT(LOCAL_E)) + netvar.hMyWeapons);
    my_weapon     = CE_INT(g_pLocalPlayer->entity, netvar.hActiveWeapon);
    my_weapon_ptr = g_IEntityList->GetClientEntity(my_weapon & 0xFFF);
    if (!my_weapon_ptr)
        return;
    if (!re::C_BaseCombatWeapon::IsBaseCombatWeapon(my_weapon_ptr))
        return;
    for (int i = 0; i < 4; i++)
    {
        handle = weapon_list[i];
        eid    = handle & 0xFFF;
        if (eid <= MAX_PLAYERS || eid > HIGHEST_ENTITY)
            continue;
        // logging::Info("eid, %i", eid);
        entity = g_IEntityList->GetClientEntity(eid);
        if (!entity)
            continue;
        // TODO IsBaseCombatWeapon
        // or TODO PlatformOffset
        if (!re::C_BaseCombatWeapon::IsBaseCombatWeapon(entity))
            continue;
        if ((my_weapon_ptr != last_weapon_out) || !cookie.Check())
        {
            GetModifier(NET_INT(entity, netvar.iItemDefinitionIndex)).Apply(eid);
        }
    }
    if ((my_weapon_ptr != last_weapon_out) || !cookie.Check())
        cookie.Update(my_weapon & 0xFFF);
    last_weapon_out = my_weapon_ptr;
}

void DrawText()
{
    CAttributeList *list;

    if (!enable)
        return;
    if (!debug)
        return;
    if (CE_GOOD(LOCAL_W))
    {
        AddSideString(format("dIDX: ", CE_INT(LOCAL_W, netvar.iItemDefinitionIndex)));
        list = (CAttributeList *) ((uintptr_t)(RAW_ENT(LOCAL_W)) + netvar.AttributeList);
        for (int i = 0; i < list->m_Attributes.Size(); i++)
        {
            AddSideString(format('[', i, "] ", list->m_Attributes[i].defidx, ": ", list->m_Attributes[i].value));
        }
    }
}

#define BINARY_FILE_WRITE(handle, data) handle.write(reinterpret_cast<const char *>(&data), sizeof(data))
#define BINARY_FILE_READ(handle, data) handle.read(reinterpret_cast<char *>(&data), sizeof(data))

void Save(std::string filename)
{
    DIR *cathook_directory = opendir(paths::getDataPath("/skinchanger").c_str());
    if (!cathook_directory)
    {
        logging::Info("Skinchanger directory doesn't exist, creating one!");
        mkdir(paths::getDataPath("/skinchanger").c_str(), S_IRWXU | S_IRWXG);
    }
    else
        closedir(cathook_directory);
    try
    {
        std::ofstream file(paths::getDataPath("/skinchanger/" + filename), std::ios::out | std::ios::binary);
        BINARY_FILE_WRITE(file, SERIALIZE_VERSION);
        size_t size = modifier_map.size();
        BINARY_FILE_WRITE(file, size);
        for (const auto &item : modifier_map)
        {
            BINARY_FILE_WRITE(file, item.first);
            // modifier data isn't a POD (it contains a vector), we can't
            // BINARY_WRITE it completely.
            BINARY_FILE_WRITE(file, item.second.defidx_redirect);
            const auto &modifiers = item.second.modifiers;
            size_t modifier_count = modifiers.size();
            BINARY_FILE_WRITE(file, modifier_count);
            // this code is a bit tricky - I'm treating vector as an array
            if (modifier_count)
            {
                file.write(reinterpret_cast<const char *>(modifiers.data()), modifier_count * sizeof(attribute_s));
            }
        }
        file.close();
        logging::Info("Writing successful");
    }
    catch (std::exception &e)
    {
        logging::Info("Writing unsuccessful: %s", e.what());
    }
}

void Load(std::string filename, bool merge)
{
    DIR *cathook_directory = opendir(paths::getDataPath("/skinchanger").c_str());
    if (!cathook_directory)
    {
        logging::Info("Skinchanger directory doesn't exist, creating one!");
        mkdir(paths::getDataPath("/skinchanger").c_str(), S_IRWXU | S_IRWXG);
    }
    else
        closedir(cathook_directory);
    try
    {
        std::ifstream file(paths::getDataPath("/skinchanger/" + filename), std::ios::in | std::ios::binary);
        unsigned file_serialize = 0;
        BINARY_FILE_READ(file, file_serialize);
        if (file_serialize != SERIALIZE_VERSION)
        {
            logging::Info("Outdated/corrupted SkinChanger file! Cannot load this.");
            file.close();
            return;
        }
        size_t size = 0;
        BINARY_FILE_READ(file, size);
        logging::Info("Reading %i entries...", size);
        if (!merge)
            modifier_map.clear();
        for (int i = 0; i < size; i++)
        {
            int defindex;
            BINARY_FILE_READ(file, defindex);
            size_t count;
            def_attribute_modifier modifier;
            BINARY_FILE_READ(file, modifier.defidx_redirect);
            BINARY_FILE_READ(file, count);
            modifier.modifiers.resize(count);
            file.read(reinterpret_cast<char *>(modifier.modifiers.data()), sizeof(attribute_s) * count);
            if (!merge)
            {
                modifier_map.insert(std::make_pair(defindex, std::move(modifier)));
            }
            else
            {
                if (!modifier.Default())
                {
                    modifier_map[defindex] = modifier;
                }
            }
        }
        file.close();
        logging::Info("Reading successful! Result: %i entries.", modifier_map.size());
    }
    catch (std::exception &e)
    {
        logging::Info("Reading unsuccessful: %s", e.what());
    }
}

void def_attribute_modifier::Set(int id, float value)
{
    for (auto &i : modifiers)
    {
        if (i.defidx == id)
        {
            i.value = value;
            return;
        }
    }
    if (modifiers.size() > 13)
    {
        logging::Info("Woah there, that's too many! Remove some.");
        return;
    }
    modifiers.push_back(attribute_s{ (uint16_t) id, value });
    logging::Info("Added new attribute: %i %.2f (%i)", id, value, modifiers.size());
}

void InvalidateCookie()
{
    cookie.valid = false;
}

patched_weapon_cookie::patched_weapon_cookie(int entity)
{
}

void patched_weapon_cookie::Update(int entity)
{
    IClientEntity *ent;
    CAttributeList *list;

    ent = g_IEntityList->GetClientEntity(entity);
    if (!ent || ent->IsDormant())
        return;
    if (debug)
        logging::Info("Updating cookie for %i", entity); // FIXME DEBUG LOGS!
    list   = (CAttributeList *) ((uintptr_t) ent + netvar.AttributeList);
    attrs  = list->m_Attributes.Size();
    eidx   = entity;
    defidx = NET_INT(ent, netvar.iItemDefinitionIndex);
    eclass = ent->GetClientClass()->m_ClassID;
    valid  = true;
}

bool patched_weapon_cookie::Check()
{
    IClientEntity *ent;
    CAttributeList *list;

    if (!valid)
        return false;
    ent = g_IEntityList->GetClientEntity(eidx);
    if (!ent || ent->IsDormant())
        return false;
    list = (CAttributeList *) ((uintptr_t) ent + netvar.AttributeList);
    if (attrs != list->m_Attributes.Size())
        return false;
    if (eclass != ent->GetClientClass()->m_ClassID)
        return false;
    if (defidx != NET_INT(ent, netvar.iItemDefinitionIndex))
        return false;
    return true;
}

void def_attribute_modifier::Remove(int id)
{
    auto it = modifiers.begin();
    while (it != modifiers.end())
    {
        if ((*it).defidx == id)
        {
            it = modifiers.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

bool def_attribute_modifier::Default() const
{
    return defidx_redirect == 0 && modifiers.empty();
}

void def_attribute_modifier::Apply(int entity)
{
    IClientEntity *ent;
    CAttributeList *list;

    ent = g_IEntityList->GetClientEntity(entity);
    if (!ent)
        return;
    if (!re::C_BaseCombatWeapon::IsBaseCombatWeapon(ent))
        return;
    if (defidx_redirect && NET_INT(ent, netvar.iItemDefinitionIndex) != defidx_redirect)
    {
        NET_INT(ent, netvar.iItemDefinitionIndex) = defidx_redirect;
        if (debug)
            logging::Info("Redirect -> %i", NET_INT(ent, netvar.iItemDefinitionIndex));
        GetModifier(defidx_redirect).Apply(entity);
        return;
    }
    list = (CAttributeList *) ((uintptr_t) ent + netvar.AttributeList);
    for (const auto &mod : modifiers)
    {
        if (mod.defidx)
        {
            list->SetAttribute(mod.defidx, mod.value);
        }
    }
}

def_attribute_modifier &GetModifier(int idx)
{
    try
    {
        return modifier_map.at(idx);
    }
    catch (std::out_of_range &oor)
    {
        modifier_map.emplace(idx, def_attribute_modifier{});
        return modifier_map.at(idx);
    }
}
// A map that maps an Item Definition Index to a modifier
std::unordered_map<int, def_attribute_modifier> modifier_map{};
// A map that maps an Entity Index to a cookie
// std::unordered_map<int, patched_weapon_cookie> cookie_map {};
patched_weapon_cookie cookie{ 0 };
} // namespace hacks::tf2::skinchanger
