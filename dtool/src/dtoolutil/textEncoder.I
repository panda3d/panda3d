// Filename: textEncoder.I
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


////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TextEncoder::
TextEncoder() {
  _encoding = _default_encoding;
  
  // Initially, since the text string is empty, we know that both
  // _text and _wtext accurately reflect the empty state; so we "got"
  // both of them.
  _flags = (F_got_text | F_got_wtext);
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE TextEncoder::
TextEncoder(const TextEncoder &copy) :
  _flags(copy._flags),
  _encoding(copy._encoding),
  _text(copy._text),
  _wtext(copy._wtext)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::set_encoding
//       Access: Published
//  Description: Specifies how the string set via set_text() is to be
//               interpreted.  The default, E_iso8859, means a
//               standard string with one-byte characters
//               (i.e. ASCII).  Other encodings are possible to take
//               advantage of character sets with more than 256
//               characters.
//
//               This affects only future calls to set_text(); it does
//               not change text that was set previously.
////////////////////////////////////////////////////////////////////
INLINE void TextEncoder::
set_encoding(TextEncoder::Encoding encoding) {
  // Force the previously-set strings to be encoded or decoded now.
  get_text();
  get_wtext();
  _encoding = encoding;
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::get_encoding
//       Access: Published
//  Description: Returns the encoding by which the string set via
//               set_text() is to be interpreted.  See set_encoding().
////////////////////////////////////////////////////////////////////
INLINE TextEncoder::Encoding TextEncoder::
get_encoding() const {
  return _encoding;
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::set_default_encoding
//       Access: Published, Static
//  Description: Specifies the default encoding to be used for all
//               subsequently created TextEncoder objects.  See
//               set_encoding().
////////////////////////////////////////////////////////////////////
INLINE void TextEncoder::
set_default_encoding(TextEncoder::Encoding encoding) {
  _default_encoding = encoding;
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::get_default_encoding
//       Access: Published, Static
//  Description: Specifies the default encoding to be used for all
//               subsequently created TextEncoder objects.  See
//               set_encoding().
////////////////////////////////////////////////////////////////////
INLINE TextEncoder::Encoding TextEncoder::
get_default_encoding() {
  return _default_encoding;
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::set_text
//       Access: Published
//  Description: Changes the text that is stored in the encoder.  The
//               text should be encoded according to the method
//               indicated by set_encoding().  Subsequent calls to
//               get_text() will return this same string, while
//               get_wtext() will return the decoded version of the
//               string.
////////////////////////////////////////////////////////////////////
INLINE void TextEncoder::
set_text(const string &text) {
  if (!has_text() || _text != text) {
    _text = text;
    _flags = (_flags | F_got_text) & ~F_got_wtext;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::set_text
//       Access: Published
//  Description: The two-parameter version of set_text() accepts an
//               explicit encoding; the text is immediately decoded
//               and stored as a wide-character string.  Subsequent
//               calls to get_text() will return the same text
//               re-encoded using whichever encoding is specified by
//               set_encoding().
////////////////////////////////////////////////////////////////////
INLINE void TextEncoder::
set_text(const string &text, TextEncoder::Encoding encoding) {
  set_wtext(decode_text(text, encoding));
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::clear_text
//       Access: Published
//  Description: Removes the text from the TextEncoder.
////////////////////////////////////////////////////////////////////
INLINE void TextEncoder::
clear_text() {
  _text = string();
  _wtext = wstring();
  _flags |= (F_got_text | F_got_wtext);
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::has_text
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool TextEncoder::
has_text() const {
  if (_flags & F_got_wtext) {
    return !_wtext.empty();
  } else {
    return !_text.empty();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::get_text
//       Access: Published
//  Description: Returns the current text, as encoded via the current
//               encoding system.
////////////////////////////////////////////////////////////////////
INLINE string TextEncoder::
get_text() const {
  if ((_flags & F_got_text) == 0) {
    ((TextEncoder *)this)->_text = encode_wtext(_wtext);
    ((TextEncoder *)this)->_flags |= F_got_text;
  }
  return _text;
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::get_text
//       Access: Published
//  Description: Returns the current text, as encoded via the indicated
//               encoding system.
////////////////////////////////////////////////////////////////////
INLINE string TextEncoder::
get_text(TextEncoder::Encoding encoding) const {
  return encode_wtext(get_wtext(), encoding);
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::append_text
//       Access: Published
//  Description: Appends the indicates string to the end of the stored
//               text.
////////////////////////////////////////////////////////////////////
INLINE void TextEncoder::
append_text(const string &text) {
  _text = get_text() + text;
  _flags = (_flags | F_got_text) & ~F_got_wtext;
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::append_unicode_char
//       Access: Published
//  Description: Appends a single character to the end of the stored
//               text.  This may be a wide character, up to 16 bits in
//               Unicode.
////////////////////////////////////////////////////////////////////
INLINE void TextEncoder::
append_unicode_char(int character) {
  _wtext = get_wtext() + wstring(1, (wchar_t)character);
  _flags = (_flags | F_got_wtext) & ~F_got_text;
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::get_num_chars
//       Access: Published
//  Description: Returns the number of characters in the stored text.
//               This is a count of wide characters, after the string
//               has been decoded according to set_encoding().
////////////////////////////////////////////////////////////////////
INLINE int TextEncoder::
get_num_chars() const {
  return get_wtext().length();
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::get_unicode_char
//       Access: Published
//  Description: Returns the Unicode value of the nth character in the
//               stored text.  This may be a wide character (greater
//               than 255), after the string has been decoded
//               according to set_encoding().
////////////////////////////////////////////////////////////////////
INLINE int TextEncoder::
get_unicode_char(int index) const {
  get_wtext();
  if (index >= 0 && index < (int)_wtext.length()) {
    return _wtext[index];
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::set_unicode_char
//       Access: Published
//  Description: Sets the Unicode value of the nth character in the
//               stored text.  This may be a wide character (greater
//               than 255), after the string has been decoded
//               according to set_encoding().
////////////////////////////////////////////////////////////////////
INLINE void TextEncoder::
set_unicode_char(int index, int character) {
  get_wtext();
  if (index >= 0 && index < (int)_wtext.length()) {
    _wtext[index] = character;
    _flags &= ~F_got_text;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::get_encoded_char
//       Access: Published
//  Description: Returns the nth char of the stored text, as a one-,
//               two-, or three-byte encoded string.
////////////////////////////////////////////////////////////////////
INLINE string TextEncoder::
get_encoded_char(int index) const {
  return get_encoded_char(index, get_encoding());
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::get_encoded_char
//       Access: Published
//  Description: Returns the nth char of the stored text, as a one-,
//               two-, or three-byte encoded string.
////////////////////////////////////////////////////////////////////
INLINE string TextEncoder::
get_encoded_char(int index, TextEncoder::Encoding encoding) const {
  wstring wch(1, (wchar_t)get_unicode_char(index));
  return encode_wtext(wch, encoding);
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::get_text_as_ascii
//       Access: Published
//  Description: Returns the text associated with the node, converted
//               as nearly as possible to a fully-ASCII
//               representation.  This means replacing accented
//               letters with their unaccented ASCII equivalents.
//
//               It is possible that some characters in the string
//               cannot be converted to ASCII.  (The string may
//               involve symbols like the copyright symbol, for
//               instance, or it might involve letters in some other
//               alphabet such as Greek or Cyrillic, or even Latin
//               letters like thorn or eth that are not part of the
//               ASCII character set.)  In this case, as much of the
//               string as possible will be converted to ASCII, and
//               the nonconvertible characters will remain encoded in
//               the encoding specified by set_encoding().
////////////////////////////////////////////////////////////////////
INLINE string TextEncoder::
get_text_as_ascii() const {
  return encode_wtext(get_wtext_as_ascii());
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::reencode_text
//       Access: Published, Static
//  Description: Given the indicated text string, which is assumed to
//               be encoded via the encoding "from", decodes it and
//               then reencodes it into the encoding "to", and returns
//               the newly encoded string.  This does not change or
//               affect any properties on the TextEncoder itself.
////////////////////////////////////////////////////////////////////
INLINE string TextEncoder::
reencode_text(const string &text, TextEncoder::Encoding from, 
              TextEncoder::Encoding to) {
  return encode_wtext(decode_text(text, from), to);
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::unicode_isalpha
//       Access: Published, Static
//  Description: Returns true if the indicated character is an
//               alphabetic letter, false otherwise.  This is akin to
//               ctype's isalpha(), extended to Unicode.
////////////////////////////////////////////////////////////////////
INLINE bool TextEncoder::
unicode_isalpha(int character) {
  const UnicodeLatinMap::Entry *entry = UnicodeLatinMap::look_up(character);
  if (entry == (const UnicodeLatinMap::Entry *)NULL) {
    return false;
  }
  return entry->_char_type == UnicodeLatinMap::CT_upper ||
    entry->_char_type == UnicodeLatinMap::CT_lower;
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::unicode_isdigit
//       Access: Published, Static
//  Description: Returns true if the indicated character is a
//               numeric digit, false otherwise.  This is akin to
//               ctype's isdigit(), extended to Unicode.
////////////////////////////////////////////////////////////////////
INLINE bool TextEncoder::
unicode_isdigit(int character) {
  const UnicodeLatinMap::Entry *entry = UnicodeLatinMap::look_up(character);
  if (entry == (const UnicodeLatinMap::Entry *)NULL) {
    // The digits aren't actually listed in the map.
    return (character >= '0' && character <= '9');
  }
  // This silly test (!= 0) is necessary to prevent a VC++ warning.
  return (isdigit(entry->_ascii_equiv) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::unicode_ispunct
//       Access: Published, Static
//  Description: Returns true if the indicated character is a
//               punctuation mark, false otherwise.  This is akin to
//               ctype's ispunct(), extended to Unicode.
////////////////////////////////////////////////////////////////////
INLINE bool TextEncoder::
unicode_ispunct(int character) {
  const UnicodeLatinMap::Entry *entry = UnicodeLatinMap::look_up(character);
  if (entry == (const UnicodeLatinMap::Entry *)NULL) {
    // Some punctuation marks aren't listed in the map.
    return (character >= 0 && character < 128 && ispunct(character));
  }
  return entry->_char_type == UnicodeLatinMap::CT_punct;
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::unicode_isupper
//       Access: Published, Static
//  Description: Returns true if the indicated character is an
//               uppercase letter, false otherwise.  This is akin to
//               ctype's isupper(), extended to Unicode.
////////////////////////////////////////////////////////////////////
INLINE bool TextEncoder::
unicode_isupper(int character) {
  const UnicodeLatinMap::Entry *entry = UnicodeLatinMap::look_up(character);
  if (entry == (const UnicodeLatinMap::Entry *)NULL) {
    return false;
  }
  return entry->_char_type == UnicodeLatinMap::CT_upper;
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::unicode_isspace
//       Access: Published, Static
//  Description: Returns true if the indicated character is a
//               whitespace letter, false otherwise.  This is akin to
//               ctype's isspace(), extended to Unicode.
////////////////////////////////////////////////////////////////////
INLINE bool TextEncoder::
unicode_isspace(int character) {
  switch (character) {
  case ' ':
  case '\t':
  case '\n':
    return true;

  default:
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::unicode_islower
//       Access: Published, Static
//  Description: Returns true if the indicated character is a
//               lowercase letter, false otherwise.  This is akin to
//               ctype's islower(), extended to Unicode.
////////////////////////////////////////////////////////////////////
INLINE bool TextEncoder::
unicode_islower(int character) {
  const UnicodeLatinMap::Entry *entry = UnicodeLatinMap::look_up(character);
  if (entry == (const UnicodeLatinMap::Entry *)NULL) {
    return false;
  }
  return entry->_char_type == UnicodeLatinMap::CT_lower;
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::unicode_toupper
//       Access: Published, Static
//  Description: Returns the uppercase equivalent of the given Unicode
//               character.  This is akin to ctype's toupper(),
//               extended to Unicode.
////////////////////////////////////////////////////////////////////
INLINE int TextEncoder::
unicode_toupper(int character) {
  const UnicodeLatinMap::Entry *entry = UnicodeLatinMap::look_up(character);
  if (entry == (const UnicodeLatinMap::Entry *)NULL) {
    return character;
  } 
  return entry->_toupper_character;
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::unicode_tolower
//       Access: Published, Static
//  Description: Returns the uppercase equivalent of the given Unicode
//               character.  This is akin to ctype's tolower(),
//               extended to Unicode.
////////////////////////////////////////////////////////////////////
INLINE int TextEncoder::
unicode_tolower(int character) {
  const UnicodeLatinMap::Entry *entry = UnicodeLatinMap::look_up(character);
  if (entry == (const UnicodeLatinMap::Entry *)NULL) {
    return character;
  } 
  return entry->_tolower_character;
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::upper
//       Access: Published, Static
//  Description: Converts the string to uppercase, assuming the string
//               is encoded in the default encoding.
////////////////////////////////////////////////////////////////////
INLINE string TextEncoder::
upper(const string &source) {
  return upper(source, get_default_encoding());
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::upper
//       Access: Published, Static
//  Description: Converts the string to uppercase, assuming the string
//               is encoded in the indicated encoding.
////////////////////////////////////////////////////////////////////
INLINE string TextEncoder::
upper(const string &source, TextEncoder::Encoding encoding) {
  TextEncoder encoder;
  encoder.set_encoding(encoding);
  encoder.set_text(source);
  encoder.make_upper();
  return encoder.get_text();
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::lower
//       Access: Published, Static
//  Description: Converts the string to lowercase, assuming the string
//               is encoded in the default encoding.
////////////////////////////////////////////////////////////////////
INLINE string TextEncoder::
lower(const string &source) {
  return lower(source, get_default_encoding());
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::lower
//       Access: Published, Static
//  Description: Converts the string to lowercase, assuming the string
//               is encoded in the indicated encoding.
////////////////////////////////////////////////////////////////////
INLINE string TextEncoder::
lower(const string &source, TextEncoder::Encoding encoding) {
  TextEncoder encoder;
  encoder.set_encoding(encoding);
  encoder.set_text(source);
  encoder.make_lower();
  return encoder.get_text();
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::set_wtext
//       Access: Published
//  Description: Changes the text that is stored in the encoder.
//               Subsequent calls to get_wtext() will return this same
//               string, while get_text() will return the encoded
//               version of the string.
////////////////////////////////////////////////////////////////////
INLINE void TextEncoder::
set_wtext(const wstring &wtext) {
  if (!has_text() || _wtext != wtext) {
    _wtext = wtext;
    _flags = (_flags | F_got_wtext) & ~F_got_text;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::get_wtext
//       Access: Published
//  Description: Returns the text associated with the TextEncoder, as
//               a wide-character string.
////////////////////////////////////////////////////////////////////
INLINE const wstring &TextEncoder::
get_wtext() const {
  if ((_flags & F_got_wtext) == 0) {
    ((TextEncoder *)this)->_wtext = decode_text(_text);
    ((TextEncoder *)this)->_flags |= F_got_wtext;
  }
  return _wtext;
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::append_wtext
//       Access: Published
//  Description: Appends the indicates string to the end of the stored
//               wide-character text.
////////////////////////////////////////////////////////////////////
INLINE void TextEncoder::
append_wtext(const wstring &wtext) {
  _wtext = get_wtext() + wtext;
  _flags = (_flags | F_got_wtext) & ~F_got_text;
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::encode_wtext
//       Access: Published
//  Description: Encodes a wide-text string into a single-char string,
//               according to the current encoding.
////////////////////////////////////////////////////////////////////
INLINE string TextEncoder::
encode_wtext(const wstring &wtext) const {
  return encode_wtext(wtext, _encoding);
}

////////////////////////////////////////////////////////////////////
//     Function: TextEncoder::decode_text
//       Access: Published
//  Description: Returns the given wstring decoded to a single-byte
//               string, via the current encoding system.
////////////////////////////////////////////////////////////////////
INLINE wstring TextEncoder::
decode_text(const string &text) const {
  return decode_text(text, _encoding);
}

////////////////////////////////////////////////////////////////////
//     Function: wstring ostream operator
//  Description: Uses the current default encoding to output the
//               wstring.
////////////////////////////////////////////////////////////////////
INLINE ostream &
operator << (ostream &out, const wstring &str) {
  TextEncoder encoder;
  encoder.set_wtext(str);
  out << encoder.get_text();
  return out;
}
