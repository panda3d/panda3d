// Filename: p3dSession.I
// Created by:  drose (03Jun09)
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
//     Function: P3DSession::get_session_key
//       Access: Public
//  Description: Returns a string that uniquely identifies this
//               session.  See P3dInstance::get_session_key().
////////////////////////////////////////////////////////////////////
INLINE const string &P3DSession::
get_session_key() const {
  return _session_key;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::get_python_version
//       Access: Public
//  Description: Returns a string that uniquely identifies this
//               session's required Python version.
////////////////////////////////////////////////////////////////////
INLINE const string &P3DSession::
get_python_version() const {
  return _python_version;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::get_num_instances
//       Access: Public
//  Description: Returns the number of instances currently running
//               within the session.  When this is zero, the session
//               may be safely deleted.
////////////////////////////////////////////////////////////////////
INLINE int P3DSession::
get_num_instances() const {
  return _instances.size();
}
