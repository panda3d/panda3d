/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textEncoder.h
 * @author drose
 * @date 2003-03-26
 */

#ifndef TEXTENCODER_H
#define TEXTENCODER_H

#include "dtoolbase.h"
#include "unicodeLatinMap.h"

#include <ctype.h>

class StringDecoder;

/**
 * This class can be used to convert text between multiple representations,
 * e.g.  UTF-8 to UTF-16.  You may use it as a static class object, passing
 * the encoding each time, or you may create an instance and use that object,
 * which will record the current encoding and retain the current string.
 *
 * This class is also a base class of TextNode, which inherits this
 * functionality.
 */
class EXPCL_DTOOL_DTOOLUTIL TextEncoder {
PUBLISHED:
  enum Encoding {
    E_iso8859,
    E_utf8,
    E_utf16be,

    // Deprecated alias for E_utf16be
    E_unicode = E_utf16be,
  };

  INLINE TextEncoder();
  INLINE TextEncoder(const TextEncoder &copy);

  virtual ~TextEncoder() = default;

  INLINE void set_encoding(Encoding encoding);
  INLINE Encoding get_encoding() const;

  INLINE static void set_default_encoding(Encoding encoding);
  INLINE static Encoding get_default_encoding();
  MAKE_PROPERTY(default_encoding, get_default_encoding, set_default_encoding);

#ifdef CPPPARSER
  EXTEND void set_text(PyObject *text);
  EXTEND void set_text(PyObject *text, Encoding encoding);
#else
  INLINE void set_text(const std::string &text);
  INLINE void set_text(const std::string &text, Encoding encoding);
#endif
  INLINE void clear_text();
  INLINE bool has_text() const;

  void make_upper();
  void make_lower();

#ifdef CPPPARSER
  EXTEND PyObject *get_text() const;
  EXTEND PyObject *get_text(Encoding encoding) const;
  EXTEND void append_text(PyObject *text);
#else
  INLINE std::string get_text() const;
  INLINE std::string get_text(Encoding encoding) const;
  INLINE void append_text(const std::string &text);
#endif
  INLINE void append_unicode_char(char32_t character);
  INLINE size_t get_num_chars() const;
  INLINE int get_unicode_char(size_t index) const;
  INLINE void set_unicode_char(size_t index, char32_t character);
  INLINE std::string get_encoded_char(size_t index) const;
  INLINE std::string get_encoded_char(size_t index, Encoding encoding) const;
  INLINE std::string get_text_as_ascii() const;

  INLINE static std::string reencode_text(const std::string &text, Encoding from, Encoding to);

  INLINE static bool unicode_isalpha(char32_t character);
  INLINE static bool unicode_isdigit(char32_t character);
  INLINE static bool unicode_ispunct(char32_t character);
  INLINE static bool unicode_islower(char32_t character);
  INLINE static bool unicode_isupper(char32_t character);
  INLINE static bool unicode_isspace(char32_t character);
  INLINE static int unicode_toupper(char32_t character);
  INLINE static int unicode_tolower(char32_t character);

  INLINE static std::string upper(const std::string &source);
  INLINE static std::string upper(const std::string &source, Encoding encoding);
  INLINE static std::string lower(const std::string &source);
  INLINE static std::string lower(const std::string &source, Encoding encoding);

  // Direct support for wide-character strings.  Now publishable with the new
  // wstring support in interrogate.
  INLINE void set_wtext(const std::wstring &wtext);
  INLINE const std::wstring &get_wtext() const;
  INLINE void append_wtext(const std::wstring &text);
  std::wstring get_wtext_as_ascii() const;
  bool is_wtext() const;

#ifdef CPPPARSER
  EXTEND static PyObject *encode_wchar(char32_t ch, Encoding encoding);
  EXTEND INLINE PyObject *encode_wtext(const std::wstring &wtext) const;
  EXTEND static PyObject *encode_wtext(const std::wstring &wtext, Encoding encoding);
  EXTEND INLINE PyObject *decode_text(PyObject *text) const;
  EXTEND static PyObject *decode_text(PyObject *text, Encoding encoding);
#else
  static std::string encode_wchar(char32_t ch, Encoding encoding);
  INLINE std::string encode_wtext(const std::wstring &wtext) const;
  static std::string encode_wtext(const std::wstring &wtext, Encoding encoding);
  INLINE std::wstring decode_text(const std::string &text) const;
  static std::wstring decode_text(const std::string &text, Encoding encoding);
#endif

  MAKE_PROPERTY(text, get_text, set_text);

protected:
  virtual void text_changed();

private:
  enum Flags {
    F_got_text         =  0x0001,
    F_got_wtext        =  0x0002,
  };
  static std::wstring decode_text_impl(StringDecoder &decoder);

  int _flags;
  Encoding _encoding;
  std::string _text;
  std::wstring _wtext;

  static Encoding _default_encoding;
};

EXPCL_DTOOL_DTOOLUTIL std::ostream &
operator << (std::ostream &out, TextEncoder::Encoding encoding);
EXPCL_DTOOL_DTOOLUTIL std::istream &
operator >> (std::istream &in, TextEncoder::Encoding &encoding);

// We'll define the output operator for wstring here, too.  Presumably this
// will not be automatically defined by any system libraries.

// This function is declared inline to minimize the risk of link conflicts
// should another third-party module also define the same output operator.
INLINE EXPCL_DTOOL_DTOOLUTIL std::ostream &
operator << (std::ostream &out, const std::wstring &str);

#include "textEncoder.I"

#endif
