// Filename: pStatClientVersion.I
// Created by:  drose (21May01)
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
//     Function: PStatClientVersion::get_major_version
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int PStatClientVersion::
get_major_version() const {
  return _major_version;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientVersion::get_minor_version
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int PStatClientVersion::
get_minor_version() const {
  return _minor_version;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientVersion::set_version
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void PStatClientVersion::
set_version(int major_version, int minor_version) {
  _major_version = major_version;
  _minor_version = minor_version;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientVersion::is_at_least
//       Access: Public
//  Description: Returns true if the client version is at least the
//               indicated major/minor version number, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool PStatClientVersion::
is_at_least(int major_version, int minor_version) const {
  return (_major_version > major_version ||
          (_major_version == major_version && _minor_version >= minor_version));
}
