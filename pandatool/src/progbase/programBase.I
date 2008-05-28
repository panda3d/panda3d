// Filename: programBase.I
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


////////////////////////////////////////////////////////////////////
//     Function: ProgramBase::show_text
//       Access: Public
//  Description: Formats the indicated text to stderr with the known
//               _terminal_width.
////////////////////////////////////////////////////////////////////
INLINE void ProgramBase::
show_text(const string &text) {
  show_text("", 0, text);
}
