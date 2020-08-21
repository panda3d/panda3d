/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file zStream.I
 * @author drose
 * @date 2002-08-05
 */

/**
 *
 */
INLINE IDecompressStream::
IDecompressStream() : std::istream(&_buf) {
}

/**
 *
 */
INLINE IDecompressStream::
IDecompressStream(std::istream *source, bool owns_source) : std::istream(&_buf) {
  open(source, owns_source);
}

/**
 *
 */
INLINE IDecompressStream &IDecompressStream::
open(std::istream *source, bool owns_source) {
  clear((ios_iostate)0);
  _buf.open_read(source, owns_source);
  return *this;
}

/**
 * Resets the ZStream to empty, but does not actually close the source istream
 * unless owns_source was true.
 */
INLINE IDecompressStream &IDecompressStream::
close() {
  _buf.close_read();
  return *this;
}


/**
 *
 */
INLINE OCompressStream::
OCompressStream() : std::ostream(&_buf) {
}

/**
 *
 */
INLINE OCompressStream::
OCompressStream(std::ostream *dest, bool owns_dest, int compression_level) :
  std::ostream(&_buf)
{
  open(dest, owns_dest, compression_level);
}

/**
 *
 */
INLINE OCompressStream &OCompressStream::
open(std::ostream *dest, bool owns_dest, int compression_level) {
  clear((ios_iostate)0);
  _buf.open_write(dest, owns_dest, compression_level);
  return *this;
}

/**
 * Resets the ZStream to empty, but does not actually close the dest ostream
 * unless owns_dest was true.
 */
INLINE OCompressStream &OCompressStream::
close() {
  _buf.close_write();
  return *this;
}
