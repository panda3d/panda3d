// Filename: wordWrapStreamBuf.I
// Created by:  drose (01Jul00)
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
//     Function: WordWrapStreamBuf::set_literal_mode
//       Access: Private
//  Description: An internal function called to update the internal
//               state according to the current value of the
//               Notify::literal flag, which might or might not be set
//               of the ostream at any time.  When the literal flag is
//               true, we should not word-wrap, so toggling this flag
//               means we need to flush the current buffer.
////////////////////////////////////////////////////////////////////
INLINE void WordWrapStreamBuf::
set_literal_mode(bool mode) {
  if (mode != _literal_mode) {
    flush_data();
    _literal_mode = mode;
  }
}

