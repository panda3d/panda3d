/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stringStream.cxx
 * @author drose
 * @date 2007-07-03
 */

#include "stringStream.h"

/**
 * Replaces the contents of the data stream.  This implicitly reseeks to 0.
 */
void StringStream::
set_data(const unsigned char *data, size_t size) {
  _buf.clear();
  vector_uchar pv;
  pv.insert(pv.end(), data, data + size);
  _buf.swap_data(pv);
}
