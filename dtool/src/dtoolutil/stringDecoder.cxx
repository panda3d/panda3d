/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stringDecoder.cxx
 * @author drose
 * @date 2002-02-11
 */

#include "stringDecoder.h"
#include "config_dtoolutil.h"

std::ostream *StringDecoder::_notify_ptr = &std::cerr;

/**
 *
 */
StringDecoder::
~StringDecoder() {
}

/**
 * Returns the next character in sequence.
 */
char32_t StringDecoder::
get_next_character() {
  if (test_eof()) {
    return -1;
  }
  return (unsigned char)_input[_p++];
}

/**
 * Sets the ostream that is used to write error messages to.  This is
 * necessary because of the low-level placement of this class, before the
 * definition of the NotifyCategory class, so it cannot specify its own
 * notify.
 */
void StringDecoder::
set_notify_ptr(std::ostream *notify_ptr) {
  _notify_ptr = notify_ptr;
}

/**
 * Returns the ostream that is used to write error messages to.  See
 * set_notify_ptr().
 */
std::ostream *StringDecoder::
get_notify_ptr() {
  return _notify_ptr;
}


/*
In UTF-8, each 16-bit Unicode character is encoded as a sequence of
one, two, three or four 8-bit bytes, depending on the value of the
character. The following table shows the format of such UTF-8 byte
sequences (where the "free bits" shown by x's in the table are
combined in the order shown, and interpreted from most significant to
least significant):

 Binary format of bytes in sequence:
                                              Number of    Maximum expressible
 1st byte    2nd byte   3rd byte   4th byte   free bits:     Unicode value:

 0xxxxxxx                                         7          007F hex   (127)
 110xxxxx    10xxxxxx                          (5+6)=11      07FF hex  (2047)
 1110xxxx    10xxxxxx   10xxxxxx              (4+6+6)=16     FFFF hex (65535)
 11110xxx    10xxxxxx   10xxxxxx   10xxxxxx   (4+6*3)=21   10FFFF hex (1114111)

The value of each individual byte indicates its UTF-8 function, as follows:

 00 to 7F hex   (0 to 127):  first and only byte of a sequence.
 80 to BF hex (128 to 191):  continuing byte in a multi-byte sequence.
 C2 to DF hex (194 to 223):  first byte of a two-byte sequence.
 E0 to EF hex (224 to 239):  first byte of a three-byte sequence.
 F0 to F7 hex (240 to 247):  first byte of a four-byte sequence.
*/

/**
 * Returns the next character in sequence.
 */
char32_t StringUtf8Decoder::
get_next_character() {
  unsigned int result;
  while (!test_eof()) {
    result = (unsigned char)_input[_p++];
    if ((result & 0x80) == 0) {
      // A 7-bit ascii value in one byte.
      return result;

    } if ((result & 0xe0) == 0xc0) {
      // First byte of two.
      unsigned int two = 0;
      if (test_eof()) {
        if (_notify_ptr != nullptr) {
          (*_notify_ptr)
            << "utf-8 encoded string '" << _input << "' ends abruptly.\n";
        }
        return -1;
      }
      two = (unsigned char)_input[_p++];
      result = ((result & 0x1f) << 6) | (two & 0x3f);
      return result;

    } else if ((result & 0xf0) == 0xe0) {
      // First byte of three.
      if (test_eof()) {
        if (_notify_ptr != nullptr) {
          (*_notify_ptr)
            << "utf-8 encoded string '" << _input << "' ends abruptly.\n";
        }
        return -1;
      }
      unsigned int two = (unsigned char)_input[_p++];
      if (test_eof()) {
        if (_notify_ptr != nullptr) {
          (*_notify_ptr)
            << "utf-8 encoded string '" << _input << "' ends abruptly.\n";
        }
        return -1;
      }
      unsigned int three = (unsigned char)_input[_p++];
      result = ((result & 0x0f) << 12) | ((two & 0x3f) << 6) | (three & 0x3f);
      return result;

    } else if ((result & 0xf8) == 0xf0) {
      // First byte of four.
      if (test_eof()) {
        if (_notify_ptr != nullptr) {
          (*_notify_ptr)
            << "utf-8 encoded string '" << _input << "' ends abruptly.\n";
        }
        return -1;
      }
      unsigned int two = (unsigned char)_input[_p++];
      if (test_eof()) {
        if (_notify_ptr != nullptr) {
          (*_notify_ptr)
            << "utf-8 encoded string '" << _input << "' ends abruptly.\n";
        }
        return -1;
      }
      unsigned int three = (unsigned char)_input[_p++];
      if (test_eof()) {
        if (_notify_ptr != nullptr) {
          (*_notify_ptr)
            << "utf-8 encoded string '" << _input << "' ends abruptly.\n";
        }
        return -1;
      }
      unsigned int four = (unsigned char)_input[_p++];
      result = ((result & 0x07) << 18) | ((two & 0x3f) << 12) | ((three & 0x3f) << 6) | (four & 0x3f);
      return result;
    }

    // Otherwise--the high bit is set but it is not one of the introductory
    // utf-8 bytes--we have an error.
    if (_notify_ptr != nullptr) {
      (*_notify_ptr)
        << "Non utf-8 byte in string: 0x" << std::hex << result << std::dec
        << ", string is '" << _input << "'\n";
    }
    return -1;
  }

  // End of string reached.
  return -1;
}

/**
 * Returns the next character in sequence.
 */
char32_t StringUtf16Decoder::
get_next_character() {
  if (test_eof()) {
    return -1;
  }

  unsigned int high = (unsigned char)_input[_p++];
  if (test_eof()) {
    if (_notify_ptr != nullptr) {
      (*_notify_ptr)
        << "Unicode-encoded string has odd number of bytes.\n";
    }
    return -1;
  }
  unsigned int low = (unsigned char)_input[_p++];
  int ch = ((high << 8) | low);

  /*
  using std::swap;

  if (ch == 0xfffe) {
    // This is a byte-swapped byte-order-marker.  That means we need to swap
    // the endianness of the rest of the stream.
    char *data = (char *)_input.data();
    for (size_t p = _p; p < _input.size() - 1; p += 2) {
      std::swap(data[p], data[p + 1]);
    }
    ch = 0xfeff;
  }
  */

  if (ch >= 0xd800 && ch < 0xdc00 && (_p + 1) < _input.size()) {
    // This is a high surrogate.  Look for a subsequent low surrogate.
    unsigned int high = (unsigned char)_input[_p];
    unsigned int low = (unsigned char)_input[_p + 1];
    int ch2 = ((high << 8) | low);
    if (ch2 >= 0xdc00 && ch2 < 0xe000) {
      // Yes, this is a low surrogate.
      _p += 2;
      return 0x10000 + ((ch - 0xd800) << 10) + (ch2 - 0xdc00);
    }
  }
  // No, this is just a regular character, or an unpaired surrogate.
  return ch;
}
