/************************************************************************************

PublicHeader:   OVR.h
Filename    :   OVR_Types.h
Content     :   Standard library defines and simple types
Created     :   September 19, 2012
Notes       : 

Copyright   :   Copyright 2012 Oculus VR, Inc. All Rights reserved.

Use of this software is subject to the terms of the Oculus license
agreement provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

************************************************************************************/

#ifndef OVR_Types_H
#define OVR_Types_H

//-----------------------------------------------------------------------------------
// ****** Operating System
//
// Type definitions exist for the following operating systems: (OVR_OS_x)
//
//    WIN32    - Win32 (Windows 95/98/ME and Windows NT/2000/XP)
//    DARWIN   - Darwin OS (Mac OS X)
//    LINUX    - Linux
//    ANDROID  - Android
//    IPHONE   - iPhone

#if (defined(__APPLE__) && (defined(__GNUC__) ||\
     defined(__xlC__) || defined(__xlc__))) || defined(__MACOS__)
#  if (defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__) || defined(__IPHONE_OS_VERSION_MIN_REQUIRED))
#    define OVR_OS_IPHONE
#  else
#    define OVR_OS_DARWIN
#    define OVR_OS_MAC
#  endif
#elif (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
#  define OVR_OS_WIN32
#elif (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#  define OVR_OS_WIN32
#elif defined(__linux__) || defined(__linux)
#  define OVR_OS_LINUX
#else
#  define OVR_OS_OTHER
#endif

#if defined(ANDROID)
#  define OVR_OS_ANDROID
#endif


//-----------------------------------------------------------------------------------
// ***** CPU Architecture
//
// The following CPUs are defined: (OVR_CPU_x)
//
//    X86        - x86 (IA-32)
//    X86_64     - x86_64 (amd64)
//    PPC        - PowerPC
//    PPC64      - PowerPC64
//    MIPS       - MIPS
//    OTHER      - CPU for which no special support is present or needed


#if defined(__x86_64__) || defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#  define OVR_CPU_X86_64
#  define OVR_64BIT_POINTERS
#elif defined(__i386__) || defined(OVR_OS_WIN32)
#  define OVR_CPU_X86
#elif defined(__powerpc64__)
#  define OVR_CPU_PPC64
#elif defined(__ppc__)
#  define OVR_CPU_PPC
#elif defined(__mips__) || defined(__MIPSEL__)
#  define OVR_CPU_MIPS
#elif defined(__arm__)
#  define OVR_CPU_ARM
#else
#  define OVR_CPU_OTHER
#endif

//-----------------------------------------------------------------------------------
// ***** Co-Processor Architecture
//
// The following co-processors are defined: (OVR_CPU_x)
//
//    SSE        - Available on all modern x86 processors.
//    Altivec    - Available on all modern ppc processors.
//    Neon       - Available on some armv7+ processors.

#if defined(__SSE__) || defined(OVR_OS_WIN32)
#  define  OVR_CPU_SSE
#endif // __SSE__

#if defined( __ALTIVEC__ )
#  define OVR_CPU_ALTIVEC
#endif // __ALTIVEC__

#if defined(__ARM_NEON__)
#  define OVR_CPU_ARM_NEON
#endif // __ARM_NEON__


//-----------------------------------------------------------------------------------
// ***** Compiler
//
//  The following compilers are defined: (OVR_CC_x)
//
//     MSVC     - Microsoft Visual C/C++
//     INTEL    - Intel C++ for Linux / Windows
//     GNU      - GNU C++
//     ARM      - ARM C/C++

#if defined(__INTEL_COMPILER)
// Intel 4.0                    = 400
// Intel 5.0                    = 500
// Intel 6.0                    = 600
// Intel 8.0                    = 800
// Intel 9.0                    = 900
#  define OVR_CC_INTEL       __INTEL_COMPILER

#elif defined(_MSC_VER)
// MSVC 5.0                     = 1100
// MSVC 6.0                     = 1200
// MSVC 7.0 (VC2002)            = 1300
// MSVC 7.1 (VC2003)            = 1310
// MSVC 8.0 (VC2005)            = 1400
// MSVC 9.0 (VC2008)            = 1500
// MSVC 10.0 (VC2010)           = 1600
#  define OVR_CC_MSVC        _MSC_VER

#elif defined(__GNUC__)
#  define OVR_CC_GNU

#elif defined(__CC_ARM)
#  define OVR_CC_ARM

#else
#  error "Oculus does not support this Compiler"
#endif


//-----------------------------------------------------------------------------------
// ***** Compiler Warnings

// Disable MSVC warnings
#if defined(OVR_CC_MSVC)
#  pragma warning(disable : 4127)    // Inconsistent dll linkage
#  pragma warning(disable : 4530)    // Exception handling
#  if (OVR_CC_MSVC<1300)
#    pragma warning(disable : 4514)  // Unreferenced inline function has been removed
#    pragma warning(disable : 4710)  // Function not inlined
#    pragma warning(disable : 4714)  // _force_inline not inlined
#    pragma warning(disable : 4786)  // Debug variable name longer than 255 chars
#  endif // (OVR_CC_MSVC<1300)
#endif // (OVR_CC_MSVC)



// *** Linux Unicode - must come before Standard Includes

#ifdef OVR_OS_LINUX
// Use glibc unicode functions on linux.
#  ifndef  _GNU_SOURCE
#    define _GNU_SOURCE
#  endif
#endif

//-----------------------------------------------------------------------------------
// ***** Standard Includes
//
#include    <stddef.h>
#include    <limits.h>
#include    <float.h>


// MSVC Based Memory Leak checking - for now
#if defined(OVR_CC_MSVC) && defined(OVR_BUILD_DEBUG)
#  define _CRTDBG_MAP_ALLOC
#  include <stdlib.h>
#  include <crtdbg.h>

// Uncomment this to help debug memory leaks under Visual Studio in OVR apps only.
// This shouldn't be defined in customer releases.
#  ifndef OVR_DEFINE_NEW
#    define OVR_DEFINE_NEW new(__FILE__, __LINE__)
#    define new OVR_DEFINE_NEW
#  endif

#endif


//-----------------------------------------------------------------------------------
// ***** Type definitions for Common Systems

namespace OVR {

typedef char            Char;

// Pointer-sized integer
typedef size_t          UPInt;
typedef ptrdiff_t       SPInt;


#if defined(OVR_OS_WIN32)

typedef char            SByte;  // 8 bit Integer (Byte)
typedef unsigned char   UByte;
typedef short           SInt16; // 16 bit Integer (Word)
typedef unsigned short  UInt16;
typedef long            SInt32; // 32 bit Integer
typedef unsigned long   UInt32;
typedef __int64         SInt64; // 64 bit Integer (QWord)
typedef unsigned __int64 UInt64;

 
#elif defined(OVR_OS_MAC) || defined(OVR_OS_IPHONE) || defined(OVR_CC_GNU)

typedef int             SByte  __attribute__((__mode__ (__QI__)));
typedef unsigned int    UByte  __attribute__((__mode__ (__QI__)));
typedef int             SInt16 __attribute__((__mode__ (__HI__)));
typedef unsigned int    UInt16 __attribute__((__mode__ (__HI__)));
typedef int             SInt32 __attribute__((__mode__ (__SI__)));
typedef unsigned int    UInt32 __attribute__((__mode__ (__SI__)));
typedef int             SInt64 __attribute__((__mode__ (__DI__)));
typedef unsigned int    UInt64 __attribute__((__mode__ (__DI__)));

#else

#include <sys/types.h>
typedef int8_t          SByte;
typedef uint8_t         UByte;
typedef int16_t         SInt16;
typedef uint16_t        UInt16;
typedef int32_t         SInt32;
typedef uint32_t        UInt32;
typedef int64_t         SInt64;
typedef uint64_t        UInt64;

#endif


// ***** BaseTypes Namespace

// BaseTypes namespace is explicitly declared to allow base types to be used
// by customers directly without other contents of OVR namespace.
//
// Its is expected that GFx samples will declare 'using namespace OVR::BaseTypes'
// to allow using these directly without polluting the target scope with other
// OVR declarations, such as Ptr<>, String or Mutex.
namespace BaseTypes
{
    using OVR::UPInt;
    using OVR::SPInt;
    using OVR::UByte;
    using OVR::SByte;
    using OVR::UInt16;
    using OVR::SInt16;
    using OVR::UInt32;
    using OVR::SInt32;
    using OVR::UInt64;
    using OVR::SInt64;
} // OVR::BaseTypes

} // OVR


//-----------------------------------------------------------------------------------
// ***** Macro Definitions
//
// We define the following:
//
//  OVR_BYTE_ORDER      - Defined to either OVR_LITTLE_ENDIAN or OVR_BIG_ENDIAN
//  OVR_FORCE_INLINE    - Forces inline expansion of function
//  OVR_ASM             - Assembly language prefix
//  OVR_STR             - Prefixes string with L"" if building unicode
// 
//  OVR_STDCALL         - Use stdcall calling convention (Pascal arg order)
//  OVR_CDECL           - Use cdecl calling convention (C argument order)
//  OVR_FASTCALL        - Use fastcall calling convention (registers)
//

// Byte order constants, OVR_BYTE_ORDER is defined to be one of these.
#define OVR_LITTLE_ENDIAN       1
#define OVR_BIG_ENDIAN          2


// Force inline substitute - goes before function declaration
#if defined(OVR_CC_MSVC)
#  define OVR_FORCE_INLINE  __forceinline
#elif defined(OVR_CC_GNU)
#  define OVR_FORCE_INLINE  __attribute__((always_inline)) inline
#else
#  define OVR_FORCE_INLINE  inline
#endif  // OVR_CC_MSVC


#if defined(OVR_OS_WIN32)
    
    // ***** Win32

    // Byte order
    #define OVR_BYTE_ORDER    OVR_LITTLE_ENDIAN

    // Calling convention - goes after function return type but before function name
    #ifdef __cplusplus_cli
    #  define OVR_FASTCALL      __stdcall
    #else
    #  define OVR_FASTCALL      __fastcall
    #endif

    #define OVR_STDCALL         __stdcall
    #define OVR_CDECL           __cdecl


    // Assembly macros
    #if defined(OVR_CC_MSVC)
    #  define OVR_ASM           _asm
    #else
    #  define OVR_ASM           asm
    #endif // (OVR_CC_MSVC)

    #ifdef UNICODE
    #  define OVR_STR(str)      L##str
    #else
    #  define OVR_STR(str)      str
    #endif // UNICODE

#else

    // **** Standard systems

    #if (defined(BYTE_ORDER) && (BYTE_ORDER == BIG_ENDIAN))|| \
        (defined(_BYTE_ORDER) && (_BYTE_ORDER == _BIG_ENDIAN))
    #  define OVR_BYTE_ORDER    OVR_BIG_ENDIAN
    #elif (defined(__ARMEB__) || defined(OVR_CPU_PPC) || defined(OVR_CPU_PPC64))
    #  define OVR_BYTE_ORDER    OVR_BIG_ENDIAN
    #else
    #  define OVR_BYTE_ORDER    OVR_LITTLE_ENDIAN
    #endif
    
    // Assembly macros
    #define OVR_ASM                  __asm__
    #define OVR_ASM_PROC(procname)   OVR_ASM
    #define OVR_ASM_END              OVR_ASM
    
    // Calling convention - goes after function return type but before function name
    #define OVR_FASTCALL
    #define OVR_STDCALL
    #define OVR_CDECL

#endif // defined(OVR_OS_WIN32)



//-----------------------------------------------------------------------------------
// ***** OVR_DEBUG_BREAK, OVR_ASSERT
//
// If not in debug build, macros do nothing
#ifndef OVR_BUILD_DEBUG

#  define OVR_DEBUG_BREAK  ((void)0)
#  define OVR_ASSERT(p)    ((void)0)

#else 

// Microsoft Win32 specific debugging support
#if defined(OVR_OS_WIN32)
#  ifdef OVR_CPU_X86
#    if defined(__cplusplus_cli)
#      define OVR_DEBUG_BREAK   do { __debugbreak(); } while(0)
#    elif defined(OVR_CC_GNU)
#      define OVR_DEBUG_BREAK   do { OVR_ASM("int $3\n\t"); } while(0)
#    else
#      define OVR_DEBUG_BREAK   do { OVR_ASM int 3 } while (0)
#    endif
#  else
#    define OVR_DEBUG_BREAK     do { __debugbreak(); } while(0)
#  endif
// Unix specific debugging support
#elif defined(OVR_CPU_X86) || defined(OVR_CPU_X86_64)
#  define OVR_DEBUG_BREAK       do { OVR_ASM("int $3\n\t"); } while(0)
#else
#  define OVR_DEBUG_BREAK       do { *((int *) 0) = 1; } while(0)
#endif

// This will cause compiler breakpoint
#define OVR_ASSERT(p)           do { if (!(p))  { OVR_DEBUG_BREAK; } } while(0)

#endif // OVR_BUILD_DEBUG


// Compile-time assert; produces compiler error if condition is false
#define OVR_COMPILER_ASSERT(x)  { int zero = 0; switch(zero) {case 0: case x:;} }



//-----------------------------------------------------------------------------------
// ***** OVR_UNUSED - Unused Argument handling

// Macro to quiet compiler warnings about unused parameters/variables.
#if defined(OVR_CC_GNU)
#  define   OVR_UNUSED(a)   do {__typeof__ (&a) __attribute__ ((unused)) __tmp = &a; } while(0)
#else
#  define   OVR_UNUSED(a)   (a)
#endif

#define     OVR_UNUSED1(a1) OVR_UNUSED(a1)
#define     OVR_UNUSED2(a1,a2) OVR_UNUSED(a1); OVR_UNUSED(a2)
#define     OVR_UNUSED3(a1,a2,a3) OVR_UNUSED2(a1,a2); OVR_UNUSED(a3)
#define     OVR_UNUSED4(a1,a2,a3,a4) OVR_UNUSED3(a1,a2,a3); OVR_UNUSED(a4)
#define     OVR_UNUSED5(a1,a2,a3,a4,a5) OVR_UNUSED4(a1,a2,a3,a4); OVR_UNUSED(a5)
#define     OVR_UNUSED6(a1,a2,a3,a4,a5,a6) OVR_UNUSED4(a1,a2,a3,a4); OVR_UNUSED2(a5,a6)
#define     OVR_UNUSED7(a1,a2,a3,a4,a5,a6,a7) OVR_UNUSED4(a1,a2,a3,a4); OVR_UNUSED3(a5,a6,a7)
#define     OVR_UNUSED8(a1,a2,a3,a4,a5,a6,a7,a8) OVR_UNUSED4(a1,a2,a3,a4); OVR_UNUSED4(a5,a6,a7,a8)
#define     OVR_UNUSED9(a1,a2,a3,a4,a5,a6,a7,a8,a9) OVR_UNUSED4(a1,a2,a3,a4); OVR_UNUSED5(a5,a6,a7,a8,a9)


//-----------------------------------------------------------------------------------
// ***** Configuration Macros

// SF Build type
#ifdef OVR_BUILD_DEBUG
#  define OVR_BUILD_STRING  "Debug"
#else
#  define OVR_BUILD_STRING  "Release"
#endif


//// Enables SF Debugging information
//# define OVR_BUILD_DEBUG

// OVR_DEBUG_STATEMENT injects a statement only in debug builds.
// OVR_DEBUG_SELECT injects first argument in debug builds, second argument otherwise.
#ifdef OVR_BUILD_DEBUG
#define OVR_DEBUG_STATEMENT(s)   s
#define OVR_DEBUG_SELECT(d, nd)  d
#else
#define OVR_DEBUG_STATEMENT(s)
#define OVR_DEBUG_SELECT(d, nd)  nd
#endif


#define OVR_ENABLE_THREADS
//
// Prevents OVR from defining new within
// type macros, so developers can override
// new using the #define new new(...) trick
// - used with OVR_DEFINE_NEW macro
//# define OVR_BUILD_DEFINE_NEW
//


#endif  // OVR_Types_h
