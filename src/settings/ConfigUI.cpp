/*
 *  Created on: Feb 15, 2021
 */
#include <settings/SettingsIO.hpp>
#include <settings/String.hpp>
#include "common.hpp"
#if ENABLE_VISUALS
#include "Menu.hpp"
#include "special/ConfigsManagerList.hpp"
#endif

namespace settings::configui
{
static settings::String cfg_save{ "cfg.save", "" };
static settings::String cfg_load{ "cfg.load", "" };
static settings::String cfg_delete{ "cfg.delete", "" };

void refreshConfigList()
{
    zerokernel::Container *cl = dynamic_cast<zerokernel::Container *>(zerokernel::Menu::instance->wm->getElementById("cfg-list"));
    if (cl)
    {
        zerokernel::special::ConfigsManagerList list(*cl);
        list.construct();
        printf("CL found\n");
    }
}

void Draw()
{
    if (*cfg_save != "")
    {
        settings::SettingsWriter writer{ settings::Manager::instance() };

        DIR *config_directory = opendir(paths::getConfigPath().c_str());
        if (!config_directory)
        {
            logging::Info("Configs directory doesn't exist, creating one!");
            mkdir(paths::getConfigPath().c_str(), S_IRWXU | S_IRWXG);
        }

        writer.saveTo(paths::getConfigPath() + "/" + *cfg_save + ".conf");

        logging::Info("cat_save: Sorting configs...");
        void getAndSortAllConfigs();
        logging::Info("cat_save: Closing dir...");
        closedir(config_directory);
        refreshConfigList();

        cfg_save = "";
    }
    if (*cfg_load != "")
    {
        settings::SettingsReader loader{ settings::Manager::instance() };
        loader.loadFrom(paths::getConfigPath() + "/" + *cfg_load + ".conf");

        cfg_load = "";
    }
    if (*cfg_delete != "")
    {
        remove((paths::getConfigPath() + "/" + *cfg_delete + ".conf").c_str());
        refreshConfigList();

        cfg_delete = "";
    }
}

static InitRoutine init(
    []()
    {
#if ENABLE_VISUALS
        EC::Register(EC::Draw, Draw, "config_ui", EC::average);
#endif
    });
} // namespace settings::configui
