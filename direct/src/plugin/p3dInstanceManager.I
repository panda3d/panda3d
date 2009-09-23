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
//     Function: P3DInstanceManager::get_trusted_environment
//       Access: Public
//  Description: Returns the value of the trusted_environment flag
//               passed to the constructor.  If this is true, it means
//               the environment we are running in is trusted and the
//               p3d file is already vetted.  This means the current
//               working directory will remain unchanged, and the p3d
//               file will be run without checking its signature.
//
//               This should generally be true only when run by
//               panda3d.exe, and not when run by the web plugin.
////////////////////////////////////////////////////////////////////
inline bool P3DInstanceManager::
get_trusted_environment() const {
  return _trusted_environment;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_super_mirror
//       Access: Public
//  Description: Returns the "super mirror" URL.  See p3d_plugin.h.
////////////////////////////////////////////////////////////////////
inline const string &P3DInstanceManager::
get_super_mirror() const {
  return _super_mirror_url;
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

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::encode_hexdigit
//       Access: Public
//  Description: Returns the hex digit corresponding to the
//               indicated integer value.
////////////////////////////////////////////////////////////////////
inline char P3DInstanceManager::
encode_hexdigit(int c) {
  if (c >= 10) {
    return c - 10 + 'a';
  }
  return c + '0';
}
