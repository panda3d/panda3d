// Filename: textAssembler.cxx
// Created by:  drose (06Apr04)
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

#include "textAssembler.h"
#include "textGlyph.h"
#include "cullFaceAttrib.h"
#include "colorAttrib.h"
#include "cullBinAttrib.h"
#include "textureAttrib.h"
#include "transparencyAttrib.h"
#include "textPropertiesManager.h"
#include "textEncoder.h"
#include "config_text.h"

#include <ctype.h>
  
#ifndef CPPPARSER  // interrogate has a bit of trouble with wstring.



// This is the factor by which CT_small scales the character down.
static const float small_accent_scale = 0.6f;

// This is the factor by which CT_tiny scales the character down.
static const float tiny_accent_scale = 0.4f;

// This is the factor by which CT_squash scales the character in X and Y.
static const float squash_accent_scale_x = 0.8f;
static const float squash_accent_scale_y = 0.5f;

// This is the factor by which CT_small_squash scales the character in X and Y.
static const float small_squash_accent_scale_x = 0.6f;
static const float small_squash_accent_scale_y = 0.3f;

// This is the factor by which the advance is reduced for the first
// character of a two-character ligature.
static const float ligature_advance_scale = 0.6f;


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
//     Function: isbreakpoint
//  Description: An internal function, similar to isspace(), except it
//               does not consider newlines to be whitespace.  It also
//               includes the soft-hyphen character.
////////////////////////////////////////////////////////////////////
static INLINE bool
isbreakpoint(unsigned int ch) {
  return (ch == ' ' || ch == '\t' || 
          ch == (unsigned int)text_soft_hyphen_key ||
          ch == (unsigned int)text_soft_break_key);
}


////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::Constructor
//       Access: Public
//  Description: Places all of the indicated text according to the
//               associated TextProperties.
////////////////////////////////////////////////////////////////////
TextAssembler::
TextAssembler(TextEncoder *encoder) : _encoder(encoder) {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TextAssembler::
~TextAssembler() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::clear
//       Access: Public
//  Description: Reinitializes the contents of the TextAssembler.
////////////////////////////////////////////////////////////////////
void TextAssembler::
clear() {
  _num_rows = 0;
  _ul.set(0.0f, 0.0f);
  _lr.set(0.0f, 0.0f);

  _wordwrapped_string.clear();

  PropertiesList::iterator li;
  for (li = _properties_list.begin(); li != _properties_list.end(); ++li) {
    delete (*li);
  }
  _properties_list.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::set_wtext
//       Access: Public
//  Description: Accepts a new text string and associated properties
//               structure, and precomputes the wordwrapping layout
//               appropriately.  After this call,
//               get_wordwrapped_wtext() and get_num_rows() can be
//               called.
//
//               If max_rows is greater than zero, no more than
//               max_rows will be accepted.  Text beyond that will be
//               truncated.
//
//               The return value is true if all the text is accepted,
//               or false if some was truncated.
////////////////////////////////////////////////////////////////////
bool TextAssembler::
set_wtext(const wstring &wtext, const TextProperties &properties,
          int max_rows) {
  clear();

  // First, expand all of the embedded TextProperties references
  // within the string.
  TextString text_string;
  wstring::const_iterator si = wtext.begin();
  scan_wtext(si, wtext.end(), &properties, text_string);

  while (si != wtext.end()) {
    // If we returned without consuming the whole string, it means
    // there was an embedded text_pop_properties_key that didn't match
    // the push.  That's worth a warning, and then go back and pick up
    // the rest of the string.
    text_cat.warning()
      << "pop_properties encountered without preceding push_properties.\n";
    scan_wtext(si, wtext.end(), &properties, text_string);
  }

  // Then apply any wordwrap requirements.
  return wordwrap_text(text_string, _wordwrapped_string, max_rows);
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::get_wordwrapped_wtext
//       Access: Public
//  Description: Returns a wstring that represents the contents of the
//               text, as it has been formatted by wordwrap rules.
//               This will not contain any embedded special characters
//               like \1 or \3.
////////////////////////////////////////////////////////////////////
wstring TextAssembler::
get_wordwrapped_wtext() const {
  wstring wtext;

  TextString::const_iterator ti;
  for (ti = _wordwrapped_string.begin(); 
       ti != _wordwrapped_string.end(); 
       ++ti) {
    wtext += (*ti)._character;
  }

  return wtext;
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::assemble_text
//       Access: Public
//  Description: Actually assembles all of the text into a GeomNode,
//               and returns the node (or possibly a parent of the
//               node, to keep the shadow separate).  Once this has
//               been called, you may query the extents of the text
//               via get_ul(), get_lr().
////////////////////////////////////////////////////////////////////
PT(PandaNode) TextAssembler::
assemble_text() {
  // Now assemble the text into glyphs.
  PlacedGlyphs placed_glyphs;
  assemble_paragraph(_wordwrapped_string.begin(), _wordwrapped_string.end(), 
                     placed_glyphs);

  // Now that we have a bunch of GlyphPlacements, pull out the Geoms
  // and put them under a common node.
  PT(PandaNode) parent_node = new PandaNode("common");

  PT(GeomNode) shadow_node = new GeomNode("shadow");
  PT(GeomNode) text_node = new GeomNode("text");

  const TextProperties *properties = NULL;
  CPT(RenderState) text_state;
  CPT(RenderState) shadow_state;
  LMatrix4f shadow_xform;

  bool any_shadow = false;
  
  PlacedGlyphs::const_iterator pgi;
  for (pgi = placed_glyphs.begin(); pgi != placed_glyphs.end(); ++pgi) {
    const GlyphPlacement *placement = (*pgi);

    if (placement->_properties != properties) {
      // Get a new set of properties for future glyphs.
      properties = placement->_properties;
      text_state = RenderState::make_empty();
      shadow_state = RenderState::make_empty();
      shadow_xform = LMatrix4f::ident_mat();

      if (properties->has_text_color()) {
        text_state = text_state->add_attrib(ColorAttrib::make_flat(properties->get_text_color()));
        if (properties->get_text_color()[3] != 1.0) {
          text_state = text_state->add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha));
        }
      }

      if (properties->has_bin()) {
        text_state = text_state->add_attrib(CullBinAttrib::make(properties->get_bin(), properties->get_draw_order() + 2));
      }

      if (properties->has_shadow()) {
        if (properties->has_shadow_color()) {
          shadow_state = shadow_state->add_attrib(ColorAttrib::make_flat(properties->get_shadow_color()));
          if (properties->get_shadow_color()[3] != 1.0) {
            shadow_state = shadow_state->add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha));
          }
        }

        if (properties->has_bin()) {
          shadow_state = shadow_state->add_attrib(CullBinAttrib::make(properties->get_bin(), properties->get_draw_order() + 1));
        }

        LVector2f offset = properties->get_shadow();
        shadow_xform = LMatrix4f::translate_mat(offset[0], 0.0f, -offset[1]);
      }
    }

    // We have to place the shadow first, because it copies as it
    // goes, while the place-text function just stomps on the
    // vertices.
    if (properties->has_shadow()) {
      placement->assign_copy_to(shadow_node, shadow_state, shadow_xform);
      any_shadow = true;
    }
    placement->assign_to(text_node, text_state);
    delete placement;
  }  
  placed_glyphs.clear();

  if (any_shadow) {
    // The shadow_node must appear first to guarantee the correct
    // rendering order.
    parent_node->add_child(shadow_node);
  }
  parent_node->add_child(text_node);

  return parent_node;
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::calc_width
//       Access: Private, Static
//  Description: Returns the width of a single character, according to
//               its associated font.  This also correctly calculates
//               the width of cheesy ligatures and accented
//               characters, which may not exist in the font as such.
////////////////////////////////////////////////////////////////////
float TextAssembler::
calc_width(wchar_t character, const TextProperties &properties) {
  if (character == ' ') {
    // A space is a special case.
    TextFont *font = properties.get_font();
    nassertr(font != (TextFont *)NULL, 0.0f);
    return font->get_space_advance();
  }

  bool got_glyph;
  const TextGlyph *first_glyph;
  const TextGlyph *second_glyph;
  UnicodeLatinMap::AccentType accent_type;
  int additional_flags;
  float glyph_scale;
  float advance_scale;
  get_character_glyphs(character, &properties, 
                       got_glyph, first_glyph, second_glyph, accent_type,
                       additional_flags, glyph_scale, advance_scale);

  float advance = 0.0f;
  
  if (first_glyph != (TextGlyph *)NULL) {
    advance = first_glyph->get_advance() * advance_scale;
  }
  if (second_glyph != (TextGlyph *)NULL) {
    advance += second_glyph->get_advance();
  }

  glyph_scale *= properties.get_glyph_scale();

  return advance * glyph_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::scan_wtext
//       Access: Private
//  Description: Scans through the text string, decoding embedded
//               references to TextProperties.  The string is copied
//               character-by-character into the indicated TextString
//               output.
//
//               As new TextProperties are discovered, TextProperties
//               structures are allocated and pushed into the
//               _properties_list list; these pointers are
//               referenced by the text_string.  When the text_string
//               is no longer need, the TextProperties structures in
//               _properties_list should be deleted to free up
//               memory.
////////////////////////////////////////////////////////////////////
void TextAssembler::
scan_wtext(wstring::const_iterator &si, 
           const wstring::const_iterator &send,
           const TextProperties *current_properties,
           TextAssembler::TextString &text_string) {
  while (si != send) {
    if ((*si) == text_push_properties_key) {
      // This indicates a nested properties structure.  Pull off the
      // name of the TextProperties structure, which is everything
      // until the next text_push_properties_key.
      wstring wname;
      ++si;
      while (si != send && (*si) != text_push_properties_key) {
        wname += (*si);
        ++si;
      }

      if (si == send) {
        // We didn't close the text_push_properties_key.  That's an
        // error.
        text_cat.warning()
          << "Unclosed push_properties in text.\n";
        return;
      }

      ++si;

      // Now we have to encode the wstring into a string, for lookup
      // in the TextPropertiesManager.
      string name = _encoder->encode_wtext(wname);
      
      TextPropertiesManager *manager = 
        TextPropertiesManager::get_global_ptr();
      
      // Define the new properties by extending the current properties.
      TextProperties *new_properties = new TextProperties(*current_properties);
      new_properties->add_properties(manager->get_properties(name));
      _properties_list.push_back(new_properties);
      
      // And recursively scan with the nested properties.
      scan_wtext(si, send, new_properties, text_string);

      if (text_cat.is_debug()) {
        if (si == send) {
          // The push was not closed by a pop.  That's not an error,
          // since we allow people to be sloppy about that; but we'll
          // print a debug message at least.
          text_cat.debug()
            << "push_properties not matched by pop_properties.\n";
        }
      }

    } else if ((*si) == text_pop_properties_key) {
      // This indicates the undoing of a previous push_properties_key.
      // We simply return to the previous level.
      ++si;
      return;

    } else {
      // A normal character.  Apply it.
      text_string.push_back(TextCharacter(*si, current_properties));
      ++si;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::wordwrap_text
//       Access: Private
//  Description: Inserts newlines into the given text at the
//               appropriate places in order to make each line be the
//               longest possible line that is not longer than
//               wordwrap_width (and does not break any words, if
//               possible).
//
//               If max_rows is greater than zero, no more than
//               max_rows will be accepted.  Text beyond that will be
//               truncated.
//
//               The return value is true if all the text is accepted,
//               or false if some was truncated.
////////////////////////////////////////////////////////////////////
bool TextAssembler::
wordwrap_text(const TextAssembler::TextString &text,
              TextAssembler::TextString &output_text,
              int max_rows) {
  if (text.empty()) {
    // A special case: empty text means no rows.
    _num_rows = 0;
    return true;
  }

  size_t p = 0;
  _num_rows = 1;

  // Preserve any initial whitespace and newlines.
  float initial_width = 0.0f;
  while (p < text.size() && isspacew(text[p]._character)) {
    if (text[p]._character == '\n') {
      initial_width = 0.0f;
      if (max_rows > 0 && _num_rows >= max_rows) {
        // Truncate.
        return false;
      }
      _num_rows++;
    } else {
      initial_width += calc_width(text[p]);
    }
    output_text.push_back(text[p]);
    p++;
  }
  bool needs_newline = false;

  while (p < text.size()) {
    nassertr(!isspacew(text[p]._character), false);

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
    float wordwrap_width = -1.0f;

    bool last_was_space = false;
    float width = initial_width;
    while (q < text.size() && text[q]._character != '\n') {
      if (text[q]._properties->has_wordwrap()) {
        wordwrap_width = text[q]._properties->get_wordwrap();
      } else {
        wordwrap_width = -1.0f;
      }

      if (isspacew(text[q]._character) || 
          text[q]._character == text_soft_break_key) {
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
      // at which we might hyphenate a word if we need to.
      if (text[q]._character == text_soft_hyphen_key && 
          wordwrap_width > 0.0f) {
        // We only consider this as a possible hyphenation point if
        // (a) it is not the very first character, and (b) there is
        // enough room for a hyphen character to be printed following
        // it.
        if (q != p && width + calc_hyphen_width(text[q]) <= wordwrap_width) {
          any_hyphens = true;
          last_hyphen = q;
        }

      } else if (text[q]._character != text_soft_break_key) {
        // Some normal, printable character.
        width += calc_width(text[q]);
      }

      q++;
        
      if (wordwrap_width > 0.0f && width > wordwrap_width) {
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
      nassertr(wordwrap_width > 0.0f, false);

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

      } else {
        // Otherwise, this is a forced break.  Accept the longest line
        // we can that does not leave the next line beginning with one
        // of our forbidden characters.
        size_t i = 0;
        while ((int)i < text_max_never_break && q - i > p && 
               text_never_break_before->find(text[q - i]._character) != wstring::npos) {
          i++;
        }
        if ((int)i < text_max_never_break) {
          q -= i;
        }
      }
    }

    // Skip additional whitespace between the lines.
    size_t next_start = q;
    while (next_start < text.size() && 
           isbreakpoint(text[next_start]._character)) {
      next_start++;
    }

    // Trim off any more blanks on the end.
    while (q > p && isspacew(text[q - 1]._character)) {
      q--;
    }

    if (next_start == p) {
      // No characters got in at all.  This could only happen if the
      // wordwrap width is narrower than a single character, or if we
      // have a substantial number of leading spaces in a line.

      if (initial_width == 0.0f) {
        // There was no leading whitespace on the line, so the
        // character itself didn't fit within the margins.  Let it in
        // anyway; what else can we do?
        q++;
        next_start++;
        while (next_start < text.size() && 
               isbreakpoint(text[next_start]._character)) {
          next_start++;
        }
      }
    }
    
    if (needs_newline) {
      if (max_rows > 0 && _num_rows >= max_rows) {
        // Truncate.
        return false;
      }
      _num_rows++;
      output_text.push_back(TextCharacter('\n', text[next_start - 1]._properties));
    }
    needs_newline = true;

    if (text[next_start - 1]._properties->get_preserve_trailing_whitespace()) {
      q = next_start;
    }

    for (size_t pi = p; pi < q; pi++) {
      if (text[pi]._character != text_soft_hyphen_key && 
          text[pi]._character != text_soft_break_key) {
        output_text.push_back(text[pi]);
      }
    }
    if (output_hyphen) {
      wstring::const_iterator wi;
      for (wi = text_soft_hyphen_output->begin();
           wi != text_soft_hyphen_output->end();
           ++wi) {
        output_text.push_back(TextCharacter(*wi, text[last_hyphen]._properties));
      }
    }

    // Now prepare to wrap the next line.

    if (next_start < text.size() && text[next_start]._character == '\n') {
      // Preserve a single embedded newline.
      if (max_rows > 0 && _num_rows >= max_rows) {
        // Truncate.
        return false;
      }
      _num_rows++;
      output_text.push_back(text[next_start]);
      next_start++;
      needs_newline = false;
    }
    p = next_start;

    // Preserve any initial whitespace and newlines.
    initial_width = 0.0f;
    while (p < text.size() && isspacew(text[p]._character)) {
      if (text[p]._character == '\n') {
        initial_width = 0.0f;
        if (max_rows > 0 && _num_rows >= max_rows) {
          // Truncate.
          return false;
        }
        _num_rows++;
      } else {
        initial_width += calc_width(text[p]);
      }
      output_text.push_back(text[p]);
      p++;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::calc_hyphen_width
//       Access: Private, Static
//  Description: Returns the width of the soft-hyphen replacement
//               string, according to the indicated character's
//               associated font.
////////////////////////////////////////////////////////////////////
float TextAssembler::
calc_hyphen_width(const TextCharacter &tch) {
  TextFont *font = tch._properties->get_font();
  nassertr(font != (TextFont *)NULL, 0.0f);

  float hyphen_width = 0.0f;
  wstring::const_iterator wi;
  for (wi = text_soft_hyphen_output->begin();
       wi != text_soft_hyphen_output->end();
       ++wi) {
    hyphen_width += calc_width(*wi, *tch._properties);
  }
  
  return hyphen_width;
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::assemble_paragraph
//       Access: Private
//  Description: Fills up _placed_glyphs, _ul, _lr, and _num_rows with
//               the contents of the indicated text.
////////////////////////////////////////////////////////////////////
void TextAssembler::
assemble_paragraph(TextAssembler::TextString::const_iterator si, 
                   const TextAssembler::TextString::const_iterator &send,
                   TextAssembler::PlacedGlyphs &placed_glyphs) {
  _ul.set(0.0f, 0.0f);
  _lr.set(0.0f, 0.0f);
  int num_rows = 0;

  float posy = 0.0f;
  while (si != send) {
    // First, assemble all the glyphs of this row.
    PlacedGlyphs row_placed_glyphs;
    float row_width, line_height;
    TextProperties::Alignment align;
    assemble_row(si, send, row_placed_glyphs,
                 row_width, line_height, align);

    // Now move the row to its appropriate position.  This might
    // involve a horizontal as well as a vertical translation.
    LMatrix4f mat = LMatrix4f::ident_mat();

    if (num_rows == 0) {
      // If this is the first row, account for its space.
      _ul[1] = 0.8f * line_height;

    } else {
      // If it is not the first row, shift the text downward by
      // line_height from the previous row.
      posy -= line_height;
    }
    _lr[1] = posy - 0.2f * line_height;

    // Apply the requested horizontal alignment to the row.
    switch (align) {
    case TextProperties::A_left:
      mat.set_row(3, LVector3f(0.0f, 0.0f, posy));
      _lr[0] = max(_lr[0], row_width);
      break;

    case TextProperties::A_right:
      mat.set_row(3, LVector3f(-row_width, 0.0f, posy));
      _ul[0] = min(_ul[0], -row_width);
      break;

    case TextProperties::A_center:
      {
        float half_row_width = 0.5f * row_width;
        mat.set_row(3, LVector3f(-half_row_width, 0.0f, posy));
        _lr[0] = max(_lr[0], half_row_width);
        _ul[0] = min(_ul[0], -half_row_width);
      }
      break;
    }

    // Now store the geoms we assembled.
    PlacedGlyphs::iterator pi;
    for (pi = row_placed_glyphs.begin(); pi != row_placed_glyphs.end(); ++pi) {
      (*pi)->_xform *= mat;
      placed_glyphs.push_back(*pi);
    }

    // Advance to the next line.
    num_rows++;
  }

  // num_rows may be smaller than _num_rows, if there are trailing
  // newlines on the string.
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::assemble_row
//       Access: Private
//  Description: Assembles the letters in the source string, up until
//               the first newline or the end of the string into a
//               single row (which is parented to _geom_node), and
//               computes the length of the row and the maximum
//               line_height of all the fonts used in the row.  The
//               source pointer is moved to the terminating character.
////////////////////////////////////////////////////////////////////
void TextAssembler::
assemble_row(TextAssembler::TextString::const_iterator &si, 
             const TextAssembler::TextString::const_iterator &send, 
             TextAssembler::PlacedGlyphs &row_placed_glyphs,
             float &row_width, float &line_height, 
             TextProperties::Alignment &align) {
  line_height = 0.0f;
  float xpos = 0.0f;
  align = TextProperties::A_left;

  while (si != send) {
    wchar_t character = (*si)._character;
    const TextProperties *properties = (*si)._properties;

    TextFont *font = properties->get_font();
    nassertv(font != (TextFont *)NULL);

    // We get the row's alignment property from that of the last
    // character to be placed in the row (or the newline character).
    align = properties->get_align();

    // And the height of the row is the maximum of all the fonts used
    // within the row.
    line_height = max(line_height, font->get_line_height());

    if (character == '\n') {
      // The newline character marks the end of the row.
      row_width = xpos;
      ++si;
      return;
    }

    if (character == ' ') {
      // A space is a special case.
      xpos += font->get_space_advance();

    } else if (character == '\t') {
      // So is a tab character.
      float tab_width = properties->get_tab_width();
      xpos = (floor(xpos / tab_width) + 1.0f) * tab_width;

    } else if (character == text_soft_hyphen_key) {
      // And so is the 'soft-hyphen' key character.

    } else {
      // A printable character.
      bool got_glyph;
      const TextGlyph *first_glyph;
      const TextGlyph *second_glyph;
      UnicodeLatinMap::AccentType accent_type;
      int additional_flags;
      float glyph_scale;
      float advance_scale;
      get_character_glyphs(character, properties, 
                           got_glyph, first_glyph, second_glyph, accent_type,
                           additional_flags, glyph_scale, advance_scale);

      if (!got_glyph) {
        text_cat.warning()
          << "No definition in " << font->get_name() 
          << " for character " << character;
        if (character < 128 && isprint((unsigned int)character)) {
          text_cat.warning(false)
            << " ('" << (char)character << "')";
        }
        text_cat.warning(false)
          << "\n";
      }

      // Build up a GlyphPlacement, indicating all of the Geoms that go
      // into this character.  Normally, there is only one Geom per
      // character, but it may involve multiple Geoms if we need to
      // add cheesy accents or ligatures.
      GlyphPlacement *placement = new GlyphPlacement;
      row_placed_glyphs.push_back(placement);

      float advance = 0.0f;

      if (first_glyph != (TextGlyph *)NULL) {
        PT(Geom) first_char_geom = first_glyph->get_geom();
        if (first_char_geom != (Geom *)NULL) {
          placement->add_piece(first_char_geom, first_glyph->get_state());
        }
        advance = first_glyph->get_advance() * advance_scale;
      }
      if (second_glyph != (TextGlyph *)NULL) {
        PT(Geom) second_char_geom = second_glyph->get_geom();
        if (second_char_geom != (Geom *)NULL) {
          second_char_geom->transform_vertices(LMatrix4f::translate_mat(advance, 0.0f, 0.0f));
          placement->add_piece(second_char_geom, second_glyph->get_state());
        }
        advance += second_glyph->get_advance();
      }

      glyph_scale *= properties->get_glyph_scale();

      // Now compute the matrix that will transform the glyph (or
      // glyphs) into position.
      LMatrix4f glyph_xform = LMatrix4f::scale_mat(glyph_scale);

      if (accent_type != UnicodeLatinMap::AT_none || additional_flags != 0) {
        // If we have some special handling to perform, do so now.
        // This will probably require the bounding volume of the
        // glyph, so go get that.
        LPoint3f min_vert, max_vert;
        bool found_any = false;
        placement->calc_tight_bounds(min_vert, max_vert, found_any);

        if (found_any) {
          LPoint3f centroid = (min_vert + max_vert) / 2.0f;
          tack_on_accent(accent_type, min_vert, max_vert, centroid, 
                         properties, placement);
    
          if ((additional_flags & UnicodeLatinMap::AF_turned) != 0) {
            // Invert the character.  Should we also invert the accent
            // mark, so that an accent that would have been above the
            // glyph will now be below it?  That's what we do here,
            // which is probably the right thing to do for n-tilde,
            // but not for most of the rest of the accent marks.  For
            // now we'll assume there are no characters with accent
            // marks that also have the turned flag.

            // We rotate the character around its centroid, which may
            // not always be the right point, but it's the best we've
            // got and it's probably pretty close.
            LMatrix4f rotate =
              LMatrix4f::translate_mat(-centroid) *
              LMatrix4f::rotate_mat_normaxis(180.0f, LVecBase3f(0.0f, -1.0f, 0.0f)) *
              LMatrix4f::translate_mat(centroid);
            glyph_xform *= rotate;
          }
        }
      }

      glyph_xform(3, 0) += xpos;
      glyph_xform(3, 2) += properties->get_glyph_shift();

      if (properties->has_slant()) {
        LMatrix4f shear(1.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.0f,
                        properties->get_slant(), 0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 1.0f);
        glyph_xform = shear * glyph_xform;
      }
      
      placement->_xform = glyph_xform;
      placement->_properties = properties;

      xpos += advance * glyph_scale;
    }
    ++si;
  }

  row_width = xpos;
}
  

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::get_character_glyphs
//       Access: Private, Static
//  Description: Looks up the glyph(s) from the font for the
//               appropriate character.  If the desired glyph isn't
//               available (especially in the case of an accented
//               letter), tries to find a suitable replacement.
//               Normally, only one glyph is returned per character,
//               but in the case in which we have to simulate a
//               missing ligature in the font, two glyphs might be
//               returned.
//
//               All parameters except the first two are output
//               parameters.  got_glyph is set true if the glyph (or
//               an acceptable substitute) is successfully found,
//               false otherwise; but even if it is false, glyph might
//               still be non-NULL, indicating a stand-in glyph for a
//               missing character.
////////////////////////////////////////////////////////////////////
void TextAssembler::
get_character_glyphs(int character, const TextProperties *properties,
                     bool &got_glyph, const TextGlyph *&glyph,
                     const TextGlyph *&second_glyph,
                     UnicodeLatinMap::AccentType &accent_type,
                     int &additional_flags,
                     float &glyph_scale, float &advance_scale) {
  TextFont *font = properties->get_font();
  nassertv(font != (TextFont *)NULL);

  got_glyph = false;
  glyph = NULL;
  second_glyph = NULL;
  accent_type = UnicodeLatinMap::AT_none;
  additional_flags = 0;
  glyph_scale = 1.0f;
  advance_scale = 1.0f;

  // Maybe we should remap the character to something else--e.g. a
  // small capital.
  const UnicodeLatinMap::Entry *map_entry = 
    UnicodeLatinMap::look_up(character);
  if (map_entry != NULL) {
    if (properties->get_small_caps() && 
        map_entry->_toupper_character != character) {
      character = map_entry->_toupper_character;
      map_entry = UnicodeLatinMap::look_up(character);
      glyph_scale = properties->get_small_caps_scale();
    }
  }
  
  got_glyph = font->get_glyph(character, glyph);
  if (!got_glyph && map_entry != NULL && map_entry->_ascii_equiv != 0) {
    // If we couldn't find the Unicode glyph, try the ASCII
    // equivalent (without the accent marks).
    got_glyph = font->get_glyph(map_entry->_ascii_equiv, glyph);
    
    if (!got_glyph && map_entry->_toupper_character != character) {
      // If we still couldn't find it, try the uppercase
      // equivalent.
      character = map_entry->_toupper_character;
      map_entry = UnicodeLatinMap::look_up(character);
      if (map_entry != NULL) {
        got_glyph = font->get_glyph(map_entry->_ascii_equiv, glyph);
      }
    }
    
    if (got_glyph) {
      accent_type = map_entry->_accent_type;
      additional_flags = map_entry->_additional_flags;
      
      bool got_second_glyph = false;
      if (map_entry->_ascii_additional != 0) {
        // There's another character, too--probably a ligature.
        got_second_glyph = 
          font->get_glyph(map_entry->_ascii_additional, second_glyph);
      }
      
      if ((additional_flags & UnicodeLatinMap::AF_ligature) != 0 &&
          got_second_glyph) {
        // If we have two letters that are supposed to be in a
        // ligature, just jam them together.
        additional_flags &= ~UnicodeLatinMap::AF_ligature;
        advance_scale = ligature_advance_scale;
      }
      
      if ((additional_flags & UnicodeLatinMap::AF_smallcap) != 0) {
        additional_flags &= ~UnicodeLatinMap::AF_smallcap;
        glyph_scale = properties->get_small_caps_scale();
      }
    }
  }
}
  
  
////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::tack_on_accent
//       Access: Private, Static
//  Description: This is a cheesy attempt to tack on an accent to an
//               ASCII letter for which we don't have the appropriate
//               already-accented glyph in the font.
////////////////////////////////////////////////////////////////////
void TextAssembler::
tack_on_accent(UnicodeLatinMap::AccentType accent_type,
               const LPoint3f &min_vert, const LPoint3f &max_vert,
               const LPoint3f &centroid,
               const TextProperties *properties, 
               TextAssembler::GlyphPlacement *placement) {
  switch (accent_type) {
  case UnicodeLatinMap::AT_grave:
    // We use the slash as the grave and acute accents.  ASCII does
    // have a grave accent character, but a lot of fonts put the
    // reverse apostrophe there instead.  And some fonts (particularly
    // fonts from mf) don't even do backslash.
    tack_on_accent('/', CP_above, CT_small_squash_mirror_y, min_vert, max_vert, centroid,
                   properties, placement);
    break;

  case UnicodeLatinMap::AT_acute:
    tack_on_accent('/', CP_above, CT_small_squash, min_vert, max_vert, centroid,
                   properties, placement);
    break;

  case UnicodeLatinMap::AT_breve:
    tack_on_accent(')', CP_above, CT_tiny_rotate_270, min_vert, max_vert, centroid,
                   properties, placement);
    break;

  case UnicodeLatinMap::AT_inverted_breve:
    tack_on_accent('(', CP_above, CT_tiny_rotate_270, min_vert, max_vert, centroid,
                   properties, placement);
    break;

  case UnicodeLatinMap::AT_circumflex:
    tack_on_accent('^', CP_above, CT_none, min_vert, max_vert, centroid,
                   properties, placement) ||
      tack_on_accent('v', CP_above, CT_squash_mirror_y, min_vert, max_vert, centroid,
                     properties, placement);
    break;

  case UnicodeLatinMap::AT_circumflex_below:
    tack_on_accent('^', CP_below, CT_none, min_vert, max_vert, centroid,
                   properties, placement) ||
      tack_on_accent('v', CP_below, CT_squash_mirror_y, min_vert, max_vert, centroid,
                     properties, placement);
    break;

  case UnicodeLatinMap::AT_caron:
    tack_on_accent('^', CP_above, CT_mirror_y, min_vert, max_vert, centroid,
                   properties, placement) ||
      tack_on_accent('v', CP_above, CT_squash, min_vert, max_vert, centroid,
                     properties, placement);

    break;

  case UnicodeLatinMap::AT_tilde:
    tack_on_accent('~', CP_above, CT_none, min_vert, max_vert, centroid,
                   properties, placement) ||
      tack_on_accent('s', CP_above, CT_squash_mirror_diag, min_vert, max_vert, centroid,
                     properties, placement);

    break;

  case UnicodeLatinMap::AT_tilde_below:
    tack_on_accent('~', CP_below, CT_none, min_vert, max_vert, centroid,
                   properties, placement) ||
      tack_on_accent('s', CP_below, CT_squash_mirror_diag, min_vert, max_vert, centroid,
                     properties, placement);
    break;

  case UnicodeLatinMap::AT_diaeresis:
    tack_on_accent(':', CP_above, CT_small_rotate_270, min_vert, max_vert, centroid,
                   properties, placement);
    break;

  case UnicodeLatinMap::AT_diaeresis_below:
    tack_on_accent(':', CP_below, CT_small_rotate_270, min_vert, max_vert, centroid,
                   properties, placement);
    break;

  case UnicodeLatinMap::AT_dot_above:
    tack_on_accent('.', CP_above, CT_none, min_vert, max_vert, centroid,
                   properties, placement);
    break;

  case UnicodeLatinMap::AT_dot_below:
    tack_on_accent('.', CP_below, CT_none, min_vert, max_vert, centroid,
                   properties, placement);
    break;

  case UnicodeLatinMap::AT_macron:
    tack_on_accent('-', CP_above, CT_none, min_vert, max_vert, centroid,
                   properties, placement);
    break;

  case UnicodeLatinMap::AT_line_below:
    tack_on_accent('-', CP_below, CT_none, min_vert, max_vert, centroid,
                   properties, placement);
    break;

  case UnicodeLatinMap::AT_ring_above:
    tack_on_accent('o', CP_top, CT_tiny, min_vert, max_vert, centroid,
                   properties, placement);
    break;

  case UnicodeLatinMap::AT_ring_below:
    tack_on_accent('o', CP_bottom, CT_tiny, min_vert, max_vert, centroid,
                   properties, placement);
    break;

  case UnicodeLatinMap::AT_cedilla:
    tack_on_accent('c', CP_bottom, CT_tiny_mirror_x, min_vert, max_vert, centroid,
                   properties, placement);
    //tack_on_accent(',', CP_bottom, CT_none, min_vert, max_vert, centroid,
    //               properties, placement);
    break;

  case UnicodeLatinMap::AT_comma_below:
    tack_on_accent(',', CP_below, CT_none, min_vert, max_vert, centroid,
                   properties, placement);
    break;

  case UnicodeLatinMap::AT_ogonek:
    tack_on_accent(',', CP_bottom, CT_mirror_x, min_vert, max_vert, centroid,
                   properties, placement);
    break;

  case UnicodeLatinMap::AT_stroke:
    tack_on_accent('/', CP_within, CT_none, min_vert, max_vert, centroid,
                   properties, placement);
    break;

  default:
    // There are lots of other crazy kinds of accents.  Forget 'em.
    break;
  }    
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::tack_on_accent
//       Access: Private, Static
//  Description: Generates a cheesy accent mark above (or below, etc.)
//               the character.  Returns true if successful, or false
//               if the named accent character doesn't exist in the
//               font.
////////////////////////////////////////////////////////////////////
bool TextAssembler::
tack_on_accent(char accent_mark, TextAssembler::CheesyPosition position,
               TextAssembler::CheesyTransform transform,
               const LPoint3f &min_vert, const LPoint3f &max_vert,
               const LPoint3f &centroid,
               const TextProperties *properties,
               TextAssembler::GlyphPlacement *placement) {
  TextFont *font = properties->get_font();
  nassertr(font != (TextFont *)NULL, false);
  
  const TextGlyph *accent_glyph;
  if (font->get_glyph(accent_mark, accent_glyph) ||
      font->get_glyph(toupper(accent_mark), accent_glyph)) {
    PT(Geom) accent_geom = accent_glyph->get_geom();
    if (accent_geom != (Geom *)NULL) {
      LPoint3f min_accent, max_accent;
      bool found_any = false;
      accent_geom->calc_tight_bounds(min_accent, max_accent, found_any);
      if (found_any) {
        float t, u;
        LMatrix4f accent_mat;

        // This gets set to true if the glyph gets mirrored and needs
        // to have backface culling disabled.
        bool mirrored = false;

        switch (transform) {
        case CT_none:
          accent_mat = LMatrix4f::ident_mat();
          break;

        case CT_mirror_x:
          accent_mat = LMatrix4f::scale_mat(-1.0f, 1.0f, 1.0f);
          t = min_accent[0];
          min_accent[0] = -max_accent[0];
          max_accent[0] = -t;
          mirrored = true;
          break;

        case CT_mirror_y:
          accent_mat = LMatrix4f::scale_mat(1.0f, 1.0f, -1.0f);
          t = min_accent[2];
          min_accent[2] = -max_accent[2];
          max_accent[2] = -t;
          mirrored = true;
          break;

        case CT_rotate_90:
          accent_mat =
            LMatrix4f::rotate_mat_normaxis(90.0f, LVecBase3f(0.0f, -1.0f, 0.0f));
          // rotate min, max
          t = min_accent[0];
          u = max_accent[0];
          max_accent[0] = -min_accent[2];
          min_accent[0] = -max_accent[2];
          max_accent[2] = u;
          min_accent[2] = t;
          break;

        case CT_rotate_180:
          accent_mat = LMatrix4f::scale_mat(-1.0f, -1.0f, 1.0f);
          
          t = min_accent[0];
          min_accent[0] = -max_accent[0];
          max_accent[0] = -t;
          t = min_accent[2];
          min_accent[2] = -max_accent[2];
          max_accent[2] = -t;
          break;

        case CT_rotate_270:
          accent_mat =
            LMatrix4f::rotate_mat_normaxis(270.0f, LVecBase3f(0.0f, -1.0f, 0.0f));
          // rotate min, max
          t = min_accent[0];
          u = max_accent[0];
          min_accent[0] = min_accent[2];
          max_accent[0] = max_accent[2];
          min_accent[2] = -u;
          max_accent[2] = -t;
          break;

        case CT_squash:
          accent_mat = LMatrix4f::scale_mat(squash_accent_scale_x, 1.0f, squash_accent_scale_y);
          min_accent[0] *= squash_accent_scale_x;
          max_accent[0] *= squash_accent_scale_x;
          min_accent[2] *= squash_accent_scale_y;
          max_accent[2] *= squash_accent_scale_y;
          break;

        case CT_squash_mirror_y:
          accent_mat = LMatrix4f::scale_mat(squash_accent_scale_x, 1.0f, -squash_accent_scale_y);
          min_accent[0] *= squash_accent_scale_x;
          max_accent[0] *= squash_accent_scale_x;
          t = min_accent[2];
          min_accent[2] = -max_accent[2] * squash_accent_scale_y;
          max_accent[2] = -t * squash_accent_scale_y;
          mirrored = true;
          break;

        case CT_squash_mirror_diag:
          accent_mat =
            LMatrix4f::rotate_mat_normaxis(270.0f, LVecBase3f(0.0f, -1.0f, 0.0f)) *
            LMatrix4f::scale_mat(-squash_accent_scale_x, 1.0f, squash_accent_scale_y);
          
          // rotate min, max
          t = min_accent[0];
          u = max_accent[0];
          min_accent[0] = min_accent[2] * -squash_accent_scale_x;
          max_accent[0] = max_accent[2] * -squash_accent_scale_x;
          min_accent[2] = -u * squash_accent_scale_y;
          max_accent[2] = -t * squash_accent_scale_y;
          mirrored = true;
          break;

        case CT_small_squash:
          accent_mat = LMatrix4f::scale_mat(small_squash_accent_scale_x, 1.0f, small_squash_accent_scale_y);
          min_accent[0] *= small_squash_accent_scale_x;
          max_accent[0] *= small_squash_accent_scale_x;
          min_accent[2] *= small_squash_accent_scale_y;
          max_accent[2] *= small_squash_accent_scale_y;
          break;

        case CT_small_squash_mirror_y:
          accent_mat = LMatrix4f::scale_mat(small_squash_accent_scale_x, 1.0f, -small_squash_accent_scale_y);
          min_accent[0] *= small_squash_accent_scale_x;
          max_accent[0] *= small_squash_accent_scale_x;
          t = min_accent[2];
          min_accent[2] = -max_accent[2] * small_squash_accent_scale_y;
          max_accent[2] = -t * small_squash_accent_scale_y;
          mirrored = true;
          break;

        case CT_small_squash_mirror_diag:
          accent_mat =
            LMatrix4f::rotate_mat_normaxis(270.0f, LVecBase3f(0.0f, -1.0f, 0.0f)) *
            LMatrix4f::scale_mat(-small_squash_accent_scale_x, 1.0f, small_squash_accent_scale_y);
          
          // rotate min, max
          t = min_accent[0];
          u = max_accent[0];
          min_accent[0] = min_accent[2] * -small_squash_accent_scale_x;
          max_accent[0] = max_accent[2] * -small_squash_accent_scale_x;
          min_accent[2] = -u * small_squash_accent_scale_y;
          max_accent[2] = -t * small_squash_accent_scale_y;
          mirrored = true;
          break;

        case CT_small:
          accent_mat = LMatrix4f::scale_mat(small_accent_scale);
          min_accent *= small_accent_scale;
          max_accent *= small_accent_scale;
          break;

        case CT_small_rotate_270:
          accent_mat =
            LMatrix4f::rotate_mat_normaxis(270.0f, LVecBase3f(0.0f, -1.0f, 0.0f)) *
            LMatrix4f::scale_mat(small_accent_scale);

          // rotate min, max
          t = min_accent[0];
          u = max_accent[0];
          min_accent[0] = min_accent[2] * small_accent_scale;
          max_accent[0] = max_accent[2] * small_accent_scale;
          min_accent[2] = -u * small_accent_scale;
          max_accent[2] = -t * small_accent_scale;
          break;

        case CT_tiny:
          accent_mat = LMatrix4f::scale_mat(tiny_accent_scale);
          min_accent *= tiny_accent_scale;
          max_accent *= tiny_accent_scale;
          break;

        case CT_tiny_mirror_x:
          accent_mat = LMatrix4f::scale_mat(-tiny_accent_scale, 1.0f, tiny_accent_scale);
          
          t = min_accent[0];
          min_accent[0] = -max_accent[0] * tiny_accent_scale;
          max_accent[0] = -t * tiny_accent_scale;
          min_accent[2] *= tiny_accent_scale;
          max_accent[2] *= tiny_accent_scale;
          mirrored = true;
          break;

        case CT_tiny_rotate_270:
          accent_mat =
            LMatrix4f::rotate_mat_normaxis(270.0f, LVecBase3f(0.0f, -1.0f, 0.0f)) *
            LMatrix4f::scale_mat(tiny_accent_scale);

          // rotate min, max
          t = min_accent[0];
          u = max_accent[0];
          min_accent[0] = min_accent[2] * tiny_accent_scale;
          max_accent[0] = max_accent[2] * tiny_accent_scale;
          min_accent[2] = -u * tiny_accent_scale;
          max_accent[2] = -t * tiny_accent_scale;
          break;
        }

        LPoint3f accent_centroid = (min_accent + max_accent) / 2.0f;
        float accent_height = max_accent[2] - min_accent[2];
        LVector3f trans;
        switch (position) {
        case CP_above:
          // A little above the character.
          trans.set(centroid[0] - accent_centroid[0], 0.0f,
                    max_vert[2] - accent_centroid[2] + accent_height * 0.5);
          break;

        case CP_below:
          // A little below the character.
          trans.set(centroid[0] - accent_centroid[0], 0.0f,
                    min_vert[2] - accent_centroid[2] - accent_height * 0.5);
          break;

        case CP_top:
          // Touching the top of the character.
          trans.set(centroid[0] - accent_centroid[0], 0.0f,
                    max_vert[2] - accent_centroid[2]);
          break;

        case CP_bottom:
          // Touching the bottom of the character.
          trans.set(centroid[0] - accent_centroid[0], 0.0f,
                    min_vert[2] - accent_centroid[2]);
          break;

        case CP_within:
          // Centered within the character.
          trans.set(centroid[0] - accent_centroid[0], 0.0f,
                    centroid[2] - accent_centroid[2]);
          break;
        }

        accent_mat.set_row(3, trans);
        accent_geom->transform_vertices(accent_mat);

        if (mirrored) {
          // Once someone asks for this pointer, we hold its reference
          // count and never free it.
          static CPT(RenderState) disable_backface;
          if (disable_backface == (const RenderState *)NULL) {
            disable_backface = RenderState::make
              (CullFaceAttrib::make(CullFaceAttrib::M_cull_none));
          }
            
          CPT(RenderState) state = 
            accent_glyph->get_state()->compose(disable_backface);
          placement->add_piece(accent_geom, state);
        } else {
          placement->add_piece(accent_geom, accent_glyph->get_state());
        }

        return true;
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::GlyphPlacement::calc_tight_bounds
//       Access: Private
//  Description: Expands min_point and max_point to include all of the
//               vertices in the glyph(s), if any.  found_any is set
//               true if any points are found.  It is the caller's
//               responsibility to initialize min_point, max_point,
//               and found_any before calling this function.
////////////////////////////////////////////////////////////////////
void TextAssembler::GlyphPlacement::
calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point,
                  bool &found_any) const {
  Pieces::const_iterator pi;
  for (pi = _pieces.begin(); pi != _pieces.end(); ++pi) {
    (*pi)._geom->calc_tight_bounds(min_point, max_point, found_any);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::GlyphPlacement::assign_to
//       Access: Private
//  Description: Puts the pieces of the GlyphPlacement in the
//               indicated GeomNode.  The vertices of the Geoms are
//               modified by this operation.
////////////////////////////////////////////////////////////////////
void TextAssembler::GlyphPlacement::
assign_to(GeomNode *geom_node, const RenderState *state) const {
  Pieces::const_iterator pi;
  for (pi = _pieces.begin(); pi != _pieces.end(); ++pi) {
    (*pi)._geom->transform_vertices(_xform);
    geom_node->add_geom((*pi)._geom, state->compose((*pi)._state));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::GlyphPlacement::assign_copy_to
//       Access: Private
//  Description: Puts the pieces of the GlyphPlacement in the
//               indicated GeomNode.  This flavor will make a copy of
//               the Geoms first, and then apply the additional
//               transform to the vertices.
////////////////////////////////////////////////////////////////////
void TextAssembler::GlyphPlacement::
assign_copy_to(GeomNode *geom_node, const RenderState *state,
               const LMatrix4f &extra_xform) const {
  LMatrix4f new_xform = _xform * extra_xform;
  Pieces::const_iterator pi;
  for (pi = _pieces.begin(); pi != _pieces.end(); ++pi) {
    PT(Geom) new_geom = (*pi)._geom->make_copy();
    new_geom->transform_vertices(new_xform);
    geom_node->add_geom(new_geom, state->compose((*pi)._state));
  }
}

#endif  // CPPPARSER

