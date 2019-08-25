/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file encryptStream.cxx
 * @author drose
 * @date 2004-09-01
 */

#include "encryptStream.h"

#ifdef HAVE_OPENSSL

/**
 * Must be called immediately after open_read().  Decrypts the given number of
 * bytes and checks that they match.  The amount of header bytes are added to
 * an offset so that skipping to 0 will skip past the header.
 *
 * Returns true if the read magic matches the given magic, false on error.
 */
bool IDecryptStream::
read_magic(const char *magic, size_t size) {
  char *this_magic = (char *)alloca(size);
  read(this_magic, size);

  if (!fail() && (size_t)gcount() == size && memcmp(this_magic, magic, size) == 0) {
    _buf.set_magic_length(size);
    return true;
  } else {
    return false;
  }
}

#endif
