// Filename: lineStream.I
// Created by:  drose (26Feb00)
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
//     Function: LineStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE LineStream::
LineStream() : ostream(&_lsb) {
}

////////////////////////////////////////////////////////////////////
//     Function: LineStream::is_text_available
//       Access: Public
//  Description: Returns true if there is at least one line of text
//               (or even a partial line) available in the LineStream
//               object.  If this returns true, the line may then be
//               retrieved via get_line().
////////////////////////////////////////////////////////////////////
INLINE bool LineStream::
is_text_available() const {
  return _lsb.is_text_available();
}

////////////////////////////////////////////////////////////////////
//     Function: LineStream::get_line
//       Access: Public
//  Description: Extracts and returns the next line (or partial line)
//               of text available in the LineStream object.  Once the
//               line has been extracted, you may call has_newline()
//               to determine whether or not there was an explicit
//               newline character written following this line.
////////////////////////////////////////////////////////////////////
INLINE string LineStream::
get_line() {
  return _lsb.get_line();
}

////////////////////////////////////////////////////////////////////
//     Function: LineStream::has_newline
//       Access: Public
//  Description: Returns true if the line of text most recently
//               returned by get_line() was written out with a
//               terminating newline, or false if a newline character
//               has not yet been written to the LineStream.
////////////////////////////////////////////////////////////////////
INLINE bool LineStream::
has_newline() const {
  return _lsb.has_newline();
}
