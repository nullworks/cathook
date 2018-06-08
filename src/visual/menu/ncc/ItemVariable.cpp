/*
 * ItemVariable.cpp
 *
 *  Created on: Mar 26, 2017
 *      Author: nullifiedcat
 */

#include "menu/ncc/ItemVariable.hpp"
#include "menu/ncc/Item.hpp"
#include "menu/ncc/Menu.hpp"
#include "common.hpp"

namespace menu
{
namespace ncc
{

ItemVariable::ItemVariable(CatVar &variable)
    : Item(XORSTR("ncc_item_variable_") + variable.name), catvar(variable)
{
}

void ItemVariable::Update()
{
    Item::Update();
    if (catvar.name.c_str())
        if (catvar.registered == true)
            if (catvar.desc_long.c_str())
                if (!catvar.desc_long.empty())
                    if (catvar.desc_long.length() && IsHovered() &&
                        catvar.desc_long != XORSTR("no description"))
                        ShowTooltip(catvar.desc_long);
}

void ItemVariable::Change(float amount)
{
    if (!amount)
        return;
    switch (catvar.type)
    {
    case CV_SWITCH:
    {
        if (catvar.desc_long != XORSTR("INVALID COMMAND"))
            catvar = !catvar;
    }
    break;
    case CV_ENUM:
    case CV_INT:
    {
        catvar = (int) catvar + (int) amount;
        if (catvar.restricted && (int) catvar > catvar.max)
            catvar = (int) catvar.max;
        if (catvar.restricted && (int) catvar < catvar.min)
            catvar = (int) catvar.min;
    }
    break;
    case CV_FLOAT:
    {
        catvar = (float) catvar + amount;
        if (catvar.restricted && (float) catvar > catvar.max)
            catvar = (float) catvar.max;
        if (catvar.restricted && (float) catvar < catvar.min)
            catvar = (float) catvar.min;
    }
    break;
    }
}

bool ItemVariable::ConsumesKey(ButtonCode_t key)
{
    if (capturing)
        return true;
    if (key == ButtonCode_t::MOUSE_WHEEL_DOWN ||
        key == ButtonCode_t::MOUSE_WHEEL_UP ||
        key >= ButtonCode_t::KEY_FIRST && key <= ButtonCode_t::KEY_BACKSPACE)
        return true;
    return false;
}

void ItemVariable::OnMousePress()
{
    if (catvar.type == CV_KEY)
    {
        if (capturing)
        {
            catvar    = ButtonCode_t::MOUSE_LEFT;
            capturing = false;
        }
        else
            capturing = true;
    }
    if (catvar.type == CV_SWITCH)
        if (catvar.desc_long != XORSTR("INVALID COMMAND"))
            catvar = !catvar;
}

void ItemVariable::OnFocusLose()
{
    capturing = false;
}

void ItemVariable::PutChar(char ch)
{
    catvar.SetValue(catvar.GetString() + std::string(1, ch));
}

void ItemVariable::OnKeyPress(ButtonCode_t key, bool repeat)
{
    if (capturing)
    {
        if (key == ButtonCode_t::KEY_ESCAPE)
            key   = (ButtonCode_t) 0;
        catvar    = (int) key;
        capturing = false;
        return;
    }

    float change = 1.0f;

    switch (catvar.type)
    {
    case CV_STRING:
    {
        if (key == ButtonCode_t::KEY_BACKSPACE)
        {
            std::string val = catvar.GetString();
            if (val.length() > 0)
                catvar.SetValue(val.substr(0, val.length() - 1));
            return;
        }
        else if (key == ButtonCode_t::KEY_SPACE)
        {
            PutChar(' ');
            return;
        }
        else
        {
            char ch = 0;
            if (g_IInputSystem->IsButtonDown(ButtonCode_t::KEY_LSHIFT) ||
                g_IInputSystem->IsButtonDown(ButtonCode_t::KEY_RSHIFT))
                ch = GetUpperChar(key);
            else
                ch = GetChar(key);
            if (ch)
                PutChar(ch);
        }
    }
    break;
    case CV_FLOAT:
    {
        if (catvar.restricted)
            change = float(catvar.max - catvar.min) / 50.0f;
        else
            change = 1.0f;
    }
    break;
    }

    if (change < 1.0f && catvar.type == CV_INT)
        change = 1.0f;

    if (key == ButtonCode_t::MOUSE_WHEEL_UP)
    {
        if ((catvar.type == CV_FLOAT || catvar.type == CV_INT) &&
            g_IInputSystem->IsButtonDown(ButtonCode_t::KEY_LSHIFT))
            Change(change * 2);
        else if ((catvar.type == CV_FLOAT || catvar.type == CV_INT) &&
                 g_IInputSystem->IsButtonDown(ButtonCode_t::KEY_LCONTROL))
            Change(change / 4);
        else
            Change(change);
    }
    else if (key == ButtonCode_t::MOUSE_WHEEL_DOWN)
    {
        if (catvar.type == CV_FLOAT &&
            g_IInputSystem->IsButtonDown(ButtonCode_t::KEY_LSHIFT))
            Change(-change * 2);
        else if (catvar.type == CV_FLOAT &&
                 g_IInputSystem->IsButtonDown(ButtonCode_t::KEY_LCONTROL))
            Change(-change / 4);
        else
            Change(-change);
    }
}

void ItemVariable::Draw(int x, int y)
{
    Item::Draw(x, y);
    std::string val = XORSTR("[UNDEFINED]");
    switch (catvar.type)
    {
    case CV_SWITCH:
    {
        if (catvar.desc_long != XORSTR("INVALID COMMAND"))
            val = catvar ? XORSTR("ON") : XORSTR("OFF");
    }
    break;
    case CV_INT:
    {
        val = std::to_string((int) catvar);
    }
    break;
    case CV_FLOAT:
    {
        val = std::to_string((float) catvar);
    }
    break;
    case CV_ENUM:
    {
        val = catvar.enum_type->value_names[(int) catvar];
    }
    break;
    case CV_STRING:
    {
        val = std::string(catvar.GetString());
    }
    break;
    case CV_KEY:
    {
        if (capturing)
        {
            val = XORSTR("[PRESS A KEY]");
        }
        else
        {
            if (catvar)
            {
                const char *name = g_IInputSystem->ButtonCodeToString(
                    (ButtonCode_t)((int) catvar));
                if (name)
                {
                    val = std::string(name);
                }
            }
            else
            {
                val = XORSTR("[CLICK TO SET]");
            }
        }
    }
    break;
    }
    draw::String(menu::ncc::font_item, x + 2, y, colorsint::white, 2,
                 format(catvar.desc_short, XORSTR(": "), val));
}
}
}
