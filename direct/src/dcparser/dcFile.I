// Filename: dcFile.I
// Created by:  drose (10Jan06)
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
//     Function: DCFile::all_objects_valid
//       Access: Published
//  Description: Returns true if all of the classes read from the DC
//               file were defined and valid, or false if any of them
//               were undefined ("bogus classes").  If this is true,
//               we might have read a partial file.
////////////////////////////////////////////////////////////////////
INLINE bool DCFile::
all_objects_valid() const {
  return _all_objects_valid;
}

////////////////////////////////////////////////////////////////////
//     Function: DCFile::check_inherited_fields
//       Access: Public
//  Description: Rebuilds all of the inherited fields tables, if
//               necessary.
////////////////////////////////////////////////////////////////////
INLINE void DCFile::
check_inherited_fields() {
  if (_inherited_fields_stale) {
    rebuild_inherited_fields();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCFile::mark_inherited_fields_stale
//       Access: Public
//  Description: Indicates that something has changed in one or more
//               of the inheritance chains or the set of fields; the
//               next time check_inherited_fields() is called, the
//               inherited fields tables of all classes will be
//               rebuilt.
////////////////////////////////////////////////////////////////////
INLINE void DCFile::
mark_inherited_fields_stale() {
  _inherited_fields_stale = true;
}
