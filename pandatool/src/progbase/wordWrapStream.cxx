// Filename: wordWrapStream.cxx
// Created by:  drose (28Jun00)
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

#include "wordWrapStream.h"


////////////////////////////////////////////////////////////////////
//     Function: WordWrapStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
WordWrapStream::
WordWrapStream(ProgramBase *program) :
  ostream(&_lsb),
  _lsb(this, program)
{
}
