/*
 * Credits to the Hoster Alex for making this and giving us the permission
 * To add it to Cathook.
 *
 * Refined and fixed by BenCat07
 */
/* License: GPLv3 */
#include <common.hpp>
/* For MENU_COLOR definition */
#include <MiscTemporary.hpp>
/* e8call_direct */
#include <e8call.hpp>
#include <set>
/* For Textmode */
#include <map>
/* Splitting strings nicely */
#include <boost/algorithm/string.hpp>

/* Global switch for DataCenter hooks */
static settings::Boolean enable{ "dc.enable", "false" };
/* List of preferred data centers separated by comma ',' */
static settings::String regions{ "dc.regions", "" };
/* Find servers only in data centers provided by regions */
static settings::Boolean restrict { "dc.restrict", "true" };
/* Enable/disable individual continents */
static settings::Boolean enable_eu{ "dc.toggle-europe", "false" };
static settings::Boolean enable_north_america{ "dc.toggle-north-america", "false" };
static settings::Boolean enable_south_america{ "dc.toggle-south-america", "false" };
static settings::Boolean enable_asia{ "dc.toggle-asia", "false" };
static settings::Boolean enable_oceania{ "dc.toggle-oceania", "false" };
static settings::Boolean enable_africa{ "dc.toggle-africa", "false" };

typedef std::array<char, 5> CidStr_t;

struct SteamNetworkingPOPID_decl
{
    unsigned v;
    /* 'out' must point to array with capacity at least 5 */
    void ToString(char *out)
    {
        out[0] = char(v >> 16);
        out[1] = char(v >> 8);
        out[2] = char(v);
        out[3] = char(v >> 24);
        out[4] = 0;
    }
};

static std::set<CidStr_t> regionsSet;

static void *g_ISteamNetworkingUtils;

// clang-format off
static boost::unordered_flat_map<std::string,std::string> dc_name_map{
    {"ams", "Amsterdam"},
    {"atl", "Atlanta"},
    {"bom", "Mumbai"},
    {"dxb", "Dubai"},
    {"eat", "Moses Lake/Washington"},
    {"mwh", "Moses Lake/Washington"},
    {"fra", "Frankfurt"},
    {"gnrt", "Tokyo (gnrt)"},
    {"gru", "Sao Paulo"},
    {"hkg", "Honk Kong"},
    {"iad", "Sterling/Virginia"},
    {"jnb", "Johannesburg"},
    {"lax", "Los Angelos"},
    {"lhr", "London"},
    {"lim", "Lima"},
    {"lux", "Luxembourg"},
    {"maa", "Chennai"},
    {"mad", "Madrid"},
    {"man", "Manilla"},
    {"okc", "Oklahoma City"},
    {"ord", "Chicago"},
    {"par", "Paris"},
    {"scl", "Santiago"},
    {"sea", "Seaattle"},
    {"sgp", "Singapore"},
    {"sto", "Stockholm (Kista)"},
    {"sto2", "Stockholm (Bromma)"},
    {"syd", "Sydney"},
    {"tyo", "Tokyo (North)"},
    {"tyo2", "Tokyo (North)"},
    {"tyo1", "Tokyo (South)"},
    {"vie", "Vienna"},
    {"waw", "Warsaw"}};
// clang-format on
static std::vector<std::string> eu_datacenters            = { { "ams" }, { "fra" }, { "lhr" }, { "mad" }, { "par" }, { "sto" }, { "sto2" }, { "waw" }, { "lux" }, { "lux1" }, { "lux2" } };
static std::vector<std::string> north_america_datacenters = { { "atl" }, { "eat" }, { "mwh" }, { "iad" }, { "lax" }, { "okc" }, { "ord" }, { "sea" } };
static std::vector<std::string> south_america_datacenters = { { "gru" }, { "lim" }, { "scl" } };
static std::vector<std::string> asia_datacenters          = { { "bom" }, { "dxb" }, { "gnrt" }, { "hkg" }, { "maa" }, { "man" }, { "sgp" }, { "tyo" }, { "tyo2" }, { "tyo1" } };
static std::vector<std::string> oceana_datacenters        = { { "syd" }, { "vie" } };
static std::vector<std::string> africa_datacenters        = { { "jnb" } };

static CatCommand print("dc_print", "Print codes of all available data centers",
                        []()
                        {
                            static auto GetPOPCount = *(int (**)(void *))(*(uintptr_t *) g_ISteamNetworkingUtils + 37);
                            static auto GetPOPList  = *(int (**)(void *, SteamNetworkingPOPID_decl *, int))(*(uintptr_t *) g_ISteamNetworkingUtils + 41);

                            char region[5];

                            int count = GetPOPCount(g_ISteamNetworkingUtils);
                            if (count <= 0)
                            {
                                g_ICvar->ConsoleColorPrintf(MENU_COLOR, "List of regions is not available yet\n");
                                return;
                            }
                            SteamNetworkingPOPID_decl *list = new SteamNetworkingPOPID_decl[count];
                            GetPOPList(g_ISteamNetworkingUtils, list, count);

                            auto it = list;
                            while (count--)
                            {
                                (it++)->ToString(region);
                                std::string region_name = dc_name_map[region];
                                if (region_name == "")
                                    region_name = "Not set";
                                g_ICvar->ConsoleColorPrintf(MENU_COLOR, "%s [%s]\n", region_name.c_str(), region);
                            }
                            delete[] list;
                        });

static void Refresh()
{
    auto gc = (bool *) re::CTFGCClientSystem::GTFGCClientSystem();
    if (!gc)
        return;
    /* GC's flag to force ping refresh */
    gc[0x374] = true;
}

static CatCommand force_refersh("dc_refresh", "Force refresh of ping data", Refresh);

static void OnRegionsUpdate(std::string regions)
{
    CidStr_t region;

    regionsSet.clear();

    std::vector<std::string> regions_vec;
    boost::split(regions_vec, regions, boost::is_any_of(","));
    for (auto &region_str : regions_vec)
    {
        if (region_str == "")
            continue;
        if (region_str.length() > 4)
        {
            logging::Info("Ignoring invalid region %s", region_str.c_str());
            continue;
        }
        region.fill(0);
        std::strcpy(region.data(), region_str.c_str());
        regionsSet.emplace(region);
    }
    if (*enable)
        Refresh();
}

static int (*o_GetDirectPingToPOP)(void *self, SteamNetworkingPOPID_decl cid);
static int h_GetDirectPingToPOP(void *self, SteamNetworkingPOPID_decl cid)
{
    CidStr_t cidStr;

    if (regionsSet.empty())
        return UniformRandomInt(5, 30);

    cid.ToString(cidStr.data());
    auto it = regionsSet.find(cidStr);
    if (it != regionsSet.end())
        return UniformRandomInt(5, 30);

    return *restrict ? UniformRandomInt(500, 800) : o_GetDirectPingToPOP(self, cid);
}

static void Hook(bool on)
{
    static hooks::VMTHook v;

    if (on)
    {
        v.Set(g_ISteamNetworkingUtils);
        v.HookMethod(h_GetDirectPingToPOP, 9, &o_GetDirectPingToPOP);
        v.Apply();
    }
    else
        v.Release();

    Refresh();
}

// if add is false it will remove instead
void manageRegions(std::vector<std::string> regions_vec, bool add)
{
    std::set<std::string> regions_split;
    if ((*regions).length())
        boost::split(regions_split, *regions, boost::is_any_of(","));

    std::set<std::string> new_regions = regions_split;

    if (add)
    {
        for (auto &region : regions_vec)
        {
            auto found = regions_split.find(region) != regions_split.end();
            if (!found)
                new_regions.emplace(region);
        }
    }
    else
    {
        for (auto &region : regions_vec)
        {
            auto position = new_regions.find(region);
            if (position != new_regions.end())
                new_regions.erase(position);
        }
    }
    regions = boost::join(new_regions, ",");
}

static void Init()
{
    enable.installChangeCallback(
        [](settings::VariableBase<bool> &, bool on)
        {
            if (!g_ISteamNetworkingUtils)
            {
                g_ISteamNetworkingUtils = ((void *(*) ())(dlsym(sharedobj::steamnetworkingsockets().lmap, "SteamNetworkingUtils_LibV4")))();
                if (!g_ISteamNetworkingUtils)
                {
                    logging::Info("DataCenter.cpp: Failed to create ISteamNetworkingUtils!");
                    return;
                }
            }
            Hook(on);
        });
    regions.installChangeCallback([](settings::VariableBase<std::string> &, std::string regions) { OnRegionsUpdate(regions); });
    restrict.installChangeCallback([](settings::VariableBase<bool> &, bool) { Refresh(); });
    enable_eu.installChangeCallback([](settings::VariableBase<bool> &, bool after) { manageRegions(eu_datacenters, after); });
    enable_north_america.installChangeCallback([](settings::VariableBase<bool> &, bool after) { manageRegions(north_america_datacenters, after); });
    enable_south_america.installChangeCallback([](settings::VariableBase<bool> &, bool after) { manageRegions(south_america_datacenters, after); });
    enable_asia.installChangeCallback([](settings::VariableBase<bool> &, bool after) { manageRegions(asia_datacenters, after); });
    enable_oceania.installChangeCallback([](settings::VariableBase<bool> &, bool after) { manageRegions(oceana_datacenters, after); });
    enable_africa.installChangeCallback([](settings::VariableBase<bool> &, bool after) { manageRegions(africa_datacenters, after); });
}

static InitRoutine init(Init);
