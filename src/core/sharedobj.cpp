/*
 * sharedobj.cpp
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#include "common.hpp"

namespace sharedobj
{

bool LocateSharedObject(std::string &name, std::string &out_full_path)
{
    FILE *proc_maps = fopen(strfmt(XORSTR("/proc/%i/maps"), getpid()), XORSTR("r"));
    if (proc_maps == nullptr)
    {
        return false;
    }
    char buffer[512];
    while (fgets(buffer, 511, proc_maps))
    {
        char *path     = strchr(buffer, '/');
        char *filename = strrchr(buffer, '/') + 1;
        if (not path or not filename)
            continue;
        if (!strncmp(name.c_str(), filename, name.length()))
        {
            out_full_path.assign(path);
            out_full_path.resize(out_full_path.size() - 1);
            fclose(proc_maps);
            return true;
        }
    }
    fclose(proc_maps);
    return false;
}

SharedObject::SharedObject(const char *_file, bool _factory)
    : file(_file), path(XORSTR("unassigned")), factory(_factory)
{
    constructed = true;
}

void SharedObject::Load()
{
    while (not LocateSharedObject(file, path))
    {
        sleep(1);
    }
    while (!(lmap = (link_map *) dlopen(path.c_str(), RTLD_NOLOAD)))
    {
        sleep(1);
        char *error = dlerror();
        if (error)
        {
            logging::Info(XORSTR("DLERROR: %s"), error);
        }
    }
    logging::Info(XORSTR("Shared object %s loaded at 0x%08x"), basename(lmap->l_name),
                  lmap->l_addr);
    if (factory)
    {
        fptr = reinterpret_cast<fn_CreateInterface_t>(
            dlsym(lmap, XORSTR("CreateInterface")));
        if (!this->fptr)
        {
            logging::Info(XORSTR("Failed to create interface factory for %s"),
                          basename(lmap->l_name));
        }
    }
}

char *SharedObject::Pointer(uintptr_t offset) const
{
    if (this->lmap != nullptr)
    {
        return reinterpret_cast<char *>(uintptr_t(lmap->l_addr) + offset);
    }
    else
    {
        return nullptr;
    }
}

void *SharedObject::CreateInterface(const std::string &interface)
{
    return (void *) (fptr(interface.c_str(), nullptr));
}

void LoadAllSharedObjects()
{
    try
    {
        steamclient().Load();
        client().Load();
        engine().Load();
        vstdlib().Load();
        tier0().Load();
        inputsystem().Load();
        materialsystem().Load();
#if ENABLE_VISUALS
        vguimatsurface().Load();
        vgui2().Load();
        studiorender().Load();
        libsdl().Load();
#endif
    }
    catch (std::exception &ex)
    {
        logging::Info(XORSTR("Exception: %s"), ex.what());
    }
}

SharedObject &steamclient()
{
    static SharedObject obj(XORSTR("steamclient.so"), true);
    return obj;
}
SharedObject &client()
{
    static SharedObject obj(XORSTR("client.so"), true);
    return obj;
}
SharedObject &engine()
{
    static SharedObject obj(XORSTR("engine.so"), true);
    return obj;
}
SharedObject &vstdlib()
{
    static SharedObject obj(XORSTR("libvstdlib.so"), true);
    return obj;
}
SharedObject &tier0()
{
    static SharedObject obj(XORSTR("libtier0.so"), false);
    return obj;
}
SharedObject &inputsystem()
{
    static SharedObject obj(XORSTR("inputsystem.so"), true);
    return obj;
}
SharedObject &materialsystem()
{
    static SharedObject obj(XORSTR("materialsystem.so"), true);
    return obj;
}
#if ENABLE_VISUALS
SharedObject &vguimatsurface()
{
    static SharedObject obj(XORSTR("vguimatsurface.so"), true);
    return obj;
}
SharedObject &vgui2()
{
    static SharedObject obj(XORSTR("vgui2.so"), true);
    return obj;
}
SharedObject &studiorender()
{
    static SharedObject obj(XORSTR("studiorender.so"), true);
    return obj;
}
SharedObject &libsdl()
{
    static SharedObject obj(XORSTR("libSDL2-2.0.so.0"), false);
    return obj;
}
#endif
}
