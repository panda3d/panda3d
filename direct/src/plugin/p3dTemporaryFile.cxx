// Filename: p3dTemporaryFile.cxx
// Created by:  drose (19Aug09)
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

#include "p3dTemporaryFile.h"
#include "p3dInstanceManager.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DTemporaryFile::Constructor
//       Access: Public
//  Description: Constructs a new, unique temporary filename.
////////////////////////////////////////////////////////////////////
P3DTemporaryFile::
P3DTemporaryFile(const string &extension) {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  _filename = inst_mgr->make_temp_filename(extension);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DTemporaryFile::Destructor
//       Access: Public
//  Description: Deletes the temporary file, if it exists.
////////////////////////////////////////////////////////////////////
P3DTemporaryFile::
~P3DTemporaryFile() {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  inst_mgr->release_temp_filename(_filename);
}
