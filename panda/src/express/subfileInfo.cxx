/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file subfileInfo.cxx
 * @author drose
 * @date 2011-06-20
 */

#include "subfileInfo.h"

/**
 *
 */
void SubfileInfo::
output(std::ostream &out) const {
  out << "SubfileInfo(" << get_filename() << ", " << _start << ", " << _size << ")";
}
