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
//     Function: P3DInstance::get_session_platform
//       Access: Public
//  Description: Returns the platform of this particular session.
//               Before the panda3d package has been seen, this is the
//               empty string; once we have downloaded the info file
//               for the panda3d package, it is filled in with
//               whatever platform is provided (that we're also
//               runtime-compatible with).
//
//               Presumably all of the platform-specific packages that
//               are downloaded subsequently must be of the exact same
//               platform.
////////////////////////////////////////////////////////////////////
inline const string &P3DInstance::
get_session_platform() const {
  return _session_platform;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::get_session
//       Access: Public
//  Description: Returns the P3DSession that is hosting this instance,
//               or NULL if the instance is not running.
////////////////////////////////////////////////////////////////////
inline P3DSession *P3DInstance::
get_session() const {
  return _session;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::get_request_ready_func
//       Access: Public
//  Description: Returns a pointer to the asynchronous notification
//               function that was passed to the constructor, if any,
//               or NULL if asynchronous notifications are not
//               required.
////////////////////////////////////////////////////////////////////
inline P3D_request_ready_func *P3DInstance::
get_request_ready_func() const {
  return _func;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::is_trusted
//       Access: Public
//  Description: Returns true if this instance's p3d file is trusted
//               and ready to launch, false if it needs to be approved
//               by the user.
////////////////////////////////////////////////////////////////////
inline bool P3DInstance::
is_trusted() const {
  return _p3d_trusted;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::get_matches_script_origin
//       Access: Public
//  Description: Returns true if this instance is allowed to be
//               scripted by its embedding web page, false otherwise.
//               This may not be known until the p3d file has been
//               fully downloaded and opened.
////////////////////////////////////////////////////////////////////
inline bool P3DInstance::
get_matches_script_origin() const {
  return _matches_script_origin;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::is_started
//       Access: Public
//  Description: Returns true if this instance has already been
//               started within some session, false otherwise.
////////////////////////////////////////////////////////////////////
inline bool P3DInstance::
is_started() const {
  return (_session != NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::is_failed
//       Access: Public
//  Description: Returns true if this instance has tried and failed to
//               launch for some reason.
////////////////////////////////////////////////////////////////////
inline bool P3DInstance::
is_failed() const {
  return _failed;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::ImageFile::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
inline P3DInstance::ImageFile::
ImageFile() {
  _use_standard_image = true;
  _temp_filename = NULL;
  _image_placement = P3DSplashWindow::IP_none;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::ImageFile::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
inline P3DInstance::ImageFile::
~ImageFile() {
  cleanup();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstance::ImageFile::cleanup
//       Access: Public
//  Description: Removes the temporary file, if any.
////////////////////////////////////////////////////////////////////
inline void P3DInstance::ImageFile::
cleanup() {
  if (_temp_filename != NULL) {
    delete _temp_filename;
    _temp_filename = NULL;
  }
}
