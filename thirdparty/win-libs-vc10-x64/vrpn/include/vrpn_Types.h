#ifndef VRPN_TYPES_H
#define VRPN_TYPES_H

#include  "vrpn_Configure.h"

//------------------------------------------------------------------
// Do a test for a C++ compiler first, to ensure it's the first
// error message.  Otherwise, the error messages you get are
// completely cryptic.
//------------------------------------------------------------------
#ifndef __cplusplus
#ifndef	VRPN_IGNORE_NO_CPLUSPLUS
#error Need to compile with a C++ compiler, not a C compiler.  The problem is that in Windows, filenames are case-insensitive.  So the compiler cannot tell mumble.c from mumble.C.  Visual Studio decided to make .cpp (which used to mean run the C preprocessor) mean C++ and both .c and .C mean C.  The other problem is that when you insert a new file into a project, it FOR THAT FILE makes an override.  The project settings say C++ but if you right-click on the file itself it has an override to compile with C.  This needs to be changed for both the .C file and the .h file.
#endif
#endif

//------------------------------------------------------------------
//   This section contains definitions for architecture-dependent
// types.  It is important that the data sent over a vrpn_Connection
// be of the same size on all hosts sending and receiving it.  Since
// C++ does not constrain the size of 'int', 'long', 'double' and
// so forth, we create new types here that are defined correctly for
// each architecture and use them for all data that might be sent
// across a connection.
//   Part of porting VRPN to a new architecture is defining the
// types below on that architecture in such as way that the compiler
// can determine which machine type it is on.
//------------------------------------------------------------------

#undef VRPN_ARCH

#ifdef  sgi
#define  VRPN_ARCH  sgi
typedef  char            vrpn_int8;
typedef  unsigned char   vrpn_uint8;
typedef  short           vrpn_int16;
typedef  unsigned short  vrpn_uint16;
typedef  int             vrpn_int32;
typedef  unsigned int    vrpn_uint32;
typedef  float           vrpn_float32;
typedef  double          vrpn_float64;
#endif

#ifdef  hpux
#define  VRPN_ARCH  hpux
typedef  char            vrpn_int8;
typedef  unsigned char   vrpn_uint8;
typedef  short           vrpn_int16;
typedef  unsigned short  vrpn_uint16;
typedef  int             vrpn_int32;
typedef  unsigned int    vrpn_uint32;
typedef  float           vrpn_float32;
typedef  double          vrpn_float64;
#endif

// For PixelFlow aCC compiler
#ifdef  __hpux
#undef   VRPN_ARCH
#define  VRPN_ARCH  __hpux
typedef  char            vrpn_int8;
typedef  unsigned char   vrpn_uint8;
typedef  short           vrpn_int16;
typedef  unsigned short  vrpn_uint16;
typedef  int             vrpn_int32;
typedef  unsigned int    vrpn_uint32;
typedef  float           vrpn_float32;
typedef  double          vrpn_float64;
#endif

#ifdef  sparc
#define  VRPN_ARCH  sparc
typedef  char            vrpn_int8;
typedef  unsigned char   vrpn_uint8;
typedef  short           vrpn_int16;
typedef  unsigned short  vrpn_uint16;
typedef  int             vrpn_int32;
typedef  unsigned int    vrpn_uint32;
typedef  float           vrpn_float32;
typedef  double          vrpn_float64;
#endif


#ifdef  linux
#define  VRPN_ARCH  linux
typedef  char            vrpn_int8;
typedef  unsigned char   vrpn_uint8;
typedef  short           vrpn_int16;
typedef  unsigned short  vrpn_uint16;
typedef  int             vrpn_int32;
typedef  unsigned int    vrpn_uint32;
typedef  float           vrpn_float32;
typedef  double          vrpn_float64;
#endif

#ifdef  _AIX
#define  VRPN_ARCH  aix
typedef  char            vrpn_int8;
typedef  unsigned char   vrpn_uint8;
typedef  short           vrpn_int16;
typedef  unsigned short  vrpn_uint16;
typedef  int             vrpn_int32;
typedef  unsigned int    vrpn_uint32;
typedef  float           vrpn_float32;
typedef  double          vrpn_float64;
#endif

// _WIN32 is defined for all compilers for Windows (cygnus g++ included)
// furthermore, __CYGNUS__ is defined by g++ but not by VC++
// currently, we use __CYGNUS__ to differentiate between VC++ and g++.
//
// XXX [juliano 10/9/99] now that cygnus gcc and GNU gcc have been merged
// back into a single compiler (starting with gcc-2.95), we should probably
// start to use __GNUC__, since that's the official GNU C macro. XXX
//
// WIN32 (sans underline) is defined only by the Windows VC++ compiler.
//
//     DO NOT EVER USE WIN32
//
// It is too hard to differentiate from _WIN32, and may not actually be
// defined by VC++ (it's a project option).  If you use WIN32 to distinguish
// between VC++ and cygwin/g++, may your wrists quickly develop a nerve
// disorder that prevents you from ever typing again ;)
// 
#ifdef  _WIN32
#define  VRPN_ARCH  _WIN32
typedef  char            vrpn_int8;
typedef  unsigned char   vrpn_uint8;
typedef  short           vrpn_int16;
typedef  unsigned short  vrpn_uint16;
typedef  int             vrpn_int32;
typedef  unsigned int	 vrpn_uint32;
typedef  float           vrpn_float32;
typedef  double          vrpn_float64;
#endif

#if defined(FreeBSD) || defined(__FreeBSD__)
#ifndef FreeBSD
#define FreeBSD
#endif
#define  VRPN_ARCH  FreeBSD
typedef  char            vrpn_int8;
typedef  unsigned char   vrpn_uint8;
typedef  short           vrpn_int16;
typedef  unsigned short  vrpn_uint16;
typedef  int		 vrpn_int32;
typedef  unsigned int    vrpn_uint32;
typedef  float           vrpn_float32;
typedef  double          vrpn_float64;
#endif

#ifdef __APPLE__ 
#define  VRPN_ARCH  MacOSX
typedef  char            vrpn_int8;
typedef  unsigned char   vrpn_uint8;
typedef  short           vrpn_int16;
typedef  unsigned short  vrpn_uint16;
typedef  int             vrpn_int32;
typedef  unsigned int    vrpn_uint32;
typedef  float           vrpn_float32;
typedef  double          vrpn_float64;
#endif

// Architecture of last resort.
#ifndef	VRPN_ARCH
#ifdef  __GNUC__
#define  VRPN_ARCH  _WIN32
typedef  char            vrpn_int8;
typedef  unsigned char   vrpn_uint8;
typedef  short           vrpn_int16;
typedef  unsigned short  vrpn_uint16;
typedef  int             vrpn_int32;
typedef  unsigned int    vrpn_uint32;
typedef  float           vrpn_float32;
typedef  double          vrpn_float64;
#endif
#endif


#ifndef VRPN_ARCH
#error Need to define architecture-dependent sizes in this file
#endif

// prevent use of these macros outside this file
// if you need to distinguish, then define new types in this file
//
// [juliano 10/10/99] actually, we do need to test (and do so directly)
// outside this file for functions that differ between platforms.  A much
// better solution is to create our own functions that are wrappers for
// the platform-specific ones or the platform-specific hacks.  All such
// things go in some common file (that file is the ONLY place that knows
// which platform you are actually on).  Then, you call our wrappers in
// the real code.
//
//   Localizing platform-specific stuff like this is much more robust, easy to
//   port, and makes the rest of the source code easier to understand
//
//   however, I'm not sure this would really work for us.  We'd be restriced
//   to defining the vrpn_ versions to be the greatest-common-denominator
//   of features available on all the platforms.  But aren't we really
//   restricted to that already?
//
#undef  VRPN_ARCH

// *******************************************************
// you should NOT need to modify anything below this point
// *******************************************************
#ifdef __cplusplus
//XXX     Need to compile with a C++ compiler, not a C compiler
typedef vrpn_int16 vrpn_bool;

const vrpn_int16 vrpn_true  = 1;
const vrpn_int16 vrpn_false = 0;
const vrpn_int16 vrpn_TRUE  = 1;
const vrpn_int16 vrpn_FALSE = 0;
const vrpn_int16 VRPN_TRUE  = 1;
const vrpn_int16 VRPN_FALSE = 0;
#endif

// should we add a success & fail?


// [juliano 10/9/99] The vrpn bool variables can not actually be fully
// optimized away, because the compiler is not allowed to assume their
// values don't change.
//
//   [juliano 11/28/99] Perhaps the optimization can be done if they are
//   static?  I don't know enough about what compilers can/cannot do today.
//
// If you are willing to assume templates, there is an alternative using
// a traits class that does make the optimization possible (and likely).
//
// If you don't want to use templates, but still want the sizeof
// these things be vrpn_int16, you can use macros like this.
//
//    #define vrpn_false /*false*/vrpn_int16(0)
//    #define vrpn_true  /*true*/vrpn_int16(1)
//
// With this method, you will still be able to tell, in the
// compiler error messages, what the real code contains.
// 
// If you don't care about them being a different type than
// vrpn_int16 (probably not a good idea), you can use this technique,
// which guarantees optimizations can be performed.
//
//     enum vrpn_bool_constants_t{
//         vrpn_false=0, vrpn_FALSE=0, VRPN_FALSE=0,
//         vrpn_true=1,  vrpn_TRUE=1,  VRPN_TRUE=1 };
//

#endif //VRPN_TYPES_H
