/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableString.cxx
 * @author drose
 * @date 2004-10-20
 */

#include "configVariableString.h"

/**
 * Refreshes the variable's cached value.
 */
void ConfigVariableString::
reload_cache() {
  // NB. MSVC doesn't guarantee that this mutex is initialized in a
  // thread-safe manner.  But chances are that the first time this is called
  // is at static init time, when there is no risk of data races.
  static MutexImpl lock;
  lock.acquire();

  // We check again for cache validity since another thread may have beaten
  // us to the punch while we were waiting for the lock.
  if (!is_cache_valid(_local_modified)) {
    _cache = get_string_value();
    mark_cache_valid(_local_modified);
  }

  lock.release();
}
