// Filename: xml_helpers.cxx
// Created by:  drose (28Sep12)
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

#include "p3d_plugin_common.h"
#include "xml_helpers.h"


////////////////////////////////////////////////////////////////////
//     Function: parse_bool_attrib
//  Description: Examines the indicated attrib from the XML attrib and
//               returns its true or false value.  Returns
//               default_value if the attrib is not present or is
//               empty.
////////////////////////////////////////////////////////////////////
bool
parse_bool_attrib(TiXmlElement *xelem, const string &attrib,
                  bool default_value) {
  const char *value = xelem->Attribute(attrib.c_str());
  if (value == NULL || *value == '\0') {
    return default_value;
  }

  char *endptr;
  int result = strtol(value, &endptr, 10);
  if (*endptr == '\0') {
    // A valid integer.
    return (result != 0);
  }

  // An invalid integer.
  return default_value;
}
