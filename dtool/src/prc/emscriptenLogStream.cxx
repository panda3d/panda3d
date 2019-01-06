/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file emscriptenLogStream.cxx
 * @author rdb
 * @date 2015-04-02
 */

#include "emscriptenLogStream.h"
#include "configVariableString.h"

#ifdef __EMSCRIPTEN__

#include <emscripten.h>

/**
 *
 */
EmscriptenLogStream::EmscriptenLogStreamBuf::
EmscriptenLogStreamBuf(int flags) :
  _flags(flags) {

  // The EmscriptenLogStreamBuf doesn't actually need a buffer--it's happy
  // writing characters one at a time, since they're just getting stuffed into
  // a string.  (Although the code is written portably enough to use a buffer
  // correctly, if we had one.)
  setg(0, 0, 0);
  setp(0, 0);
}

/**
 *
 */
EmscriptenLogStream::EmscriptenLogStreamBuf::
~EmscriptenLogStreamBuf() {
  sync();
}

/**
 * Called by the system ostream implementation when the buffer should be
 * flushed to output (for instance, on destruction).
 */
int EmscriptenLogStream::EmscriptenLogStreamBuf::
sync() {
  std::streamsize n = pptr() - pbase();

  // Write the characters that remain in the buffer.
  for (char *p = pbase(); p < pptr(); ++p) {
    write_char(*p);
  }

  pbump(-n);  // Reset pptr().
  return 0;  // EOF to indicate write full.
}

/**
 * Called by the system ostream implementation when its internal buffer is
 * filled, plus one character.
 */
int EmscriptenLogStream::EmscriptenLogStreamBuf::
overflow(int ch) {
  std::streamsize n = pptr() - pbase();

  if (n != 0 && sync() != 0) {
    return EOF;
  }

  if (ch != EOF) {
    // Write one more character.
    write_char(ch);
  }

  return 0;
}

/**
 * Stores a single character.
 */
void EmscriptenLogStream::EmscriptenLogStreamBuf::
write_char(char c) {
  if (c == '\n') {
    // Write a line to the log file.
    emscripten_log(_flags, "%.*s", _data.size(), _data.c_str());
    _data.clear();
  } else {
    _data += c;
  }
}

/**
 *
 */
EmscriptenLogStream::
EmscriptenLogStream(int flags) :
  std::ostream(new EmscriptenLogStreamBuf(flags)) {
}

/**
 *
 */
EmscriptenLogStream::
~EmscriptenLogStream() {
  delete rdbuf();
}

#endif  // __EMSCRIPTEN__
