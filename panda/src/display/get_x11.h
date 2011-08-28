// Filename: get_x11.h
// Created by:  drose (28Aug11)
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

#ifndef GET_X11_H
#define GET_X11_H

#include "pandabase.h"

#ifdef HAVE_X11 
// This header file is designed to help work around some of the
// namespace spamming that X11 causes, by renaming the symbols that
// X11 declares that are known to conflict with other library names
// (like Apple's Core Graphics, for instance).

// In order for this to work, everyone who uses X11 within Panda
// should include this file instead of including the X11 headers
// directly.

#ifdef CPPPARSER
// A simple hack so interrogate can get all of the necessary
// typenames.
typedef int X11_Display;
typedef int X11_Window;
typedef int X11_Cursor;
typedef int XErrorEvent;
typedef int XVisualInfo;
typedef int Atom;
typedef int XIM;
typedef int XIC;
#else

#include "pre_x11_include.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>

#ifdef HAVE_XRANDR
#include <X11/extensions/Xrandr.h>
#endif  // HAVE_XRANDR

#ifdef HAVE_XCURSOR
#include <X11/Xcursor/Xcursor.h>
#endif

#ifdef HAVE_XF86DGA
#include <X11/extensions/xf86dga.h>
#endif

#include "post_x11_include.h"

#endif  // CPPPARSER
#endif  // HAVE_X11

#endif
