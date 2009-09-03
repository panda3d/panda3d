// Filename: p3dInstanceManager.I
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
//     Function: P3DInstanceManager::is_initialized
//       Access: Public
//  Description: Returns true if the instance manager is successfully
//               initialized, false otherwise.
////////////////////////////////////////////////////////////////////
inline bool P3DInstanceManager::
is_initialized() const {
  return _is_initialized;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::verify_contents
//       Access: Public
//  Description: Returns the verify_contents flag.  When this is set
//               false, it indicates that we don't need to contact the
//               server to verify that a contents.xml file is fresh
//               before using it; we should just use it as it is.
////////////////////////////////////////////////////////////////////
inline bool P3DInstanceManager::
get_verify_contents() const {
  return _verify_contents;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::reset_verify_contents
//       Access: Public
//  Description: Resets the verify_contents flag to true.  This should
//               be done whenever we discover anything needs to be
//               downloaded.  At this point, we might as well verify
//               everything.
////////////////////////////////////////////////////////////////////
inline void P3DInstanceManager::
reset_verify_contents() {
  _verify_contents = true;
}


////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_root_dir
//       Access: Public
//  Description: Returns the root directory into which all the P3D
//               runtime files are downloaded and installed.  This
//               must be a writable directory or nothing will work.
////////////////////////////////////////////////////////////////////
inline const string &P3DInstanceManager::
get_root_dir() const {
  return _root_dir;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_platform
//       Access: Public
//  Description: Returns the string that corresponds to the platform
//               on which we are running.  This string will be used to
//               determine the appropriate packages to download.
////////////////////////////////////////////////////////////////////
inline const string &P3DInstanceManager::
get_platform() const {
  return _platform;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_log_directory
//       Access: Public
//  Description: Returns the pathname of the directory into which all
//               log files should be written.  This filename will end
//               with a slash or backslash, as appropriate, so that
//               logfile pathnames may be made by concatenting
//               directly with this string.
////////////////////////////////////////////////////////////////////
inline const string &P3DInstanceManager::
get_log_directory() const {
  return _log_directory;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_log_pathname
//       Access: Public
//  Description: Returns the filename of the system log file; this
//               file is responsible for downloading and installing
//               updates, and launching applications.  This is
//               different from the session log file(s), which
//               represent the output from a particular Python
//               session.
////////////////////////////////////////////////////////////////////
inline const string &P3DInstanceManager::
get_log_pathname() const {
  return _log_pathname;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_keep_cwd
//       Access: Public
//  Description: Returns the value of the keep_cwd flag passed to the
//               constructor.  This is true if the original working
//               directory was valuable and meaningful, and should be
//               preserved; or false if it is meaningless and should
//               be changed.
////////////////////////////////////////////////////////////////////
inline bool P3DInstanceManager::
get_keep_cwd() const {
  return _keep_cwd;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_num_instances
//       Access: Public
//  Description: Returns the number of instances currently running
//               within the world.
////////////////////////////////////////////////////////////////////
inline int P3DInstanceManager::
get_num_instances() const {
  return _instances.size();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::new_undefined_object
//       Access: Public
//  Description: Returns the singleton "undefined" object, as a new
//               reference.
////////////////////////////////////////////////////////////////////
inline P3D_object *P3DInstanceManager::
new_undefined_object() {
  P3D_OBJECT_INCREF(_undefined_object);
  return _undefined_object;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::new_none_object
//       Access: Public
//  Description: Returns the singleton "none" object, as a new
//               reference.
////////////////////////////////////////////////////////////////////
inline P3D_object *P3DInstanceManager::
new_none_object() {
  P3D_OBJECT_INCREF(_none_object);
  return _none_object;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::new_bool_object
//       Access: Public
//  Description: Returns the singleton "true" or "false" object, as a
//               new reference.
////////////////////////////////////////////////////////////////////
inline P3D_object *P3DInstanceManager::
new_bool_object(bool value) {
  P3D_object *obj = (value) ? _true_object : _false_object;
  P3D_OBJECT_INCREF(obj);
  return obj;
}
