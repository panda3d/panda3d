// Filename: lwoHeader.I
// Created by:  drose (24Apr01)
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


////////////////////////////////////////////////////////////////////
//     Function: LwoHeader::is_valid
//       Access: Public
//  Description: Returns true if the header represents a valid and
//               recognized Lightwave header, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool LwoHeader::
is_valid() const {
  return _valid;
}

////////////////////////////////////////////////////////////////////
//     Function: LwoHeader::get_version
//       Access: Public
//  Description: Returns the version of the Lightwave file.
////////////////////////////////////////////////////////////////////
INLINE double LwoHeader::
get_version() const {
  return _version;
}

