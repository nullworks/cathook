/*
 * HUD.h
 *
 *  Created on: Jun 4, 2017
 *      Author: nullifiedcat
 */

#pragma once

#include <core/logging.hpp>
#include <utlvector.h>
#include "core/vfunc.hpp"

class CHudBaseChat
{
public:
    void *vtable;
    inline void Printf(const char *string)
    {
        typedef void (*original_t)(CHudBaseChat *, int, const char *, ...);
        original_t function = vfunc<original_t>(this, 0x15);
        function(this, 0, "%s", string);
    }
};

class CHudElement
{
public:
    void *vtable;
};

class CHud
{
public:
    void *vtable;

    CHudElement *FindElement(const char *name);
    float &GetSensitivityFactor();
};
