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
//     Function: P3DInstanceManager::get_num_instances
//       Access: Public
//  Description: Returns the number of instances currently running
//               within the world.
////////////////////////////////////////////////////////////////////
INLINE int P3DInstanceManager::
get_num_instances() const {
  return _instances.size();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DInstanceManager::get_dll_filename
//       Access: Public
//  Description: Returns the DLL filename that was passed to
//               initialize().
////////////////////////////////////////////////////////////////////
INLINE const string &P3DInstanceManager::
get_dll_filename() const {
  return _dll_filename;
}
