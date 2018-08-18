/*
  Created on 01.07.18.
*/

#pragma once

#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_system.h>
#include <SDL2/SDL_mouse.h>
#include "Settings.hpp"

namespace settings
{

struct Key
{
    int mouse{ 0 };
    SDL_Scancode scan{ static_cast<SDL_Scancode>(0) };
};

template <> class Variable<Key> : public VariableBase<Key>
{
public:
    ~Variable() override = default;

    VariableType getType() override
    {
        return VariableType::KEY;
    }

    // Valid inputs: "Mouse1", "Mouse5", "Key 6", "Key 10", "Key 2", "Space".
    void fromString(const std::string &string) override
    {
        if (string == "<null>")
        {
            reset();
            return;
        }

        Key key{};
        if (string.find("Mouse") != std::string::npos)
        {
            key.mouse = std::strtol(string.c_str() + 5, nullptr, 10);
        }
        else if (string.find("Key ") != std::string::npos)
        {
            key.scan = static_cast<SDL_Scancode>(
                std::strtol(string.c_str() + 4, nullptr, 10));
        }
        else
        {
            auto code = SDL_GetKeyFromName(string.c_str());
            if (code != SDLK_UNKNOWN)
                key.scan = SDL_GetScancodeFromKey(code);
        }

        setInternal(key);
    }

    Variable &operator=(const std::string &string)
    {
        fromString(string);
    }

    Variable &reset()
    {
        setInternal(Key{});
    }

    Variable &key(SDL_Scancode key)
    {
        Key k{};
        k.scan = key;
        setInternal(k);
    }

    Variable &mouse(int mouse_key)
    {
        Key k{};
        k.mouse = mouse_key;
        setInternal(k);
    }

    inline const Key &operator*() override
    {
        return value;
    }

    inline const std::string &toString() override
    {
        return string;
    }

    inline explicit operator bool() const
    {
        return value.mouse || value.scan;
    }

    inline bool isKeyDown() const
    {
        if (value.mouse)
        {
            auto flag = SDL_GetMouseState(nullptr, nullptr);
            if (flag & SDL_BUTTON(value.mouse))
                return true;
        }
        else if (value.scan)
        {
            auto keys = SDL_GetKeyboardState(nullptr);
            if (keys[value.scan])
                return true;
        }
        return false;
    }

protected:
    void setInternal(Key next)
    {
        if (next.mouse)
            string = "Mouse" + std::to_string(next.mouse);
        else
        {
            if (next.scan == static_cast<SDL_Scancode>(0))
                string = "<null>";
            else
            {
                const char *s =
                    SDL_GetKeyName(SDL_GetKeyFromScancode(next.scan));
                if (!s || *s == 0)
                    string = "Key " + std::to_string(next.scan);
                else
                    string = s;
            }
        }
        value = next;
        fireCallbacks(next);
    }

    std::string string{};
    Key value{};
};
} // namespace settings