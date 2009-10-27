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
inline const string &P3DSession::
get_session_key() const {
  return _session_key;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::get_log_pathname
//       Access: Public
//  Description: Returns the log filename for this particular session,
//               if the session was started and if it has a log file.
//               Returns empty string if the session never started or
//               if it lacks a log file.
////////////////////////////////////////////////////////////////////
inline const string &P3DSession::
get_log_pathname() const {
  return _log_pathname;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::get_matches_script_origin
//       Access: Public
//  Description: Returns true if the instances of this session are
//               allowed to be scripted by its embedding web page,
//               false otherwise.
////////////////////////////////////////////////////////////////////
inline bool P3DSession::
get_matches_script_origin() const {
  return _matches_script_origin;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSession::get_num_instances
//       Access: Public
//  Description: Returns the number of instances currently running
//               within the session.  When this is zero, the session
//               may be safely deleted.
////////////////////////////////////////////////////////////////////
inline int P3DSession::
get_num_instances() const {
  return _instances.size();
}
