/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configFlags.cxx
 * @author drose
 * @date 2004-10-21
 */

#include "configFlags.h"

TVOLATILE AtomicAdjust::Integer ConfigFlags::_global_modified;

/**
 *
 */
std::ostream &
operator << (std::ostream &out, ConfigFlags::ValueType type) {
  switch (type) {
  case ConfigFlags::VT_undefined:
    return out << "undefined";

  case ConfigFlags::VT_list:
    return out << "list";

  case ConfigFlags::VT_string:
    return out << "string";

  case ConfigFlags::VT_filename:
    return out << "filename";

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

  case ConfigFlags::VT_int64:
    return out << "int64";

  case ConfigFlags::VT_color:
    return out << "color";
  }

  return out << "**invalid(" << (int)type << ")**";
}
