/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file subStream.I
 * @author drose
 * @date 2002-08-02
 */

/**
 *
 */
INLINE ISubStream::
ISubStream() : std::istream(&_buf) {
}

/**
 *
 */
INLINE ISubStream::
ISubStream(IStreamWrapper *source, std::streampos start, std::streampos end) : std::istream(&_buf) {
  open(source, start, end);
}

/**
 * Starts the SubStream reading from the indicated source, with the first
 * character being the character at position "start" within the source, for
 * end - start total characters.  The character at "end" within the source
 * will never be read; this will appear to be EOF.
 *
 * If end is zero, it indicates that the ISubStream will continue until the
 * end of the source stream.
 */
INLINE ISubStream &ISubStream::
open(IStreamWrapper *source, std::streampos start, std::streampos end) {
  clear((ios_iostate)0);
  _buf.open(source, nullptr, start, end, false);
  return *this;
}

/**
 * Resets the SubStream to empty, but does not actually close the source
 * istream.
 */
INLINE ISubStream &ISubStream::
close() {
  _buf.close();
  return *this;
}

/**
 *
 */
INLINE OSubStream::
OSubStream() : std::ostream(&_buf) {
}

/**
 *
 */
INLINE OSubStream::
OSubStream(OStreamWrapper *dest, std::streampos start, std::streampos end, bool append) : std::ostream(&_buf) {
  open(dest, start, end, append);
}

/**
 * Starts the SubStream reading from the indicated dest, with the first
 * character being the character at position "start" within the dest, for end
 * - start total characters.  The character at "end" within the dest will
 * never be read; this will appear to be EOF.
 *
 * If end is zero, it indicates that the OSubStream will continue until the
 * end of the dest stream.
 */
INLINE OSubStream &OSubStream::
open(OStreamWrapper *dest, std::streampos start, std::streampos end, bool append) {
  clear((ios_iostate)0);
  _buf.open(nullptr, dest, start, end, append);
  return *this;
}

/**
 * Resets the SubStream to empty, but does not actually close the dest
 * ostream.
 */
INLINE OSubStream &OSubStream::
close() {
  _buf.close();
  return *this;
}

/**
 *
 */
INLINE SubStream::
SubStream() : std::iostream(&_buf) {
}

/**
 *
 */
INLINE SubStream::
SubStream(StreamWrapper *nested, std::streampos start, std::streampos end, bool append) : std::iostream(&_buf) {
  open(nested, start, end, append);
}

/**
 * Starts the SubStream reading and writing from the indicated nested stream,
 * within the indicated range.  "end" is the first character outside of the
 * range.
 *
 * If end is zero, it indicates that the SubStream will continue until the end
 * of the nested stream.
 */
INLINE SubStream &SubStream::
open(StreamWrapper *nested, std::streampos start, std::streampos end, bool append) {
  clear((ios_iostate)0);
  _buf.open(nested, nested, start, end, append);
  return *this;
}

/**
 * Resets the SubStream to empty, but does not actually close the nested
 * ostream.
 */
INLINE SubStream &SubStream::
close() {
  _buf.close();
  return *this;
}
