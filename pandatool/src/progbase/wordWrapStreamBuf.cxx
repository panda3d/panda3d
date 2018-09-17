/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file wordWrapStreamBuf.cxx
 * @author drose
 * @date 2000-06-28
 */

#include "wordWrapStreamBuf.h"
#include "wordWrapStream.h"
#include "programBase.h"

#include "pnotify.h"

/**
 *
 */
WordWrapStreamBuf::
WordWrapStreamBuf(WordWrapStream *owner, ProgramBase *program) :
  _owner(owner),
  _program(program)
{
  _literal_mode = false;
}

/**
 *
 */
WordWrapStreamBuf::
~WordWrapStreamBuf() {
  sync();
}

/**
 * Called by the system ostream implementation when the buffer should be
 * flushed to output (for instance, on destruction).
 */
int WordWrapStreamBuf::
sync() {
  std::streamsize n = pptr() - pbase();
  write_chars(pbase(), n);

  // Send all the data out now.
  flush_data();

  return 0;  // EOF to indicate write full.
}

/**
 * Called by the system ostream implementation when its internal buffer is
 * filled, plus one character.
 */
int WordWrapStreamBuf::
overflow(int ch) {
  std::streamsize n = pptr() - pbase();

  if (n != 0 && sync() != 0) {
    return EOF;
  }

  if (ch != EOF) {
    // Write one more character.
    char c = ch;
    write_chars(&c, 1);
  }

  pbump(-n);  // Reset pptr().
  return 0;
}

/**
 * An internal function called by sync() and overflow() to store one or more
 * characters written to the stream into the memory buffer.
 */
void WordWrapStreamBuf::
write_chars(const char *start, int length) {
  if (length > 0) {
    set_literal_mode((_owner->flags() & Notify::get_literal_flag()) != 0);
    std::string new_data(start, length);
    size_t newline = new_data.find_first_of("\n\r");
    size_t p = 0;
    while (newline != std::string::npos) {
      // The new data contains a newline; flush our data to that point.
      _data += new_data.substr(p, newline - p + 1);
      flush_data();
      p = newline + 1;
      newline = new_data.find_first_of("\n\r", p);
    }

    // Save the rest for the next write.
    _data += new_data.substr(p);
  }
}

/**
 * Writes the contents of _data to the actual output stream, either word-
 * wrapped or not as appropriate, and empties the contents of _data.
 */
void WordWrapStreamBuf::
flush_data() {
  if (!_data.empty()) {
    if (_literal_mode) {
      std::cerr << _data;
    } else {
      _program->show_text(_data);
    }
    _data = "";
  }
}
