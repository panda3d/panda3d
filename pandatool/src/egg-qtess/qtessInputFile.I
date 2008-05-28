// Filename: qtessInputFile.I
// Created by:  drose (13Oct03)
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
//     Function: QtessInputFile::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE QtessInputFile::
QtessInputFile(const QtessInputFile &copy) : 
  _entries(copy._entries) 
{ 
}

////////////////////////////////////////////////////////////////////
//     Function: QtessInputFile::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void QtessInputFile::
operator = (const QtessInputFile &copy) {
  _entries = copy._entries;
}
