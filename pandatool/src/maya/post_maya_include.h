// Filename: post_maya_include.h
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

// This header file works in conjunction with pre_maya_include.h; it
// cleans up some of the definitions that it left open.

// Remove the symbols defined from pre_maya_include.h.
#ifdef MAYA_PRE_5_0
#undef ostream
#undef istream
#endif  // MAYA_PRE_5_0
