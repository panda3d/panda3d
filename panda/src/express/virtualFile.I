// Filename: virtualFile.I
// Created by:  drose (03Aug02)
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
//     Function: VirtualFile::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VirtualFile::
VirtualFile() {
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFile::read_file
//       Access: Public
//  Description: Returns the entire contents of the file as a string.
////////////////////////////////////////////////////////////////////
INLINE string VirtualFile::
read_file() const {
  string result;
  read_file(result);
  return result;
}


INLINE ostream &
operator << (ostream &out, const VirtualFile &file) {
  file.output(out);
  return out;
}

