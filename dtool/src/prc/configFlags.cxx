// Filename: configFlags.cxx
// Created by:  drose (21Oct04)
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

#include "configFlags.h"

////////////////////////////////////////////////////////////////////
//     Function: ConfigFlags::Type output operator
//  Description: 
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, ConfigFlags::ValueType type) {
  switch (type) {
  case ConfigFlags::VT_undefined:
    return out << "undefined";

  case ConfigFlags::VT_list:
    return out << "list";

  case ConfigFlags::VT_string:
    return out << "string";

  case ConfigFlags::VT_bool:
    return out << "bool";

  case ConfigFlags::VT_int:
    return out << "int";

  case ConfigFlags::VT_double:
    return out << "double";

  case ConfigFlags::VT_enum:
    return out << "enum";

  case ConfigFlags::VT_search_path:
    return out << "search-path";
  }

  return out << "**invalid(" << (int)type << ")**";
}
