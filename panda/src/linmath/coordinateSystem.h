// Filename: coordinateSystem.h
// Created by:  drose (24Sep99)
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

#ifndef COORDINATESYSTEM_H
#define COORDINATESYSTEM_H

#include "pandabase.h"

#include <typedef.h>

#include <string>

BEGIN_PUBLISH

enum CoordinateSystem {
  // The CS_default entry does not refer to a particular coordinate
  // system, but rather to the value stored in
  // default_coordinate_system, which in turn is loaded from the
  // Configrc variable "coordinate-system".
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

END_PUBLISH

extern CoordinateSystem EXPCL_PANDA default_coordinate_system;

CoordinateSystem EXPCL_PANDA parse_coordinate_system_string(const string &str);
bool EXPCL_PANDA is_right_handed(CoordinateSystem cs = CS_default);

#define IS_LEFT_HANDED_COORDSYSTEM(cs) ((cs==CS_zup_left) || (cs==CS_yup_left))

ostream EXPCL_PANDA &operator << (ostream &out, CoordinateSystem cs);


#endif

