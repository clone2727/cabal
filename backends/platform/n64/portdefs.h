/* Cabal - Legacy Game Implementations
 *
 * Cabal is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// Based on the ScummVM (GPLv2+) file of the same name

#ifndef __N64_PORTDEFS__
#define __N64_PORTDEFS__

#include <n64utils.h>

#include <sys/types.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <new>

#undef assert
#define assert(x)  ((x) ? 0 : (print_error("ASSERT TRIGGERED:\n\n("#x")\n%s\nline: %d", __FILE__, __LINE__)))

// Typedef basic data types in a way that is compatible with the N64 SDK.
typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short int uint16_t;
typedef signed short int int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;

// Define CABAL_DONT_DEFINE_STDTYPES to prevent scummsys.h from trying to
// re-define those data types.
#define CABAL_DONT_DEFINE_STDTYPES
#define CABAL_DONT_DEFINE_UINT

#endif
