// Filename: pathStore.h
// Created by:  drose (10Feb03)
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

#ifndef PATHSTORE_H
#define PATHSTORE_H

#include "pandatoolbase.h"

////////////////////////////////////////////////////////////////////
//        Enum : PathStore
// Description : This enumerated type lists the methods by which
//               a filename path might be mangled before storing in a
//               destination file.
////////////////////////////////////////////////////////////////////
enum PathStore {
  PS_invalid,    // Never use this.
  PS_relative,   // Make relative to a user-specified directory.
  PS_absolute,   // Make absolute.
  PS_rel_abs,    // Make relative if within the directory, otherwise absolute.
  PS_strip,      // Strip prefix and just store the basename.
  PS_keep,       // Don't change the filename at all.
};

string format_path_store(PathStore unit);

ostream &operator << (ostream &out, PathStore unit);
PathStore string_path_store(const string &str);

#endif
