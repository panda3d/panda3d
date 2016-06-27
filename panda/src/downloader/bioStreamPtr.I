/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bioStreamPtr.I
 * @author drose
 * @date 2002-10-15
 */

/**
 *
 */
INLINE BioStreamPtr::
BioStreamPtr(BioStream *stream) : _stream(stream) {
}

/**
 *
 */
INLINE BioStream &BioStreamPtr::
operator *() const {
  return *_stream;
}

/**
 *
 */
INLINE BioStream *BioStreamPtr::
operator ->() const {
  return _stream;
}

/**
 *
 */
INLINE BioStreamPtr::
operator BioStream * () const {
  return _stream;
}

/**
 *
 */
INLINE void BioStreamPtr::
set_stream(BioStream *stream) {
  _stream = stream;
}

/**
 *
 */
INLINE BioStream *BioStreamPtr::
get_stream() const {
  return _stream;
}
