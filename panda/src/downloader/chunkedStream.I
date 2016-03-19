/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file chunkedStream.I
 * @author drose
 * @date 2002-09-25
 */

/**
 *
 */
INLINE IChunkedStream::
IChunkedStream() : ISocketStream(&_buf) {
}

/**
 *
 */
INLINE IChunkedStream::
IChunkedStream(BioStreamPtr *source, HTTPChannel *doc) : ISocketStream(&_buf) {
  open(source, doc);
}

/**
 *
 */
INLINE IChunkedStream &IChunkedStream::
open(BioStreamPtr *source, HTTPChannel *doc) {
  clear((ios_iostate)0);
  _buf.open_read(source, doc);
  return *this;
}
