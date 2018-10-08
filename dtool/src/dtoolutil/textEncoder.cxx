/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textEncoder.cxx
 * @author drose
 * @date 2003-03-26
 */

#include "textEncoder.h"
#include "stringDecoder.h"
#include "unicodeLatinMap.h"
#include "config_dtoolutil.h"

using std::istream;
using std::ostream;
using std::string;
using std::wstring;

TextEncoder::Encoding TextEncoder::_default_encoding = TextEncoder::E_utf8;

/**
 * Adjusts the text stored within the encoder to all uppercase letters
 * (preserving accent marks correctly).
 */
void TextEncoder::
make_upper() {
  get_wtext();
  wstring::iterator si;
  for (si = _wtext.begin(); si != _wtext.end(); ++si) {
    (*si) = unicode_toupper(*si);
  }
  _flags &= ~F_got_text;
  text_changed();
}

/**
 * Adjusts the text stored within the encoder to all lowercase letters
 * (preserving accent marks correctly).
 */
void TextEncoder::
make_lower() {
  get_wtext();
  wstring::iterator si;
  for (si = _wtext.begin(); si != _wtext.end(); ++si) {
    (*si) = unicode_tolower(*si);
  }
  _flags &= ~F_got_text;
  text_changed();
}

/**
 * Returns the text associated with the node, converted as nearly as possible
 * to a fully-ASCII representation.  This means replacing accented letters
 * with their unaccented ASCII equivalents.
 *
 * It is possible that some characters in the string cannot be converted to
 * ASCII.  (The string may involve symbols like the copyright symbol, for
 * instance, or it might involve letters in some other alphabet such as Greek
 * or Cyrillic, or even Latin letters like thorn or eth that are not part of
 * the ASCII character set.)  In this case, as much of the string as possible
 * will be converted to ASCII, and the nonconvertible characters will remain
 * in their original form.
 */
wstring TextEncoder::
get_wtext_as_ascii() const {
  get_wtext();
  wstring result;
  wstring::const_iterator si;
  for (si = _wtext.begin(); si != _wtext.end(); ++si) {
    wchar_t character = (*si);

    const UnicodeLatinMap::Entry *map_entry =
      UnicodeLatinMap::look_up(character);
    if (map_entry != nullptr && map_entry->_ascii_equiv != 0) {
      result += (wchar_t)map_entry->_ascii_equiv;
      if (map_entry->_ascii_additional != 0) {
        result += (wchar_t)map_entry->_ascii_additional;
      }

    } else {
      result += character;
    }
  }

  return result;
}

/**
 * Returns true if any of the characters in the string returned by get_wtext()
 * are out of the range of an ASCII character (and, therefore, get_wtext()
 * should be called in preference to get_text()).
 */
bool TextEncoder::
is_wtext() const {
  get_wtext();
  wstring::const_iterator ti;
  for (ti = _wtext.begin(); ti != _wtext.end(); ++ti) {
    if (((*ti) & ~0x7f) != 0) {
      return true;
    }
  }

  return false;
}

/**
 * Encodes a single Unicode character into a one-, two-, three-, or four-byte
 * string, according to the given encoding system.
 */
string TextEncoder::
encode_wchar(char32_t ch, TextEncoder::Encoding encoding) {
  switch (encoding) {
  case E_iso8859:
    if ((ch & ~0xff) == 0) {
      return string(1, (char)ch);
    } else {
      // The character won't fit in the 8-bit ISO 8859.  See if we can make it
      // fit by reducing it to its ascii equivalent (essentially stripping off
      // an unusual accent mark).
      const UnicodeLatinMap::Entry *map_entry =
        UnicodeLatinMap::look_up(ch);
      if (map_entry != nullptr && map_entry->_ascii_equiv != 0) {
        // Yes, it has an ascii equivalent.
        if (map_entry->_ascii_additional != 0) {
          // In fact, it has two of them.
          return
            string(1, map_entry->_ascii_equiv) +
            string(1, map_entry->_ascii_additional);
        }
        return string(1, map_entry->_ascii_equiv);
      }
      // Nope; return "." for lack of anything better.
      return ".";
    }

  case E_utf8:
    if ((ch & ~0x7f) == 0) {
      return string(1, (char)ch);
    } else if ((ch & ~0x7ff) == 0) {
      return
        string(1, (char)((ch >> 6) | 0xc0)) +
        string(1, (char)((ch & 0x3f) | 0x80));
    } else if ((ch & ~0xffff) == 0) {
      return
        string(1, (char)((ch >> 12) | 0xe0)) +
        string(1, (char)(((ch >> 6) & 0x3f) | 0x80)) +
        string(1, (char)((ch & 0x3f) | 0x80));
    } else {
      return
        string(1, (char)((ch >> 18) | 0xf0)) +
        string(1, (char)(((ch >> 12) & 0x3f) | 0x80)) +
        string(1, (char)(((ch >> 6) & 0x3f) | 0x80)) +
        string(1, (char)((ch & 0x3f) | 0x80));
    }

  case E_utf16be:
    if ((ch & ~0xffff) == 0) {
      // Note that this passes through surrogates and BOMs unharmed.
      return
        string(1, (char)(ch >> 8)) +
        string(1, (char)(ch & 0xff));
    } else {
      // Use a surrogate pair.
      uint32_t v = (uint32_t)ch - 0x10000u;
      uint16_t hi = (v >> 10u) | 0xd800u;
      uint16_t lo = (v & 0x3ffu) | 0xdc00u;
      char encoded[4] = {
        (char)(hi >> 8),
        (char)(hi & 0xff),
        (char)(lo >> 8),
        (char)(lo & 0xff),
      };
      return string(encoded, 4);
    }
  }

  return "";
}

/**
 * Encodes a wide-text string into a single-char string, according to the
 * given encoding.
 */
string TextEncoder::
encode_wtext(const wstring &wtext, TextEncoder::Encoding encoding) {
  string result;

  for (size_t i = 0; i < wtext.size(); ++i) {
    wchar_t ch = wtext[i];

    // On some systems, wstring may be UTF-16, and contain surrogate pairs.
#if WCHAR_MAX < 0x10FFFF
    if (ch >= 0xd800 && ch < 0xdc00 && (i + 1) < wtext.size()) {
      // This is a high surrogate.  Look for a subsequent low surrogate.
      wchar_t ch2 = wtext[i + 1];
      if (ch2 >= 0xdc00 && ch2 < 0xe000) {
        // Yes, this is a low surrogate.
        char32_t code_point = 0x10000 + ((ch - 0xd800) << 10) + (ch2 - 0xdc00);
        result += encode_wchar(code_point, encoding);
        i++;
        continue;
      }
    }
#endif

    result += encode_wchar(ch, encoding);
  }

  return result;
}

/**
 * Returns the given wstring decoded to a single-byte string, via the given
 * encoding system.
 */
wstring TextEncoder::
decode_text(const string &text, TextEncoder::Encoding encoding) {
  switch (encoding) {
  case E_utf8:
    {
      StringUtf8Decoder decoder(text);
      return decode_text_impl(decoder);
    }

  case E_utf16be:
    {
      StringUtf16Decoder decoder(text);
      return decode_text_impl(decoder);
    }

  case E_iso8859:
  default:
    {
      StringDecoder decoder(text);
      return decode_text_impl(decoder);
    }
  };
}

/**
 * Decodes the eight-bit stream from the indicated decoder, returning the
 * decoded wide-char string.
 */
wstring TextEncoder::
decode_text_impl(StringDecoder &decoder) {
  wstring result;
  // bool expand_amp = get_expand_amp();

  char32_t character = decoder.get_next_character();
  while (!decoder.is_eof()) {
    /*
    if (character == '&' && expand_amp) {
      // An ampersand in expand_amp mode is treated as an escape character.
      character = expand_amp_sequence(decoder);
    }
    */
    if (character <= WCHAR_MAX) {
      result += character;
    } else {
      // We need to encode this as a surrogate pair.
      uint32_t v = (uint32_t)character - 0x10000u;
      result += (wchar_t)((v >> 10u) | 0xd800u);
      result += (wchar_t)((v & 0x3ffu) | 0xdc00u);
    }
    character = decoder.get_next_character();
  }

  return result;
}

/**
 * Given that we have just read an ampersand from the StringDecoder, and that
 * we have expand_amp in effect and are therefore expected to expand the
 * sequence that this ampersand begins into a single unicode character, do the
 * expansion and return the character.
 */
/*
int TextEncoder::
expand_amp_sequence(StringDecoder &decoder) const {
  int result = 0;

  int character = decoder.get_next_character();
  if (!decoder.is_eof() && character == '#') {
    // An explicit numeric sequence: &#nnn;
    result = 0;
    character = decoder.get_next_character();
    while (!decoder.is_eof() && character < 128 && isdigit((unsigned int)character)) {
      result = (result * 10) + (character - '0');
      character = decoder.get_next_character();
    }
    if (character != ';') {
      // Invalid sequence.
      return 0;
    }

    return result;
  }

  string sequence;

  // Some non-numeric sequence.
  while (!decoder.is_eof() && character < 128 && isalpha((unsigned int)character)) {
    sequence += character;
    character = decoder.get_next_character();
  }
  if (character != ';') {
    // Invalid sequence.
    return 0;
  }

  static const struct {
    const char *name;
    int code;
  } tokens[] = {
    { "amp", '&' }, { "lt", '<' }, { "gt", '>' }, { "quot", '"' },
    { "nbsp", ' ' },

    { "iexcl", 161 }, { "cent", 162 }, { "pound", 163 }, { "curren", 164 },
    { "yen", 165 }, { "brvbar", 166 }, { "brkbar", 166 }, { "sect", 167 },
    { "uml", 168 }, { "die", 168 }, { "copy", 169 }, { "ordf", 170 },
    { "laquo", 171 }, { "not", 172 }, { "shy", 173 }, { "reg", 174 },
    { "macr", 175 }, { "hibar", 175 }, { "deg", 176 }, { "plusmn", 177 },
    { "sup2", 178 }, { "sup3", 179 }, { "acute", 180 }, { "micro", 181 },
    { "para", 182 }, { "middot", 183 }, { "cedil", 184 }, { "sup1", 185 },
    { "ordm", 186 }, { "raquo", 187 }, { "frac14", 188 }, { "frac12", 189 },
    { "frac34", 190 }, { "iquest", 191 }, { "Agrave", 192 }, { "Aacute", 193 },
    { "Acirc", 194 }, { "Atilde", 195 }, { "Auml", 196 }, { "Aring", 197 },
    { "AElig", 198 }, { "Ccedil", 199 }, { "Egrave", 200 }, { "Eacute", 201 },
    { "Ecirc", 202 }, { "Euml", 203 }, { "Igrave", 204 }, { "Iacute", 205 },
    { "Icirc", 206 }, { "Iuml", 207 }, { "ETH", 208 }, { "Dstrok", 208 },
    { "Ntilde", 209 }, { "Ograve", 210 }, { "Oacute", 211 }, { "Ocirc", 212 },
    { "Otilde", 213 }, { "Ouml", 214 }, { "times", 215 }, { "Oslash", 216 },
    { "Ugrave", 217 }, { "Uacute", 218 }, { "Ucirc", 219 }, { "Uuml", 220 },
    { "Yacute", 221 }, { "THORN", 222 }, { "szlig", 223 }, { "agrave", 224 },
    { "aacute", 225 }, { "acirc", 226 }, { "atilde", 227 }, { "auml", 228 },
    { "aring", 229 }, { "aelig", 230 }, { "ccedil", 231 }, { "egrave", 232 },
    { "eacute", 233 }, { "ecirc", 234 }, { "euml", 235 }, { "igrave", 236 },
    { "iacute", 237 }, { "icirc", 238 }, { "iuml", 239 }, { "eth", 240 },
    { "ntilde", 241 }, { "ograve", 242 }, { "oacute", 243 }, { "ocirc", 244 },
    { "otilde", 245 }, { "ouml", 246 }, { "divide", 247 }, { "oslash", 248 },
    { "ugrave", 249 }, { "uacute", 250 }, { "ucirc", 251 }, { "uuml", 252 },
    { "yacute", 253 }, { "thorn", 254 }, { "yuml", 255 },

    { NULL, 0 },
  };

  for (int i = 0; tokens[i].name != NULL; i++) {
    if (sequence == tokens[i].name) {
      // Here's a match.
      return tokens[i].code;
    }
  }

  // Some unrecognized sequence.
  return 0;
}
*/

/**
 * Called whenever the text has been changed.
 */
void TextEncoder::
text_changed() {
}

/**
 *
 */
ostream &
operator << (ostream &out, TextEncoder::Encoding encoding) {
  switch (encoding) {
  case TextEncoder::E_iso8859:
    return out << "iso8859";

  case TextEncoder::E_utf8:
    return out << "utf8";

  case TextEncoder::E_utf16be:
    return out << "utf16be";
  };

  return out << "**invalid TextEncoder::Encoding(" << (int)encoding << ")**";
}

/**
 *
 */
istream &
operator >> (istream &in, TextEncoder::Encoding &encoding) {
  string word;
  in >> word;

  if (word == "iso8859") {
    encoding = TextEncoder::E_iso8859;
  } else if (word == "utf8" || word == "utf-8") {
    encoding = TextEncoder::E_utf8;
  } else if (word == "unicode" || word == "utf16be" || word == "utf-16be" ||
                                  word == "utf16-be" || word == "utf-16-be") {
    encoding = TextEncoder::E_utf16be;
  } else {
    ostream *notify_ptr = StringDecoder::get_notify_ptr();
    if (notify_ptr != nullptr) {
      (*notify_ptr)
        << "Invalid TextEncoder::Encoding: " << word << "\n";
    }
    encoding = TextEncoder::E_iso8859;
  }

  return in;
}
