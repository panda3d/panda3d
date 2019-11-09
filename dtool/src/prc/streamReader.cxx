/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file streamReader.cxx
 * @author drose
 * @date 2002-08-04
 */

#include "streamReader.h"
#include "memoryHook.h"

using std::string;


/**
 * Extracts a variable-length string.
 */
string StreamReader::
get_string() {
  nassertr(!_in->eof() && !_in->fail(), string());

  // First, get the length of the string
  size_t size = get_uint16();

  std::string result(size, 0);
  if (size == 0) {
    return result;
  }

  _in->read(&result[0], size);
  size_t read_bytes = _in->gcount();

  if (read_bytes == size) {
    return result;
  } else {
    return result.substr(0, read_bytes);
  }
}

/**
 * Extracts a variable-length string with a 32-bit length field.
 */
string StreamReader::
get_string32() {
  nassertr(!_in->eof() && !_in->fail(), string());

  // First, get the length of the string
  size_t size = get_uint32();
  if (size == 0) {
    return string();
  }

  char *buffer = (char *)PANDA_MALLOC_ARRAY(size);
  _in->read(buffer, size);
  size_t read_bytes = _in->gcount();
  string result(buffer, read_bytes);
  PANDA_FREE_ARRAY(buffer);
  return result;
}

/**
 * Extracts a variable-length string, as a NULL-terminated string.
 */
string StreamReader::
get_z_string() {
  nassertr(!_in->eof() && !_in->fail(), string());

  string result;
  int ch = _in->get();
  while (!_in->fail() && ch != EOF && ch != '\0') {
    result += (char)ch;
    ch = _in->get();
  }

  return result;
}

/**
 * Extracts a fixed-length string.  However, if a zero byte occurs within the
 * string, it marks the end of the string.
 */
string StreamReader::
get_fixed_string(size_t size) {
  nassertr(!_in->eof() && !_in->fail(), string());

  std::string result(size, 0);
  if (size == 0) {
    return result;
  }

  _in->read(&result[0], size);
  size_t read_bytes = _in->gcount();
  result.resize(read_bytes);

  size_t zero_byte = result.find('\0');
  return result.substr(0, std::min(zero_byte, read_bytes));
}

/**
 * Skips over the indicated number of bytes in the stream.
 */
void StreamReader::
skip_bytes(size_t size) {
  nassertv(!_in->fail());
  nassertv((int)size >= 0);
  nassertv(size == 0 || !_in->eof());

  while (size > 0) {
    _in->get();
    size--;
  }
}

/**
 * Extracts the indicated number of bytes in the stream into the given
 * character buffer.  Assumes that the buffer is big enough to hold the
 * requested number of bytes.  Returns the number of bytes that were
 * successfully written.
 */
size_t StreamReader::
extract_bytes(unsigned char *into, size_t size) {
  if (_in->eof() || _in->fail()) {
    return 0;
  }

  _in->read((char *)into, size);
  return _in->gcount();
}

/**
 * Extracts the indicated number of bytes in the stream and returns them as a
 * string.  Returns empty string at end-of-file.
 */
vector_uchar StreamReader::
extract_bytes(size_t size) {
  vector_uchar buffer;
  if (_in->eof() || _in->fail()) {
    return buffer;
  }

  buffer.resize(size);
  _in->read((char *)&buffer[0], size);
  size_t read_bytes = _in->gcount();
  buffer.resize(read_bytes);
  return buffer;
}

/**
 * Assumes the stream represents a text file, and extracts one line up to and
 * including the trailing newline character.  Returns empty string when the
 * end of file is reached.
 *
 * The interface here is intentionally designed to be similar to that for
 * Python's File.readline() function.
 */
string StreamReader::
readline() {
  string line;
  int ch = _in->get();
  while (ch != EOF && !_in->fail()) {
    line += (char)ch;
    if (ch == '\n' || _in->eof()) {
      // Here's the newline character.
      return line;
    }
    ch = _in->get();
  }

  return line;
}
