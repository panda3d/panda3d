// Filename: cppVisibility.h
// Created by:  drose (22Oct99)
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

#ifndef CPPVISIBILITY_H
#define CPPVISIBILITY_H

#include "dtoolbase.h"

enum CPPVisibility {
  V_published,
  V_public,
  V_protected,
  V_private,
  V_unknown
};

ostream &operator << (ostream &out, CPPVisibility vis);

#endif

