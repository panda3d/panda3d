// Filename: coordinateSystem.h
// Created by:  drose (24Sep99)
//
////////////////////////////////////////////////////////////////////

#ifndef COORDINATESYSTEM_H
#define COORDINATESYSTEM_H

#include <pandabase.h>

#include <typedef.h>

#include <string>

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

extern CoordinateSystem EXPCL_PANDA default_coordinate_system;

CoordinateSystem EXPCL_PANDA parse_coordinate_system_string(const string &str);
bool EXPCL_PANDA is_right_handed(CoordinateSystem cs = CS_default);

ostream EXPCL_PANDA &operator << (ostream &out, CoordinateSystem cs);


#endif

