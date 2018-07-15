/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pandaIOStream.cxx
 * @author rdb
 * @date 2011-03-29
 */

#include "pandaIOStream.h"

using std::ios;

/**
 *
 */
PandaIOStream::
PandaIOStream(std::istream &stream) : _istream(stream) {
}

/**
 * Returns the size of this file.
 */
size_t PandaIOStream::
FileSize() const {
  std::streampos cur = _istream.tellg();
  _istream.seekg(0, ios::end);
  std::streampos end = _istream.tellg();
  _istream.seekg(cur, ios::beg);
  return end;
}

/**
 * See fflush.
 */
void PandaIOStream::
Flush() {
  nassertv(false);
}

/**
 * See fread.
 */
size_t PandaIOStream::
Read(void *buffer, size_t size, size_t count) {
  _istream.read((char*) buffer, size * count);
  return _istream.gcount();
}

/**
 * See fseek.
 */
aiReturn PandaIOStream::
Seek(size_t offset, aiOrigin origin) {
  switch (origin) {
  case aiOrigin_SET:
    _istream.seekg(offset, ios::beg);
    break;

  case aiOrigin_CUR:
    _istream.seekg(offset, ios::cur);
    break;

  case aiOrigin_END:
    _istream.seekg(offset, ios::end);
    break;

  default:
    // Keep compiler happy
    nassertr(false, AI_FAILURE);
    break;
  }

  if (_istream.good()) {
    return AI_SUCCESS;
  } else {
    return AI_FAILURE;
  }
}

/**
 * See ftell.
 */
size_t PandaIOStream::
Tell() const {
  return _istream.tellg();
}

/**
 * See fwrite.
 */
size_t PandaIOStream::
Write(const void *buffer, size_t size, size_t count) {
  nassertr(false, 0);
  return 0;
}
