// Filename: pathStore.cxx
// Created by:  drose (10Feb03)
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

#include "pathStore.h"

#include "string_utils.h"
#include "notify.h"

////////////////////////////////////////////////////////////////////
//     Function: format_path_store
//  Description: Returns the string corresponding to this method.
////////////////////////////////////////////////////////////////////
string
format_path_store(PathStore store) {
  switch (store) {
  case PS_invalid:
    return "invalid";

  case PS_relative:
    return "relative";

  case PS_absolute:
    return "absolute";

  case PS_rel_abs:
    return "rel_abs";

  case PS_strip:
    return "strip";

  case PS_keep:
    return "keep";
  }
  nout << "**unexpected PathStore value: (" << (int)store << ")**";
  return "**";
}

////////////////////////////////////////////////////////////////////
//     Function: PathStore output operator
//  Description:
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, PathStore store) {
  return out << format_path_store(store);
}

////////////////////////////////////////////////////////////////////
//     Function: string_path_store
//  Description: Stores from a string, as might be input by the
//               user, to one of the known PathStore types.
//               Returns PS_invalid if the string is unknown.
////////////////////////////////////////////////////////////////////
PathStore
string_path_store(const string &str) {
  if (cmp_nocase(str, "relative") == 0 || 
      cmp_nocase(str, "rel") == 0) {
    return PS_relative;

  } else if (cmp_nocase(str, "absolute") == 0 ||
             cmp_nocase(str, "abs") == 0) {
    return PS_absolute;

  } else if (cmp_nocase_uh(str, "rel_abs") == 0) {
    return PS_rel_abs;

  } else if (cmp_nocase(str, "strip") == 0) {
    return PS_strip;

  } else if (cmp_nocase(str, "keep") == 0) {
    return PS_keep;

  } else {
    return PS_invalid;
  }
}
