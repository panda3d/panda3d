// Filename: lwoInputFile.I
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
//     Function: LwoInputFile::get_lwo_version
//       Access: Public
//  Description: Returns the version of the Lightwave file being read.
//               This is unknown until the header record has been
//               read; then it will be set by the header.
////////////////////////////////////////////////////////////////////
INLINE double LwoInputFile::
get_lwo_version() const {
  return _lwo_version;
}


////////////////////////////////////////////////////////////////////
//     Function: LwoInputFile::set_lwo_version
//       Access: Public
//  Description: Changes the version number reported for the Lightwave
//               file.  Normally this is only called by LwoHeader as
//               it is read.
////////////////////////////////////////////////////////////////////
INLINE void LwoInputFile::
set_lwo_version(double lwo_version) {
  _lwo_version = lwo_version;
}

