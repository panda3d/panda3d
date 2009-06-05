// Filename: p3dPython.I
// Created by:  drose (04Jun09)
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
//     Function: P3DPython::get_python_version
//       Access: Public
//  Description: Returns a string that uniquely identifies this
//               Python version.  You cannot have two different
//               versions of Python loaded simultaneously within the
//               same address space.
////////////////////////////////////////////////////////////////////
INLINE const string &P3DPython::
get_python_version() const {
  return _python_version;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPython::get_num_sessions
//       Access: Public
//  Description: Returns the number of sessions currently sharing
//               this Python address space.
////////////////////////////////////////////////////////////////////
INLINE int P3DPython::
get_num_sessions() const {
  return _sessions.size();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPython::is_valid
//       Access: Public
//  Description: Returns true if the Python interpreter is up and
//               ready to run code, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool P3DPython::
is_valid() const {
  return _is_valid;
}
