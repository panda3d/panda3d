// Filename: plugin_get_x11.h
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

#ifndef PLUGIN_GET_X11_H
#define PLUGIN_GET_X11_H

#include "pandabase.h"
#include "p3d_plugin_config.h"

#ifdef HAVE_X11 
// This header file is designed to help work around some of the
// namespace spamming that X11 causes, by renaming the symbols that
// X11 declares that are known to conflict with other library names
// (like Apple's Core Graphics, for instance).

// In order for this to work, everyone who uses X11 within Panda
// should include this file instead of including the X11 headers
// directly.

#include "pre_x11_include.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "post_x11_include.h"

#endif  // HAVE_X11

#endif
