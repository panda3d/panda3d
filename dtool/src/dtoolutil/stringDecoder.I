/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stringDecoder.I
 * @author drose
 * @date 2002-02-11
 */

/**
 *
 */
INLINE StringDecoder::
StringDecoder(const std::string &input) : _input(input) {
  _p = 0;
  _eof = false;
}

/**
 * Returns true if the decoder has returned the last character in the string,
 * false if there are more to go.
 */
INLINE bool StringDecoder::
is_eof() {
  return _eof;
}

/**
 * If the pointer is past the last character of the string, set the eof flag
 * and return true.
 */
INLINE bool StringDecoder::
test_eof() {
  if (_p >= _input.size()) {
    _eof = true;
    return true;
  }
  return false;
}

/**
 *
 */
INLINE StringUtf8Decoder::
StringUtf8Decoder(const std::string &input) : StringDecoder(input) {
}

/**
 *
 */
INLINE StringUnicodeDecoder::
StringUtf16Decoder(const std::string &input) : StringDecoder(input) {
}
