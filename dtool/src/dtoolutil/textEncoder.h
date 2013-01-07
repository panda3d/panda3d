// Filename: textEncoder.h
// Created by:  drose (26Mar03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef TEXTENCODER_H
#define TEXTENCODER_H

#include "dtoolbase.h"
#include "unicodeLatinMap.h"

#include <ctype.h>

class StringDecoder;

////////////////////////////////////////////////////////////////////
//       Class : TextEncoder
// Description : This class can be used to convert text between
//               multiple representations, e.g. utf-8 to Unicode.  You
//               may use it as a static class object, passing the
//               encoding each time, or you may create an instance and
//               use that object, which will record the current
//               encoding and retain the current string.
//
//               This class is also a base class of TextNode, which
//               inherits this functionality.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL TextEncoder {
PUBLISHED:
  enum Encoding {
    E_iso8859,
    E_utf8,
    E_unicode
  };

  INLINE TextEncoder();
  INLINE TextEncoder(const TextEncoder &copy);

  INLINE void set_encoding(Encoding encoding);
  INLINE Encoding get_encoding() const;

  INLINE static void set_default_encoding(Encoding encoding);
  INLINE static Encoding get_default_encoding();

  INLINE void set_text(const string &text);
  INLINE void set_text(const string &text, Encoding encoding);
  INLINE void clear_text();
  INLINE bool has_text() const;

  void make_upper();
  void make_lower();

  INLINE string get_text() const;
  INLINE string get_text(Encoding encoding) const;
  INLINE void append_text(const string &text);
  INLINE void append_unicode_char(int character);
  INLINE int get_num_chars() const;
  INLINE int get_unicode_char(int index) const;
  INLINE void set_unicode_char(int index, int character);
  INLINE string get_encoded_char(int index) const;
  INLINE string get_encoded_char(int index, Encoding encoding) const;
  INLINE string get_text_as_ascii() const;

  INLINE static string reencode_text(const string &text, Encoding from, Encoding to);

  INLINE static bool unicode_isalpha(int character);
  INLINE static bool unicode_isdigit(int character);
  INLINE static bool unicode_ispunct(int character);
  INLINE static bool unicode_islower(int character);
  INLINE static bool unicode_isupper(int character);
  INLINE static bool unicode_isspace(int character);
  INLINE static int unicode_toupper(int character);
  INLINE static int unicode_tolower(int character);

  INLINE static string upper(const string &source);
  INLINE static string upper(const string &source, Encoding encoding);
  INLINE static string lower(const string &source);
  INLINE static string lower(const string &source, Encoding encoding);

  // Direct support for wide-character strings.  Now publishable with
  // the new wstring support in interrogate.
  INLINE void set_wtext(const wstring &wtext);
  INLINE const wstring &get_wtext() const;
  INLINE void append_wtext(const wstring &text);
  wstring get_wtext_as_ascii() const;
  bool is_wtext() const;

  static string encode_wchar(wchar_t ch, Encoding encoding);
  INLINE string encode_wtext(const wstring &wtext) const;
  static string encode_wtext(const wstring &wtext, Encoding encoding);
  INLINE wstring decode_text(const string &text) const;
  static wstring decode_text(const string &text, Encoding encoding);

private:
  enum Flags {
    F_got_text         =  0x0001,
    F_got_wtext        =  0x0002,
  };
  static wstring decode_text_impl(StringDecoder &decoder);

  int _flags;
  Encoding _encoding;
  string _text;
  wstring _wtext;

  static Encoding _default_encoding;
};

EXPCL_DTOOL ostream &
operator << (ostream &out, TextEncoder::Encoding encoding);
EXPCL_DTOOL istream &
operator >> (istream &in, TextEncoder::Encoding &encoding);

// We'll define the output operator for wstring here, too.  Presumably
// this will not be automatically defined by any system libraries.

// This function is declared inline to minimize the risk of link
// conflicts should another third-party module also define the same
// output operator.
INLINE EXPCL_DTOOL ostream &
operator << (ostream &out, const wstring &str);

#include "textEncoder.I"

#endif
