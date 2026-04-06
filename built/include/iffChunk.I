/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file iffChunk.I
 * @author drose
 * @date 2001-04-23
 */

/**
 *
 */
INLINE IffChunk::
IffChunk() {
}

/**
 * Returns the ID associated with this chunk.
 */
INLINE IffId IffChunk::
get_id() const {
  return _id;
}

/**
 * Changes the ID associated with this chunk.
 */
INLINE void IffChunk::
set_id(IffId id) {
  _id = id;
}
