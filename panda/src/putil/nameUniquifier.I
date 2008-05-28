// Filename: nameUniquifier.I
// Created by:  drose (16Feb00)
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


////////////////////////////////////////////////////////////////////
//     Function: NameUniquifier::add_name
//       Access: Public
//  Description: If name is nonempty and so far unique, returns it
//               unchanged.
//
//               Otherwise, generates and returns a new name according
//               to the following rules:
//
//               If the name is empty, the new name is the
//               NameUniquifier's "empty" string followed by a number,
//               or the "separator" string if the "empty" string is
//               empty.
//
//               If the name is nonempty, the new name is the original
//               name, followed by the NameUniquifier's "separator"
//               string, followed by a number.
////////////////////////////////////////////////////////////////////
INLINE string NameUniquifier::
add_name(const string &name) {
  return add_name_body(name, name);
}

////////////////////////////////////////////////////////////////////
//     Function: NameUniquifier::add_name
//       Access: Public
//  Description: If name is nonempty and so far unique, returns it
//               unchanged.
//
//               Otherwise, generates and returns a new name according
//               to the following rules:
//
//               If the prefix is empty, the new name is the
//               NameUniquifier's "empty" string followed by a number,
//               or the "separator" string if the "empty" string is
//               empty.
//
//               If the prefix is nonempty, the new name is the
//               prefix, followed by the NameUniquifier's "separator"
//               string, followed by a number.
////////////////////////////////////////////////////////////////////
INLINE string NameUniquifier::
add_name(const string &name, const string &prefix) {
  return add_name_body(name, prefix);
}
