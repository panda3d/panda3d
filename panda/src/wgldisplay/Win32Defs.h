
// This file exists so OGL can compile with original VC6 headers 
// without latest MS Platform SDK.  It simply reproduces all the
// necessary defs here

#ifndef DECLSPEC_SELECTANY
#define DECLSPEC_SELECTANY  __declspec(selectany)
#endif

#ifndef SetClassLongPtr
#define SetClassLongPtr SetClassLong
#endif

#ifndef GCLP_HCURSOR
#define GCLP_HCURSOR GCL_HCURSOR
#endif

#ifndef SPI_GETMOUSEVANISH
// from WinXP winuser.h
#define SPI_GETMOUSEVANISH                  0x1020
#define SPI_SETMOUSEVANISH                  0x1021
#endif

#ifndef SPI_GETCURSORSHADOW
#define SPI_GETCURSORSHADOW                 0x101A
#define SPI_SETCURSORSHADOW                 0x101B
#endif

#ifndef __int3264 
#define LONG_PTR LONG
#endif


