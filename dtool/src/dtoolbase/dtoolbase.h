/* Filename: dtoolbase.h
 * Created by:  drose (12Sep00)
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* This file is included at the beginning of every header file and/or
   C or C++ file.  It must be compilable for C as well as C++ files,
   so no C++-specific code or syntax can be put here.  See
   dtoolbase_cc.h for C++-specific stuff. */

#ifndef DTOOLBASE_H
#define DTOOLBASE_H

#include "dtool_config.h"

/* Make sure WIN32 and WIN32_VC are defined when using MSVC */
#ifdef _WIN32
#ifndef WIN32
#define WIN32
#endif
#ifdef _MSC_VER
#ifndef WIN32_VC
#define WIN32_VC
#endif
#endif
#endif

#ifdef WIN32_VC
/* These warning pragmas must appear before anything else for VC++ to
   respect them.  Sheesh. */

/* C4231: extern before template instantiation */
/* For some reason, this particular warning won't disable. */
#pragma warning (disable : 4231)
/* C4786: 255 char debug symbols */
#pragma warning (disable : 4786)
/* C4251: needs dll interface */
#pragma warning (disable : 4251)
/* C4503: decorated name length exceeded */
#pragma warning (disable : 4503)
/* C4305: truncation from 'const double' to 'float' */
#pragma warning (disable : 4305)
/* C4250: 'myclass' : inherits 'baseclass::member' via dominance */
#pragma warning (disable : 4250)
/* C4355: 'this' : used in base member initializer list */
#pragma warning (disable : 4355)
/* C4244: 'initializing' : conversion from 'double' to 'float', possible loss of data */
#pragma warning (disable : 4244)

#if _MSC_VER >= 1300
 #if _MSC_VER >= 1310
   #define USING_MSVC7_1
//#pragma message("VC 7.1")    
 #else
//#pragma message("VC 7.0") 
 #endif
#define USING_MSVC7
#else 
// #pragma message("VC 6.0")
#endif

// Use NODEFAULT to optimize a switch() stmt to tell MSVC to automatically go to the final untested case 
// after it has failed all the other cases (i.e. 'assume at least one of the cases is always true')
#ifdef _DEBUG
# define NODEFAULT  default: assert(0);
#else
# define NODEFAULT  default: __assume(0);   // special VC keyword
#endif

#else /* if !WIN32_VC */
#ifdef _DEBUG
# define NODEFAULT   default: assert(0);
#else
# define NODEFAULT
#endif
#endif  /* WIN32_VC */

/*
  include win32 defns for everything up to WinServer2003, and assume
  I'm smart enough to use GetProcAddress for backward compat on
  w95/w98 for newer fns
*/
#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif
#define _WIN32_WINNT 0x0502

#include "dtoolsymbols.h"

// always include assert.h until drose unbreaks it for opt4
#include <assert.h>

#ifdef __GNUC__
// Large file >2GB support
// this needs be be before systypes.h and other C headers
#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE 1
#endif

#ifdef HAVE_TYPES_H
#include <types.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_MALLOC_H
#include <malloc.h>
#endif

#ifdef HAVE_SYS_MALLOC_H
#include <sys/malloc.h>
#endif

#ifdef HAVE_ALLOCA_H
#include <alloca.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_IO_H
#include <io.h>
#endif

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#ifdef HAVE_MINMAX_H
#include <minmax.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef CPPPARSER
#include <stdtypedefs.h>
#endif

#ifdef USE_TAU
/* If we're building with the Tau instrumentor, include the
   appropriate header file to pick up the TAU macros. */
#include <TAU.h>
#include <Profile/Profiler.h>
#else
/* Otherwise, if we're not building with the Tau instrumentor, turn
   off all the TAU macros.  We could include the Tau header file to do
   this, but it's better not to assume that Tau is installed. */
#define TAU_TYPE_STRING(profileString, str) 
#define TAU_PROFILE(name, type, group) 
#define TAU_PROFILE_TIMER(var, name, type, group)
#define TAU_PROFILE_START(var)
#define TAU_PROFILE_STOP(var)
#define TAU_PROFILE_STMT(stmt) 
#define TAU_PROFILE_EXIT(msg)
#define TAU_PROFILE_INIT(argc, argv)
#define TAU_PROFILE_SET_NODE(node)
#define TAU_PROFILE_SET_CONTEXT(context)
#define TAU_PROFILE_SET_GROUP_NAME(newname)
#define TAU_PROFILE_TIMER_SET_GROUP_NAME(t, newname)
#define TAU_PROFILE_CALLSTACK()    
#define TAU_DB_DUMP()
#define TAU_DB_PURGE()

#define TAU_REGISTER_CONTEXT_EVENT(event, name)
#define TAU_CONTEXT_EVENT(event, data)
#define TAU_DISABLE_CONTEXT_EVENT(event)
#define TAU_ENABLE_CONTEXT_EVENT(event)

#define TAU_REGISTER_EVENT(event, name)
#define TAU_EVENT(event, data)
#define TAU_EVENT_DISABLE_MIN(event)
#define TAU_EVENT_DISABLE_MAX(event)
#define TAU_EVENT_DISABLE_MEAN(event)
#define TAU_EVENT_DISABLE_STDDEV(event)
#define TAU_REPORT_STATISTICS()
#define TAU_REPORT_THREAD_STATISTICS()
#define TAU_REGISTER_THREAD()
#define TAU_REGISTER_FORK(id, op) 
#define TAU_ENABLE_INSTRUMENTATION() 		
#define TAU_DISABLE_INSTRUMENTATION() 	
#define TAU_ENABLE_GROUP(group)
#define TAU_DISABLE_GROUP(group)
#define TAU_ENABLE_GROUP_NAME(group)
#define TAU_DISABLE_GROUP_NAME(group)
#define TAU_ENABLE_ALL_GROUPS()			
#define TAU_DISABLE_ALL_GROUPS()	
#define TAU_TRACK_MEMORY()
#define TAU_TRACK_MEMORY_HERE()
#define TAU_ENABLE_TRACKING_MEMORY()
#define TAU_DISABLE_TRACKING_MEMORY()
#define TAU_TRACK_MEMORY()
#define TAU_TRACK_MEMORY_HERE()
#define TAU_ENABLE_TRACKING_MUSE_EVENTS()	
#define TAU_DISABLE_TRACKING_MUSE_EVENTS()
#define TAU_TRACK_MUSE_EVENTS()		
#define TAU_SET_INTERRUPT_INTERVAL(value)

#define TAU_TRACE_SENDMSG(type, destination, length) 
#define TAU_TRACE_RECVMSG(type, source, length)

#define TAU_MAPPING(stmt, group) stmt
#define TAU_MAPPING_OBJECT(FuncInfoVar) 
#define TAU_MAPPING_LINK(FuncInfoVar, Group) 
#define TAU_MAPPING_PROFILE(FuncInfoVar) 
#define TAU_MAPPING_CREATE(name, type, key, groupname, tid) 
#define TAU_MAPPING_PROFILE_TIMER(Timer, FuncInfoVar, tid)
#define TAU_MAPPING_TIMER_CREATE(t, name, type, gr, group_name)
#define TAU_MAPPING_PROFILE_START(Timer, tid) 
#define TAU_MAPPING_PROFILE_STOP(tid) 
#define TAU_MAPPING_PROFILE_EXIT(msg, tid)  
#define TAU_MAPPING_DB_DUMP(tid)
#define TAU_MAPPING_DB_PURGE(tid)
#define TAU_MAPPING_PROFILE_SET_NODE(node, tid)  
#define TAU_MAPPING_PROFILE_SET_GROUP_NAME(timer, name)
#define TAU_PROFILE_TIMER_SET_NAME(t, newname)
#define TAU_PROFILE_TIMER_SET_TYPE(t, newname)
#define TAU_PROFILE_TIMER_SET_GROUP(t, id) 
#define TAU_MAPPING_PROFILE_SET_NAME(timer, name)
#define TAU_MAPPING_PROFILE_SET_TYPE(timer, name)
#define TAU_MAPPING_PROFILE_SET_GROUP(timer, id)
#define TAU_MAPPING_PROFILE_GET_GROUP_NAME(timer)
#define TAU_MAPPING_PROFILE_GET_GROUP(timer)
#define TAU_MAPPING_PROFILE_GET_NAME(timer)
#define TAU_MAPPING_PROFILE_GET_TYPE(timer)

#define TAU_PHASE(name, type, group) 
#define TAU_PHASE_CREATE_STATIC(var, name, type, group) 
#define TAU_PHASE_CREATE_DYNAMIC(var, name, type, group) 
#define TAU_PHASE_START(var) 
#define TAU_PHASE_STOP(var) 
#define TAU_GLOBAL_PHASE(timer, name, type, group) 
#define TAU_GLOBAL_PHASE_START(timer) 
#define TAU_GLOBAL_PHASE_STOP(timer)  
#define TAU_GLOBAL_PHASE_EXTERNAL(timer) 
#define TAU_GLOBAL_TIMER(timer, name, type, group)
#define TAU_GLOBAL_TIMER_EXTERNAL(timer)
#define TAU_GLOBAL_TIMER_START(timer)
#define TAU_GLOBAL_TIMER_STOP()

#endif  /* USE_TAU */

/* Try to infer the endianness of the host based on compiler
   predefined macros.  For systems on which the compiler does not
   define these macros, we rely on ppremake to define WORDS_BIGENDIAN
   correctly.  For systems on which the compiler *does* define these
   macros, we ignore what ppremake said and define WORDS_BIGENDIAN
   correctly here.  (This is essential on OSX, which requires
   compiling each file twice in different modes, for universal binary
   support.) */

#if defined(__LITTLE_ENDIAN__) || defined(__i386__)
#undef WORDS_BIGENDIAN

#elif defined(__BIG_ENDIAN__) || defined(__ppc__)
#undef WORDS_BIGENDIAN
#define WORDS_BIGENDIAN 1

#endif

/* Try to determine if we're compiling in a 64-bit mode. */

#if defined(_LP64)
#define NATIVE_WORDSIZE 64
#else
#define NATIVE_WORDSIZE 32
#endif


/*
 We define the macros BEGIN_PUBLISH and END_PUBLISH to bracket
 functions and global variable definitions that are to be published
 via interrogate to scripting languages.  Also, the macro BLOCKING is
 used to flag any function or method that might perform I/O blocking
 and thus needs to release Python threads for its duration.
 */
#ifdef CPPPARSER
#define BEGIN_PUBLISH __begin_publish
#define END_PUBLISH __end_publish
#define BLOCKING __blocking
#define MAKE_SEQ(seq_name, num_name, element_name) __make_seq(seq_name, num_name, element_name)
#undef USE_STL_ALLOCATOR  // Don't try to parse these template classes in interrogate.
#else
#define BEGIN_PUBLISH
#define END_PUBLISH
#define BLOCKING
#define MAKE_SEQ(seq_name, num_name, element_name)
#endif

#ifdef __cplusplus
#include "dtoolbase_cc.h"
#endif

#endif

