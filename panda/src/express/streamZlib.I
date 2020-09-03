/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file streamBase.I
 * @author drose
 * @date 2002-08-05
 */

/**
 *
 */
INLINE IDecompressStreamZlib::
IDecompressStreamZlib() : std::istream(&_buf) {
}

/**
 *
 */
INLINE IDecompressStreamZlib::
IDecompressStreamZlib(std::istream *source, bool owns_source) : std::istream(&_buf) {
  open(source, owns_source);
}

/**
 *
 */
INLINE IDecompressStreamZlib &IDecompressStreamZlib::
open(std::istream *source, bool owns_source) {
  clear((ios_iostate)0);
  _buf.open_read(source, owns_source);
  return *this;
}

/**
 * Resets the ZStream to empty, but does not actually close the source istream
 * unless owns_source was true.
 */
INLINE IDecompressStreamZlib &IDecompressStreamZlib::
close() {
  _buf.close_read();
  return *this;
}


/**
 *
 */
INLINE OCompressStreamZlib::
OCompressStreamZlib() : std::ostream(&_buf) {
}

/**
 *
 */
INLINE OCompressStreamZlib::
OCompressStreamZlib(std::ostream *dest, bool owns_dest, int compression_level) :
  std::ostream(&_buf)
{
  open(dest, owns_dest, compression_level);
}

/**
 *
 */
INLINE OCompressStreamZlib &OCompressStreamZlib::
open(std::ostream *dest, bool owns_dest, int compression_level) {
  clear((ios_iostate)0);
  _buf.open_write(dest, owns_dest, compression_level);
  return *this;
}

/**
 * Resets the ZStream to empty, but does not actually close the dest ostream
 * unless owns_dest was true.
 */
INLINE OCompressStreamZlib &OCompressStreamZlib::
close() {
  _buf.close_write();
  return *this;
}
