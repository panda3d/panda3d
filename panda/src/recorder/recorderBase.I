// Filename: recorderBase.I
// Created by:  drose (24Jan04)
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
//     Function: RecorderBase::is_recording
//       Access: Published
//  Description: Returns true if this recorder is presently recording
//               data for saving to a session file, false otherwise.
//               If this is true, record_data() will be called from
//               time to time.
////////////////////////////////////////////////////////////////////
INLINE bool RecorderBase::
is_recording() const {
  return (_flags & F_recording) != 0;
}

////////////////////////////////////////////////////////////////////
//     Function: RecorderBase::is_playing
//       Access: Published
//  Description: Returns true if this recorder is presently playing back
//               data from session file, false otherwise.  If this is
//               true, play_data() will be called from time to time.
////////////////////////////////////////////////////////////////////
INLINE bool RecorderBase::
is_playing() const {
  return (_flags & F_playing) != 0;
}
