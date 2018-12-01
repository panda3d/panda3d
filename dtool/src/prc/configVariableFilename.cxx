/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableFilename.cxx
 * @author drose
 * @date 2004-11-22
 */

#include "configVariableFilename.h"
#include "executionEnvironment.h"

/**
 * Recopies the config variable into the Filename for returning its value.
 */
void ConfigVariableFilename::
reload_cache() {
  // NB. MSVC doesn't guarantee that this mutex is initialized in a
  // thread-safe manner.  But chances are that the first time this is called
  // is at static init time, when there is no risk of data races.
  static MutexImpl lock;
  lock.lock();

  // We check again for cache validity since another thread may have beaten
  // us to the punch while we were waiting for the lock.
  if (!is_cache_valid(_local_modified)) {
    nassertv(_core != nullptr);
    const ConfigDeclaration *decl = _core->get_declaration(0);

    _cache = decl->get_filename_value();
    mark_cache_valid(_local_modified);
  }
  lock.unlock();
}
