// Filename: programBase.I
// Created by:  drose (28Jun00)
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
//     Function: ProgramBase::show_text
//       Access: Public
//  Description: Formats the indicated text to stderr with the known
//               _terminal_width.
////////////////////////////////////////////////////////////////////
INLINE void ProgramBase::
show_text(const string &text) {
  show_text("", 0, text);
}
