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
#include "npupp.h"

#include "load_plugin.h"

// If we are building with a version of Gecko that supports the
// asynchronous callback function, we should use it--it's just so
// handy.
#if defined(NPVERS_HAS_PLUGIN_THREAD_ASYNC_CALL) && NP_VERSION_MINOR >= NPVERS_HAS_PLUGIN_THREAD_ASYNC_CALL
#define HAS_PLUGIN_THREAD_ASYNC_CALL 1
#endif

// Appears in startup.cxx.
extern NPNetscapeFuncs *browser;

#endif

