/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dTemporaryFile.cxx
 * @author drose
 * @date 2009-08-19
 */

#include "p3dTemporaryFile.h"
#include "p3dInstanceManager.h"

/**
 * Constructs a new, unique temporary filename.
 */
P3DTemporaryFile::
P3DTemporaryFile(const std::string &extension) {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  _filename = inst_mgr->make_temp_filename(extension);
}

/**
 * Deletes the temporary file, if it exists.
 */
P3DTemporaryFile::
~P3DTemporaryFile() {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  inst_mgr->release_temp_filename(_filename);
}
