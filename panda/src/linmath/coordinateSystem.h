// Filename: coordinateSystem.h
// Created by:  drose (24Sep99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef COORDINATESYSTEM_H
#define COORDINATESYSTEM_H

#include "pandabase.h"

#include "typedef.h"

BEGIN_PUBLISH

enum CoordinateSystem {
  // The CS_default entry does not refer to a particular coordinate
  // system, but rather to the value stored in
  // default_coordinate_system, which in turn is loaded from the
  // config variable "coordinate-system".
  CS_default,

  CS_zup_right,
  CS_yup_right,
  CS_zup_left,
  CS_yup_left,

  // CS_invalid is not a coordinate system at all.  It can be used in
  // user-input processing code to indicate a contradictory coordinate
  // system request.
  CS_invalid,
};

EXPCL_PANDA CoordinateSystem get_default_coordinate_system();

END_PUBLISH

EXPCL_PANDA CoordinateSystem parse_coordinate_system_string(const string &str);
EXPCL_PANDA bool is_right_handed(CoordinateSystem cs = CS_default);

#define IS_LEFT_HANDED_COORDSYSTEM(cs) ((cs==CS_zup_left) || (cs==CS_yup_left))

EXPCL_PANDA ostream &operator << (ostream &out, CoordinateSystem cs);
EXPCL_PANDA istream &operator >> (istream &in, CoordinateSystem &cs);


#endif

