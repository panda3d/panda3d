// Filename: pathStore.h
// Created by:  drose (10Feb03)
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
