// Filename: pre_maya_include.h
// Created by:  drose (11Apr02)
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

// This header file defines a few things that are necessary to define
// before including any Maya headers, just to work around some of
// Max's assumptions about the compiler.  It must not try to protect
// itself from multiple inclusion with #ifdef .. #endif, since it must
// be used each time it is included.

// Max will try to typedef bool unless this symbol is defined.
#ifndef _BOOL
#define _BOOL 1
#endif

// Max tries to make a forward declaration for class ostream, but
// this is not necessarily a class!  Curses.  We can't use any of the
// built-in Max stream operators, and we have to protect ourselves
// from them.
#define ostream max_ostream
#define istream max_istream
