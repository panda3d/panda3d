// Filename: bamFile.I
// Created by:  drose (02Jul00)
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

