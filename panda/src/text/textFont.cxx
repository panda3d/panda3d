// Filename: textFont.cxx
// Created by:  drose (08Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "textFont.h"
#include "config_text.h"
#include <ctype.h>

TypeHandle TextFont::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: isbreakpoint
//  Description: An internal function, similar to isspace(), except it
//               does not consider newlines to be whitespace.  It also
//               includes the soft-hyphen character.
////////////////////////////////////////////////////////////////////
static INLINE bool
isbreakpoint(unsigned int ch) {
  return (ch == ' ' || ch == '\t' || ch == (unsigned int)text_soft_hyphen_key);
}

////////////////////////////////////////////////////////////////////
//     Function: isspacew
//  Description: An internal function that works like isspace() but is
//               safe to call for a wide character.
////////////////////////////////////////////////////////////////////
static INLINE bool
isspacew(unsigned int ch) {
  return isascii(ch) && isspace(ch);
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TextFont::
TextFont() {
  _is_valid = false;
  _line_height = 1.0f;
  _space_advance = 0.25f;
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
TextFont::
~TextFont() {
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::calc_width
//       Access: Published
//  Description: Returns the width of a single character of the font,
//               or 0.0 if the character is not known.
////////////////////////////////////////////////////////////////////
float TextFont::
calc_width(int character) {
  if (character == ' ') {
    // A space is a special case.
    return _space_advance;
  }

  const TextGlyph *glyph;
  get_glyph(character, glyph);
  if (glyph == (TextGlyph *)NULL) {
    // Unknown character.
    return 0.0f;
  }

  return glyph->get_advance();
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::calc_width
//       Access: Published
//  Description: Returns the width of a line of text of arbitrary
//               characters.  The line should not include the newline
//               character.
////////////////////////////////////////////////////////////////////
float TextFont::
calc_width(const string &line) {
  float width = 0.0f;

  string::const_iterator si;
  for (si = line.begin(); si != line.end(); ++si) {
    width += calc_width(*si);
  }

  return width;
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::wordwrap_to
//       Access: Published
//  Description: Inserts newlines into the given text at the
//               appropriate places in order to make each line be the
//               longest possible line that is not longer than
//               wordwrap_width (and does not break any words, if
//               possible).  Returns the new string.
////////////////////////////////////////////////////////////////////
string TextFont::
wordwrap_to(const string &text, float wordwrap_width, 
            bool preserve_trailing_whitespace) {
  // Elevate the string to a wstring for this operation.
  wstring wtext;
  wtext.reserve(text.size());
  for (string::const_iterator ti = text.begin(); ti != text.end(); ++ti) {
    wtext += (unsigned int)(*ti);
  }

  wtext = wordwrap_to(wtext, wordwrap_width, preserve_trailing_whitespace);

  // Back down from wstring to string.
  string result;
  result.reserve(wtext.size());
  for (wstring::const_iterator wi = wtext.begin();
       wi != wtext.end();
       ++wi) {
    result += (char)(*wi);
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::write
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void TextFont::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "TextFont " << get_name() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::calc_width (wide char)
//       Access: Public
//  Description: Returns the width of a line of text of arbitrary
//               characters.  The line should not include the newline
//               character.
////////////////////////////////////////////////////////////////////
float TextFont::
calc_width(const wstring &line) {
  float width = 0.0f;

  wstring::const_iterator si;
  for (si = line.begin(); si != line.end(); ++si) {
    width += calc_width(*si);
  }

  return width;
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::wordwrap_to (wide char)
//       Access: Public
//  Description: Inserts newlines into the given text at the
//               appropriate places in order to make each line be the
//               longest possible line that is not longer than
//               wordwrap_width (and does not break any words, if
//               possible).  Returns the new string.
////////////////////////////////////////////////////////////////////
wstring TextFont::
wordwrap_to(const wstring &text, float wordwrap_width, 
            bool preserve_trailing_whitespace) {
  wstring output_text;

  size_t p = 0;

  // Preserve any initial whitespace and newlines.
  float initial_width = 0.0f;
  while (p < text.length() && isspacew(text[p])) {
    if (text[p] == '\n') {
      initial_width = 0.0f;
    } else {
      initial_width += calc_width(text[p]);
    }
    output_text += text[p];
    p++;
  }
  bool needs_newline = false;

  while (p < text.length()) {
    nassertr(!isspacew(text[p]), wstring());

    // Scan the next n characters, until the end of the string or an
    // embedded newline character, or we exceed wordwrap_width.

    size_t q = p;
    bool any_spaces = false;
    size_t last_space = 0;
    float last_space_width = 0.0f;

    bool any_hyphens = false;
    size_t last_hyphen = 0;
    bool output_hyphen = false;

    bool overflow = false;

    bool last_was_space = false;
    float hyphen_width = calc_width(*text_soft_hyphen_output);
    float width = initial_width;
    while (q < text.length() && text[q] != '\n') {
      if (isspacew(text[q])) {
        if (!last_was_space) {
          any_spaces = true;
          // We only care about logging whether there is a soft-hyphen
          // character to the right of the rightmost space.  Each time
          // we encounter a space, we reset this counter.
          any_hyphens = false;
          last_space = q;
          last_space_width = width;
          last_was_space = true;
        }
      } else {
        last_was_space = false;
      }

      // A soft hyphen character is not printed, but marks a point
      // that we might hyphenate a word if we need to.
      if (text[q] == text_soft_hyphen_key) {
        // We only consider this as a possible hyphenation point if
        // (a) it is not the very first character, and (b) there is
        // enough room for a hyphen character to be printed following
        // it.
        if (q != p && width + hyphen_width <= wordwrap_width) {
          any_hyphens = true;
          last_hyphen = q;
        }

      } else {
        // Some normal, printable character.
        width += calc_width(text[q]);
      }

      q++;
        
      if (width > wordwrap_width) {
        // Oops, too many.
        q--;
        overflow = true;
        break;
      }
    }

    if (overflow) {
      // If we stopped because we exceeded the wordwrap width, then
      // try to find an appropriate place to wrap the line or to
      // hyphenate, if necessary.
      if (any_spaces && last_space_width / wordwrap_width >= text_hyphen_ratio) {
        // If we have a space that ended up within our safety margin,
        // don't use any soft-hyphen characters.
        any_hyphens = false;
      }

      if (any_hyphens) {
        // If we still have a soft-hyphen character, use it.
        q = last_hyphen;
        output_hyphen = true;

      } else if (any_spaces) {
        // Otherwise, break at a space if we can.
        q = last_space;
      }
    }

    // Skip additional whitespace between the lines.
    size_t next_start = q;
    while (next_start < text.length() && isbreakpoint(text[next_start])) {
      next_start++;
    }

    // Trim off any more blanks on the end.
    while (q > p && isspacew(text[q - 1])) {
      q--;
    }

    if (next_start == p) {
      // No characters got in at all.  This could only happen if the
      // wordwrap width is narrower than a single character, or if we
      // have a substantial number of leading spaces in a line.
      q++;
      next_start++;
      while (next_start < text.length() && isbreakpoint(text[next_start])) {
        next_start++;
      }
    }
    
    if (needs_newline) {
      output_text += '\n';
    }
    needs_newline = true;

    if (preserve_trailing_whitespace) {
      q = next_start;
    }

    for (size_t pi = p; pi < q; pi++) {
      if (text[pi] != text_soft_hyphen_key) {
        output_text += text[pi];
      }
    }
    if (output_hyphen) {
      output_text += *text_soft_hyphen_output;
    }

    // Now prepare to wrap the next line.

    if (next_start < text.length() && text[next_start] == '\n') {
      // Preserve a single embedded newline.
      output_text += '\n';
      next_start++;
      needs_newline = false;
    }
    p = next_start;

    // Preserve any initial whitespace and newlines.
    initial_width = 0.0f;
    while (p < text.length() && isspacew(text[p])) {
      if (text[p] == '\n') {
        initial_width = 0.0f;
      } else {
        initial_width += calc_width(text[p]);
      }
      output_text += text[p];
      p++;
    }
  }

  return output_text;
}
