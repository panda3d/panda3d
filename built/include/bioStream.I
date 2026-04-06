/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bioStream.I
 * @author drose
 * @date 2002-09-25
 */

/**
 *
 */
INLINE IBioStream::
IBioStream() : ISocketStream(&_buf) {
}

/**
 *
 */
INLINE IBioStream::
IBioStream(BioPtr *source) : ISocketStream(&_buf) {
  open(source);
}

/**
 *
 */
INLINE IBioStream &IBioStream::
open(BioPtr *source) {
  clear((ios_iostate)0);
  _buf.open(source);
  return *this;
}

/**
 *
 */
INLINE OBioStream::
OBioStream() : OSocketStream(&_buf) {
}

/**
 *
 */
INLINE OBioStream::
OBioStream(BioPtr *source) : OSocketStream(&_buf) {
  open(source);
}

/**
 *
 */
INLINE OBioStream &OBioStream::
open(BioPtr *source) {
  clear((ios_iostate)0);
  _buf.open(source);
  return *this;
}

/**
 *
 */
INLINE BioStream::
BioStream() : SocketStream(&_buf) {
}

/**
 *
 */
INLINE BioStream::
BioStream(BioPtr *source) : SocketStream(&_buf) {
  open(source);
}

/**
 *
 */
INLINE BioStream &BioStream::
open(BioPtr *source) {
  clear((ios_iostate)0);
  _buf.open(source);
  return *this;
}
