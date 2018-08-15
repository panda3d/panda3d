/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file samplerContext.cxx
 * @author rdb
 * @date 2014-12-11
 */

#include "samplerContext.h"

TypeHandle SamplerContext::_type_handle;

/**
 *
 */
void SamplerContext::
output(std::ostream &out) const {
  SavedContext::output(out);
}

/**
 *
 */
void SamplerContext::
write(std::ostream &out, int indent_level) const {
  SavedContext::write(out, indent_level);
}
