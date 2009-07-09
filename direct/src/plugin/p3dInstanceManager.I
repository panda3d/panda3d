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
//     Function: P3DInstanceManager::get_download_url
//       Access: Public
//  Description: Returns the URL of the download server.  All
//               downloadable files will be retrieved from various
//               subdirectories of this URL root.
////////////////////////////////////////////////////////////////////
inline const string &P3DInstanceManager::
get_download_url() const {
  return _download_url;
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
