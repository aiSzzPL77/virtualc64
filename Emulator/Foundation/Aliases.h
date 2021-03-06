// -----------------------------------------------------------------------------
// This file is part of VirtualC64
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v2
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#ifndef _ALIASES_H
#define _ALIASES_H


//
// Basic types
//

typedef char               i8;
typedef short              i16;
typedef int                i32;
typedef long long          i64;
typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

static_assert(sizeof(i8) == 1,  "i8 size mismatch");
static_assert(sizeof(i16) == 2, "i16 size mismatch");
static_assert(sizeof(i32) == 4, "i32 size mismatch");
static_assert(sizeof(i64) == 8, "i64 size mismatch");
static_assert(sizeof(u8) == 1,  "u8 size mismatch");
static_assert(sizeof(u16) == 2, "u16 size mismatch");
static_assert(sizeof(u32) == 4, "u32 size mismatch");
static_assert(sizeof(u64) == 8, "u64 size mismatch");


//
// Cycle counts
//

typedef i64 Cycle;


//
// Enumeration types
//

/* All enumeration types are declared via a special 'longenum' macro to make
 * them easily accessible in Swift.
 */

// Definition for wasm (long = 32 Bit)
#if defined(__EMSCRIPTEN__)
#define longenum(_name) \
typedef enum __attribute__((enum_extensibility(open))) _name : long long _name; \
enum _name : long long

// Definition for clang (long = 64 Bit)
#elif defined(__clang__)
#define longenum(_name) \
typedef enum __attribute__((enum_extensibility(open))) _name : long _name; \
enum _name : long

// Definition for gcc (long = 64 Bit)
#else
#define longenum(_name) \
enum _name : long
#endif

#endif
