#pragma once

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
typedef unsigned long long    uint64;
typedef signed long long      sint64;
typedef unsigned int          uint32;
typedef float                 real32;
typedef double                real64;

typedef signed   char          sbyte;
typedef unsigned char          ubyte;
typedef signed   short int     sword;
typedef unsigned short int     uword;
typedef signed   long  int    sdword;
typedef unsigned long  int    udword;
#endif

}
