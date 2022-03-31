/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file unicodeLatinMap.h
 * @author drose
 * @date 2003-02-01
 */

#ifndef UNICODELATINMAP_H
#define UNICODELATINMAP_H

#include "dtoolbase.h"
#include "pmap.h"

/**
 * This class mainly serves as a container for a largish table of the subset
 * of the Unicode character set that corresponds to the Latin alphabet, with
 * its various accent marks and so on.  Specifically, this table indicates how
 * to map between the Unicode accented character and the corresponding ASCII
 * equivalent without the accent mark; as well as how to switch case from
 * upper to lower while retaining the Unicode accent marks.
 */
class EXPCL_DTOOL_DTOOLUTIL UnicodeLatinMap {
public:
  enum AccentType {
    AT_none,
    AT_acute,
    AT_acute_and_dot_above,
    AT_breve,
    AT_breve_and_acute,
    AT_breve_and_dot_below,
    AT_breve_and_grave,
    AT_breve_and_hook_above,
    AT_breve_and_tilde,
    AT_breve_below,
    AT_caron,
    AT_caron_and_dot_above,
    AT_cedilla,
    AT_cedilla_and_acute,
    AT_cedilla_and_breve,
    AT_circumflex,
    AT_circumflex_and_acute,
    AT_circumflex_and_dot_below,
    AT_circumflex_and_grave,
    AT_circumflex_and_hook_above,
    AT_circumflex_and_tilde,
    AT_circumflex_below,
    AT_comma_below,
    AT_curl,
    AT_diaeresis,
    AT_diaeresis_and_acute,
    AT_diaeresis_and_caron,
    AT_diaeresis_and_grave,
    AT_diaeresis_and_macron,
    AT_diaeresis_below,
    AT_dot_above,
    AT_dot_above_and_macron,
    AT_dot_below,
    AT_dot_below_and_dot_above,
    AT_dot_below_and_macron,
    AT_double_acute,
    AT_double_grave,
    AT_grave,
    AT_hook,
    AT_hook_above,
    AT_horn,
    AT_horn_and_acute,
    AT_horn_and_dot_below,
    AT_horn_and_grave,
    AT_horn_and_hook_above,
    AT_horn_and_tilde,
    AT_inverted_breve,
    AT_line_below,
    AT_macron,
    AT_macron_and_acute,
    AT_macron_and_diaeresis,
    AT_macron_and_grave,
    AT_ogonek,
    AT_ogonek_and_macron,
    AT_ring_above,
    AT_ring_above_and_acute,
    AT_ring_below,
    AT_stroke,
    AT_stroke_and_acute,
    AT_stroke_and_hook,
    AT_tilde,
    AT_tilde_and_acute,
    AT_tilde_and_diaeresis,
    AT_tilde_and_macron,
    AT_tilde_below,
    AT_topbar,
  };

  enum AdditionalFlags {
    AF_ligature   = 0x0001,
    AF_turned     = 0x0002,
    AF_reversed   = 0x0004,
    AF_smallcap   = 0x0008,
    AF_dotless    = 0x0010,
  };

  enum CharType {
    CT_upper,
    CT_lower,
    CT_punct,
  };

  class Entry {
  public:
    char32_t _character;
    CharType _char_type;
    char _ascii_equiv;
    char _ascii_additional;
    char32_t _tolower_character;
    char32_t _toupper_character;
    AccentType _accent_type;
    int _additional_flags;
  };

  static const Entry *look_up(char32_t character);

  static wchar_t get_combining_accent(AccentType accent);

private:
  static void init();
  static bool _initialized;

  typedef phash_map<char32_t, const Entry *, integer_hash<char32_t> > ByCharacter;
  static ByCharacter *_by_character;
  enum { max_direct_chars = 256 };
  static const Entry *_direct_chars[max_direct_chars];
};

#endif
