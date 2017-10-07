/*
 * logging.h
 *
 *  Created on: Oct 3, 2016
 *      Author: nullifiedcat
 */

#pragma once

#include <stdio.h>

#include "stringhelpers.hpp" 	// strfmt(), format() for when you might want to include this

// Use this to log to a file
void CatLogging(const char* fmt, ...);

