// Filename: autoTextureScale.h
// Created by:  drose (28Nov11)
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

#ifndef AUTOTEXTURESCALE_H
#define AUTOTEXTURESCALE_H

#include "pandabase.h"

BEGIN_PUBLISH
enum AutoTextureScale {
  ATS_none,
  ATS_down,
  ATS_up,
  ATS_pad,
  ATS_unspecified,
};
END_PUBLISH

EXPCL_PANDA_PUTIL ostream &operator << (ostream &out, AutoTextureScale ats);
EXPCL_PANDA_PUTIL istream &operator >> (istream &in, AutoTextureScale &ats);

#endif
