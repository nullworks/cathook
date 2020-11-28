/*
  Created on 01.07.18.
*/
#pragma once

#include "config.h"

#if ENABLE_VISUALS

#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_system.h>
#include <SDL2/SDL_mouse.h>
#include "Settings.hpp"

namespace settings
{

struct Key
{
    int mouse{ 0 };
    int keycode{ 0 };
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
    void fromString(const std::string &string, bool init = false) override
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
            key.keycode = SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(std::strtol(string.c_str() + 4, nullptr, 10)));
        }
        else
        {
            auto code = SDL_GetKeyFromName(string.c_str());
            if (code != SDLK_UNKNOWN)
                key.keycode = code;
        }

        setInternal(key);
        // This is broken otherwise
        if (init)
            this->string = string;
    }

    // Variable & causes segfault with gcc optimizations + these dont even
    // return anything
    void operator=(const std::string &string)
    {
        fromString(string);
    }

    void reset()
    {
        setInternal(Key{});
    }

    void key(SDL_Keycode key)
    {
        Key k{};
        k.keycode = key;
        setInternal(k);
    }

    void mouse(int mouse_key)
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
        return value.mouse || value.keycode;
    }

    inline bool isKeyDown() const
    {
        if (value.mouse)
        {
            auto flag = SDL_GetMouseState(nullptr, nullptr);
            if (flag & SDL_BUTTON(value.mouse))
                return true;
        }
        else if (value.keycode)
        {
            auto keys = SDL_GetKeyboardState(nullptr);
            if (keys[SDL_GetScancodeFromKey(value.keycode)])
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
            if (SDL_GetScancodeFromKey(next.keycode) == static_cast<SDL_Scancode>(0))
                string = "<null>";
            else
            {
                const char *s = SDL_GetKeyName(next.keycode);
                if (!s || *s == 0)
                    string = "Key " + std::to_string(SDL_GetScancodeFromKey(next.keycode));
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

#else

#include "Settings.hpp"

namespace settings
{

struct Key
{
    int mouse{ 0 };
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
    void fromString(const std::string &string, bool = false) override
    {
    }
    // Variable & causes segfault with gcc optimizations + these dont even
    // return anything
    void operator=(const std::string &string)
    {
        fromString(string);
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
        return false;
    }

    inline bool isKeyDown() const
    {
        return false;
    }

protected:
    void setInternal(Key next)
    {
        fireCallbacks(next);
    }
    Key value{};
    std::string string{};
};
} // namespace settings

#endif
