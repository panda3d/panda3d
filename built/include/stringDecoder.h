/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stringDecoder.h
 * @author drose
 * @date 2002-02-11
 */

#ifndef STRINGDECODER_H
#define STRINGDECODER_H

#include "dtoolbase.h"

/**
 * The base class to a family of classes that decode various kinds of encoded
 * byte streams.  Give it a string, then ask it to pull the characters out one
 * at a time.  This also serves as the plain old byte-at-a-time decoder.
 */
class EXPCL_DTOOL_DTOOLUTIL StringDecoder {
public:
  INLINE StringDecoder(const std::string &input);
  virtual ~StringDecoder();

  virtual char32_t get_next_character();
  INLINE bool is_eof();

  static void set_notify_ptr(std::ostream *ptr);
  static std::ostream *get_notify_ptr();

protected:
  INLINE bool test_eof();

  std::string _input;
  size_t _p;
  bool _eof;
  static std::ostream *_notify_ptr;
};

/**
 * This decoder extracts utf-8 sequences.
 */
class StringUtf8Decoder : public StringDecoder {
public:
  INLINE StringUtf8Decoder(const std::string &input);

  virtual char32_t get_next_character();
};

/**
 * This decoder extracts characters two at a time to get a plain wide
 * character sequence.  It supports surrogate pairs.
 */
class StringUtf16Decoder : public StringDecoder {
public:
  INLINE StringUtf16Decoder(const std::string &input);

  virtual char32_t get_next_character();
};

// Deprecated alias of StringUtf16Encoder.
typedef StringUtf16Decoder StringUnicodeDecoder;

#include "stringDecoder.I"

#endif
