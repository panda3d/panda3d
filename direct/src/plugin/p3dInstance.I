// Filename: p3dInstance.I
// Created by:  drose (29May09)
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
//     Function: P3DInstance::get_fparams
//       Access: Public
//  Description: Returns the current file parameters.
////////////////////////////////////////////////////////////////////
inline const P3DFileParams &P3DInstance::
get_fparams() const {
  return _fparams;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::get_wparams
//       Access: Public
//  Description: Returns the current window parameters.
////////////////////////////////////////////////////////////////////
inline const P3DWindowParams &P3DInstance::
get_wparams() const {
  return _wparams;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::get_instance_id
//       Access: Public
//  Description: Returns a unique integer for each instance in the
//               system.
////////////////////////////////////////////////////////////////////
inline int P3DInstance::
get_instance_id() const {
  return _instance_id;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::get_session_key
//       Access: Public
//  Description: Returns a string that uniquely identifies this
//               session.  This is a constructed string that includes
//               the supplied session_name, the python and panda
//               version, and the publisher, as well as any other
//               relevant details; it is guaranteed to be unique for
//               each unique session required for different
//               P3DInstances.
////////////////////////////////////////////////////////////////////
inline const string &P3DInstance::
get_session_key() const {
  return _session_key;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::get_python_version
//       Access: Public
//  Description: Returns a string that uniquely identifies this
//               instance's required Python version.
////////////////////////////////////////////////////////////////////
inline const string &P3DInstance::
get_python_version() const {
  return _python_version;
}
