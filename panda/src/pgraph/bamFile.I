// Filename: bamFile.I
// Created by:  drose (02Jul00)
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
//     Function: BamFile::is_valid_read
//       Access: Public
//  Description: Returns true if the Bam file is open and ready for
//               reading with no errors so far detected, or false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool BamFile::
is_valid_read() const {
  return (_reader != (BamReader *)NULL);
}


////////////////////////////////////////////////////////////////////
//     Function: BamFile::is_valid_write
//       Access: Public
//  Description: Returns true if the Bam file is open and ready for
//               writing with no errors so far detected, or false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool BamFile::
is_valid_write() const {
  return (_writer != (BamWriter *)NULL);
}

