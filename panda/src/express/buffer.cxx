/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file buffer.cxx
 * @author mike
 * @date 1997-01-09
 */

#include "buffer.h"

/**

 */
Buffer::
Buffer(int size) {
  _length = size;
  _buffer = (char *)PANDA_MALLOC_ARRAY(_length);
}

/**

 */
Buffer::
~Buffer() {
  PANDA_FREE_ARRAY(_buffer);
}
