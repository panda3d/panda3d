// Filename: pre_maya_include.h
// Created by:  drose (11Apr02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

// This header file defines a few things that are necessary to define
// before including any Maya headers, just to work around some of
// Maya's assumptions about the compiler.  It must not try to protect
// itself from multiple inclusion with #ifdef .. #endif, since it must
// be used each time it is included.

// Maya will try to typedef bool unless this symbol is defined.
#ifndef _BOOL
#define _BOOL 1
#endif

// In Maya 5.0, the headers seem to provide the manifest
// REQUIRE_IOSTREAM, which forces it to use the new <iostream> headers
// instead of the old <iostream.h> headers.  It also says this is for
// Linux only, but it seems to work just fine on Windows, obviating
// the need for sneaky #defines in this and in post_maya_include.h.
#ifdef HAVE_IOSTREAM
#define REQUIRE_IOSTREAM
#endif  // HAVE_IOSTREAM

