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

// Maps cp437 characters to Unicode codepoints.
static char16_t cp437_table[256] = {
  0x0000, 0x263a, 0x263b, 0x2665, 0x2666, 0x2663, 0x2660, 0x2022,
  0x25d8, 0x25cb, 0x25d9, 0x2642, 0x2640, 0x266a, 0x266b, 0x263c,
  0x25ba, 0x25c4, 0x2195, 0x203c, 0x00b6, 0x00a7, 0x25ac, 0x21a8,
  0x2191, 0x2193, 0x2192, 0x2190, 0x221f, 0x2194, 0x25b2, 0x25bc,
  0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
  0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
  0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
  0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
  0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
  0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
  0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
  0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
  0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
  0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
  0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
  0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x2302,
  0x00c7, 0x00fc, 0x00e9, 0x00e2, 0x00e4, 0x00e0, 0x00e5, 0x00e7,
  0x00ea, 0x00eb, 0x00e8, 0x00ef, 0x00ee, 0x00ec, 0x00c4, 0x00c5,
  0x00c9, 0x00e6, 0x00c6, 0x00f4, 0x00f6, 0x00f2, 0x00fb, 0x00f9,
  0x00ff, 0x00d6, 0x00dc, 0x00a2, 0x00a3, 0x00a5, 0x20a7, 0x0192,
  0x00e1, 0x00ed, 0x00f3, 0x00fa, 0x00f1, 0x00d1, 0x00aa, 0x00ba,
  0x00bf, 0x2310, 0x00ac, 0x00bd, 0x00bc, 0x00a1, 0x00ab, 0x00bb,
  0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
  0x2555, 0x2563, 0x2551, 0x2557, 0x255d, 0x255c, 0x255b, 0x2510,
  0x2514, 0x2534, 0x252c, 0x251c, 0x2500, 0x253c, 0x255e, 0x255f,
  0x255a, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256c, 0x2567,
  0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256b,
  0x256a, 0x2518, 0x250c, 0x2588, 0x2584, 0x258c, 0x2590, 0x2580,
  0x03b1, 0x00df, 0x0393, 0x03c0, 0x03a3, 0x03c3, 0x00b5, 0x03c4,
  0x03a6, 0x0398, 0x03a9, 0x03b4, 0x221e, 0x03c6, 0x03b5, 0x2229,
  0x2261, 0x00b1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00f7, 0x2248,
  0x00b0, 0x2219, 0x00b7, 0x221a, 0x207f, 0x00b2, 0x25a0, 0x00a0,
};

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

  case E_cp437:
    if ((ch & ~0x7f) == 0) {
      return string(1, (char)ch);
    }
    else if (ch >= 0 && ch < 0x266b) {
      // This case is not optimized, because we don't really need it right now.
      for (int i = 0; i < 256; ++i) {
        if (cp437_table[i] == ch) {
          return std::string(1, (char)i);
        }
      }
    }
    return ".";
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

  case E_cp437:
    {
      std::wstring result(text.size(), 0);
      for (size_t i = 0; i < result.size(); ++i) {
        result[i] = cp437_table[(uint8_t)text[i]];
      }
      return result;
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

  case TextEncoder::E_cp437:
    return out << "cp437";
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
  } else if (word == "cp437") {
    encoding = TextEncoder::E_cp437;
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
