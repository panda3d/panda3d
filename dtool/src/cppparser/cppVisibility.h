// Filename: cppVisibility.h
// Created by:  drose (22Oct99)
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

