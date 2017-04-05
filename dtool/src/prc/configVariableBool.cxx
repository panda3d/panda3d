/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configVariableBool.cxx
 * @author drose
 * @date 2004-10-20
 */

#include "configVariableBool.h"

/**
 * Refreshes the cached value.
 */
void ConfigVariableBool::
reload_value() const {
  mark_cache_valid(_local_modified);
  _cache = get_bool_word(0);
}
