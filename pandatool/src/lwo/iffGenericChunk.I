/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file iffGenericChunk.I
 * @author drose
 * @date 2001-04-23
 */

/**
 *
 */
INLINE IffGenericChunk::
IffGenericChunk() {
}

/**
 * Returns the data in the chunk.
 */
INLINE const Datagram &IffGenericChunk::
get_data() const {
  return _data;
}

/**
 * Changes the data in the chunk
 */
INLINE void IffGenericChunk::
set_data(const Datagram &data) {
  _data = data;
}
