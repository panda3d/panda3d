// Filename: textFont.cxx
// Created by:  drose (08Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "textFont.h"
#include "config_text.h"
#include "ctype.h"

TypeHandle TextFont::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: isblank
//  Description: An internal function, similar to isspace(), except it
//               does not consider newlines to be whitespace.
////////////////////////////////////////////////////////////////////
INLINE bool
isblank(unsigned int ch) {
  return (ch == ' ' || ch == '\t');
}

////////////////////////////////////////////////////////////////////
//     Function: isspacew
//  Description: An internal function that works like isspace() but is
//               safe to call for a wide character.
////////////////////////////////////////////////////////////////////
INLINE bool
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
  string output_text;

  size_t p = 0;

  // Preserve any initial whitespace and newlines.
  float initial_width = 0.0f;
  while (p < text.length() && isspace((unsigned int)text[p])) {  // dbg runtime will bomb if text[p]>=128 without (unsigned int) cast
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
    nassertr(!isspace((unsigned int)text[p]), string());

    // Scan the next n characters, until the end of the string or an
    // embedded newline character, or we exceed wordwrap_width.

    size_t q = p;
    bool any_spaces = false;
    bool overflow = false;

    float width = initial_width;
    while (q < text.length() && text[q] != '\n') {
      if (isspace((unsigned int)text[q])) {
        any_spaces = true;
      }

      width += calc_width(text[q]);
      q++;

      if (width > wordwrap_width) {
        // Oops, too many.
        q--;
        overflow = true;
        break;
      }
    }

    if (overflow && any_spaces) {
      // If we stopped because we exceeded the wordwrap width, then
      // back up to the end of the last complete word.
      while (q > p && !isspace((unsigned int)text[q])) {
        q--;
      }
    }

    // Skip additional whitespace between the lines.
    size_t next_start = q;
    while (next_start < text.length() && isblank(text[next_start])) {
      next_start++;
    }

    // Trim off any more blanks on the end.
    while (q > p && isspace(text[q - 1])) {
      q--;
    }

    if (next_start == p) {
      // No characters got in at all.  This could only happen if the
      // wordwrap width is narrower than a single character, or if we
      // have a substantial number of leading spaces in a line.
      q++;
      next_start++;
      while (next_start < text.length() && isblank(text[next_start])) {
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
    output_text += text.substr(p, q - p);

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
    while (p < text.length() && isspace((unsigned int)text[p])) {
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
    bool overflow = false;

    float width = initial_width;
    while (q < text.length() && text[q] != '\n') {
      if (isspacew(text[q])) {
        any_spaces = true;
      }

      width += calc_width(text[q]);
      q++;

      if (width > wordwrap_width) {
        // Oops, too many.
        q--;
        overflow = true;
        break;
      }
    }
    
    if (overflow && any_spaces) {
      // If we stopped because we exceeded the wordwrap width, then
      // back up to the end of the last complete word.
      while (q > p && !isspacew(text[q])) {
        q--;
      }
    }

    // Skip additional whitespace between the lines.
    size_t next_start = q;
    while (next_start < text.length() && isblank(text[next_start])) {
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
      while (next_start < text.length() && isblank(text[next_start])) {
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
    output_text += text.substr(p, q - p);

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
