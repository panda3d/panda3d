/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stringStreamBuf.cxx
 * @author drose
 * @date 2007-07-02
 */

#include "stringStreamBuf.h"
#include "pnotify.h"
#include "config_express.h"

using std::ios;
using std::streamoff;
using std::streampos;

/**
 *
 */
StringStreamBuf::
StringStreamBuf() {
#ifdef PHAVE_IOSTREAM
  _buffer = (char *)PANDA_MALLOC_ARRAY(2048);
  char *ebuf = _buffer + 2048;
  char *mbuf = _buffer + 1024;
  setg(_buffer, mbuf, mbuf);
  setp(mbuf, ebuf);

#else
  allocate();
  // Chop the buffer in half.  The bottom half goes to the get buffer; the top
  // half goes to the put buffer.
  char *b = base();
  char *t = ebuf();
  char *m = b + (t - b) / 2;
  setg(b, m, m);
  setp(b, m);
#endif

  _gpos = 0;
  _ppos = 0;
}

/**
 *
 */
StringStreamBuf::
~StringStreamBuf() {
#ifdef PHAVE_IOSTREAM
  PANDA_FREE_ARRAY(_buffer);
#endif
}

/**
 * Empties the buffer.
 */
void StringStreamBuf::
clear() {
  _data.clear();
  _gpos = 0;
  _ppos = 0;

  pbump(pbase() - pptr());
  gbump(egptr() - gptr());
}

/**
 * Attempts to extract the indicated number of characters from the current
 * file position.  Returns the number of characters extracted.
 */
size_t StringStreamBuf::
read_chars(char *start, size_t length) {
  if (length == 0) {
    return 0;
  }

  // Make sure the write buffer is flushed.
  sync();

  if (_data.size() <= _gpos) {
    return 0;
  }

  length = std::min(length, _data.size() - _gpos);
  memcpy(start, &_data[_gpos], length);
  _gpos += length;
  return length;
}

/**
 * Appends the indicated stream of characters to the current file position.
 */
void StringStreamBuf::
write_chars(const char *start, size_t length) {
  if (length != 0) {
    // Make sure the read buffer is flushed.
    size_t n = egptr() - gptr();
    gbump(n);
    _gpos -= n;

    if (_data.size() > _ppos) {
      // We are overwriting some data.
      size_t remaining_length = _data.size() - _ppos;
      size_t overwrite_length = std::min(remaining_length, length);
      memcpy(&_data[_ppos], start, overwrite_length);
      length -= overwrite_length;
      _ppos += overwrite_length;
      start += overwrite_length;
    }

    if (_data.size() < _ppos) {
      // We need to append some zeroes.
      _data.insert(_data.end(), _ppos - _data.size(), (unsigned char)0);
    }

    if (length != 0) {
      // We are appending some data.
      _data.insert(_data.begin() + _ppos, (const unsigned char *)start, (const unsigned char *)start + length);
      _ppos += length;
    }
  }
}

/**
 * Implements seeking within the stream.
 */
streampos StringStreamBuf::
seekoff(streamoff off, ios_seekdir dir, ios_openmode which) {
  streampos result = -1;

  // Sync the iostream buffer first.
  sync();

  if (which & ios::in) {
    // Determine the current file position.
    size_t n = egptr() - gptr();
    gbump(n);
    _gpos -= n;
    size_t cur_pos = _gpos;
    size_t new_pos = cur_pos;

    // Now adjust the data pointer appropriately.
    switch (dir) {
    case ios::beg:
      new_pos = (size_t)off;
      break;

    case ios::cur:
      new_pos = (size_t)((int)cur_pos + off);
      break;

    case ios::end:
      new_pos = (size_t)((int)_data.size() + off);
      break;

    default:
      // Shouldn't get here.
      break;
    }

    _gpos = new_pos;
    result = new_pos;
  }

  if (which & ios::out) {
    // Determine the current file position.
    size_t n = pptr() - pbase();
    size_t cur_pos = _ppos + n;
    size_t new_pos = cur_pos;

    // Now adjust the data pointer appropriately.
    switch (dir) {
    case ios::beg:
      new_pos = (size_t)off;
      break;

    case ios::cur:
      new_pos = (size_t)((int)cur_pos + off);
      break;

    case ios::end:
      new_pos = (size_t)((int)_data.size() + off);
      break;

    default:
      // Shouldn't get here.
      break;
    }

    _ppos = new_pos;
    result = new_pos;
  }

  return result;
}

/**
 * A variant on seekoff() to implement seeking within a stream.
 *
 * The MSDN Library claims that it is only necessary to redefine seekoff(),
 * and not seekpos() as well, as the default implementation of seekpos() is
 * supposed to map to seekoff() exactly as I am doing here; but in fact it
 * must do something else, because seeking didn't work on Windows until I
 * redefined this function as well.
 */
streampos StringStreamBuf::
seekpos(streampos pos, ios_openmode which) {
  return seekoff(pos, ios::beg, which);
}

/**
 * Called by the system ostream implementation when its internal buffer is
 * filled, plus one character.
 */
int StringStreamBuf::
overflow(int ch) {
  size_t n = pptr() - pbase();
  if (n != 0) {
    write_chars(pbase(), n);
    pbump(-(int)n);
  }

  if (ch != EOF) {
    // Write one more character.
    char c = ch;
    write_chars(&c, 1);
  }

  return 0;
}

/**
 * Called by the system iostream implementation to implement a flush
 * operation.
 */
int StringStreamBuf::
sync() {
  size_t n = pptr() - pbase();

  write_chars(pbase(), n);
  pbump(-(int)n);

  return 0;
}

/**
 * Called by the system istream implementation when its internal buffer needs
 * more characters.
 */
int StringStreamBuf::
underflow() {
  // Sometimes underflow() is called even if the buffer is not empty.
  if (gptr() >= egptr()) {
    // Mark the buffer filled (with buffer_size bytes).
    size_t buffer_size = egptr() - eback();
    gbump(-(int)buffer_size);

    size_t num_bytes = buffer_size;
    size_t read_count = read_chars(gptr(), buffer_size);

    if (read_count != num_bytes) {
      // Oops, we didn't read what we thought we would.
      if (read_count == 0) {
        gbump(num_bytes);
        return EOF;
      }

      // Slide what we did read to the top of the buffer.
      nassertr(read_count < num_bytes, EOF);
      size_t delta = num_bytes - read_count;
      memmove(gptr() + delta, gptr(), read_count);
      gbump(delta);
    }
  }

  return (unsigned char)*gptr();
}
