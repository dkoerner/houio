#pragma once

#ifndef _MSC_VER
#include <stdint.h>
#endif

namespace houio
{

#if (defined _MSC_VER)
typedef signed   char          sbyte;
typedef signed   char           int8;
typedef unsigned char          ubyte;
typedef unsigned char          uint8;
typedef signed   short int     sword;
typedef unsigned short int     uword;
typedef signed   short int    sint16;
typedef unsigned short int    uint16;
typedef signed __int32        sint32;
typedef unsigned __int32      uint32;
typedef signed   long  int    sdword;
typedef unsigned long  int    udword;
typedef signed __int64        sint64;
typedef unsigned __int64      uint64;
typedef float                 real32;
typedef double                real64;

#else
typedef uint64_t   uint64;
typedef int64_t    sint64;
typedef uint32_t   uint32;
typedef int32_t    sint32;
typedef float      real32;
typedef double     real64;

typedef int8_t     sbyte;
typedef uint8_t    ubyte;
typedef int16_t    sword;
typedef uint16_t   uword;
typedef int32_t    sdword;
typedef uint32_t   udword;
#endif

}
