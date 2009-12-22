// Filename: nppanda3d_common.h
// Created by:  drose (19Jun09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef NPPANDA3D_COMMON
#define NPPANDA3D_COMMON

// This header file is included by all C++ files in this directory

// It's a good idea to pick up this header file, even though we don't
// actually link with dtool.  This header file defines useful
// system-wide config settings.
#include "dtool_config.h"

// We include this header file directly out of its source directory,
// so we don't have to link with the library that builds it.
#include "../plugin/p3d_plugin.h"

#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>

using namespace std;

// Appears in startup.cxx.
extern ostream *nout_stream;
#define nout (*nout_stream)

extern string global_root_dir;
extern bool has_plugin_thread_async_call;

#ifdef _WIN32

// Gecko requires all these symbols to be defined for Windows.
#define MOZILLA_STRICT_API
#define XP_WIN
#define _X86_
#define _WINDOWS
#define _USRDLL
#define NPBASIC_EXPORTS

// Panda already defines this one.
//#define WIN32

#include <windows.h>

#elif defined(__APPLE__)

// On Mac, Gecko requires this symbol to be defined.
#define XP_MACOSX

#else

#define XP_UNIX

#endif  // _WIN32, __APPLE__

#include "npapi.h"
#if NP_VERSION_MAJOR == 0 && NP_VERSION_MINOR <= 19
  #include "npupp.h"
#else
  // Somewhere between version 0.19 and 0.22, Mozilla renamed npupp.h to
  // npfunctions.h.
  #include "npfunctions.h"
#endif

#if NP_VERSION_MAJOR == 0 && NP_VERSION_MINOR <= 19
  #ifdef _WIN32
    // Also somewhere in there, they started defining and using
    // int16_t instead of int16, and so on.  They already had int32_t
    // from earlier, but the typedef is not quite the same as int32
    // (!), so we have to use #define to keep things compatible.
    typedef int16 int16_t;
    typedef uint16 uint16_t;
//    #define int32_t int32
//    #define uint32_t uint32
  #endif  // _WIN32
#endif  // NP_VERSION

#include "load_plugin.h"

// Mozilla's version of NPAPI has these names lowercase.  WebKit's
// version has them uppercase.  What a mess.  We have to define a
// duplicate of the structure to allow us to reference them
// consistently.
struct UC_NPString {
  const NPUTF8 *UTF8Characters;
  uint32_t UTF8Length;
};


// If we are building with a version of Gecko that supports the
// asynchronous callback function, we should use it--it's just so
// handy.
#if defined(NPVERS_HAS_PLUGIN_THREAD_ASYNC_CALL) && NP_VERSION_MINOR >= NPVERS_HAS_PLUGIN_THREAD_ASYNC_CALL
#define HAS_PLUGIN_THREAD_ASYNC_CALL 1
#endif

// We also need to know whether we have Apple's new Cocoa-based
// drawing and event callbacks.
#if defined(NPVERS_MACOSX_HAS_EVENT_MODELS) && NP_VERSION_MINOR >= NPVERS_MACOSX_HAS_EVENT_MODELS
#define MACOSX_HAS_EVENT_MODELS 1
#endif

// No one defined a symbol for the introduction of the Cocoa drawing,
// but it appears to have been version 19.
#if NP_VERSION_MINOR >= 19
#define MACOSX_HAS_COREGRAPHICS_DRAWING_MODEL 1
#endif

// Appears in startup.cxx.
extern NPNetscapeFuncs *browser;

#endif

