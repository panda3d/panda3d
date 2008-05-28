// Filename: post_maya_include.h
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

// This header file works in conjunction with pre_maya_include.h; it
// cleans up some of the definitions that it left open.

// Remove the symbols defined from pre_maya_include.h.
#ifdef MAYA_PRE_5_0
#undef ostream
#undef istream
#endif  // MAYA_PRE_5_0
