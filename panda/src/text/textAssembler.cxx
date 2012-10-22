// Filename: textAssembler.cxx
// Created by:  drose (06Apr04)
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
#include "geomTriangles.h"
#include "geomLines.h"
#include "geomPoints.h"
#include "geomVertexReader.h"
#include "geomVertexWriter.h"
#include "geomLines.h"
#include "geomVertexFormat.h"
#include "geomVertexData.h"
#include "geom.h"
#include "modelNode.h"

#include <ctype.h>
#include <stdio.h>  // for sprintf
  
// This is the factor by which CT_small scales the character down.
static const PN_stdfloat small_accent_scale = 0.6f;

// This is the factor by which CT_tiny scales the character down.
static const PN_stdfloat tiny_accent_scale = 0.4;

// This is the factor by which CT_squash scales the character in X and Y.
static const PN_stdfloat squash_accent_scale_x = 0.8f;
static const PN_stdfloat squash_accent_scale_y = 0.5f;

// This is the factor by which CT_small_squash scales the character in X and Y.
static const PN_stdfloat small_squash_accent_scale_x = 0.6f;
static const PN_stdfloat small_squash_accent_scale_y = 0.3;

// This is the factor by which the advance is reduced for the first
// character of a two-character ligature.
static const PN_stdfloat ligature_advance_scale = 0.6f;


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
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
TextAssembler::
TextAssembler(TextEncoder *encoder) : 
  _encoder(encoder),
  _usage_hint(Geom::UH_static),
  _max_rows(0),
  _dynamic_merge(text_dynamic_merge),
  _multiline_mode(true)
{
  _initial_cprops = new ComputedProperties(TextProperties());
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
TextAssembler::
TextAssembler(const TextAssembler &copy) :
  _initial_cprops(copy._initial_cprops),
  _text_string(copy._text_string),
  _text_block(copy._text_block),
  _ul(copy._ul),
  _lr(copy._lr),
  _next_row_ypos(copy._next_row_ypos),
  _encoder(copy._encoder),
  _usage_hint(copy._usage_hint),
  _max_rows(copy._max_rows),
  _dynamic_merge(copy._dynamic_merge),
  _multiline_mode(copy._multiline_mode)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::Copy Assignment Operator
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void TextAssembler::
operator = (const TextAssembler &copy) {
  _initial_cprops = copy._initial_cprops;
  _text_string = copy._text_string;
  _text_block = copy._text_block;
  _ul = copy._ul;
  _lr = copy._lr;
  _next_row_ypos = copy._next_row_ypos;
  _encoder = copy._encoder;
  _usage_hint = copy._usage_hint;
  _max_rows = copy._max_rows;
  _dynamic_merge = copy._dynamic_merge;
  _multiline_mode = copy._multiline_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
TextAssembler::
~TextAssembler() {
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::clear
//       Access: Published
//  Description: Reinitializes the contents of the TextAssembler.
////////////////////////////////////////////////////////////////////
void TextAssembler::
clear() {
  _ul.set(0.0f, 0.0f);
  _lr.set(0.0f, 0.0f);
  _next_row_ypos = 0.0f;

  _text_string.clear();
  _text_block.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::set_wtext
//       Access: Published
//  Description: Accepts a new text string and associated properties
//               structure, and precomputes the wordwrapping layout
//               appropriately.  After this call,
//               get_wordwrapped_wtext() and get_num_rows() can be
//               called.
//
//               The return value is true if all the text is accepted,
//               or false if some was truncated (see set_max_rows()).
////////////////////////////////////////////////////////////////////
bool TextAssembler::
set_wtext(const wstring &wtext) {
  clear();

  // First, expand all of the embedded TextProperties references
  // within the string.
  wstring::const_iterator si = wtext.begin();
  scan_wtext(_text_string, si, wtext.end(), _initial_cprops);

  while (si != wtext.end()) {
    // If we returned without consuming the whole string, it means
    // there was an embedded text_pop_properties_key that didn't match
    // the push.  That's worth a warning, and then go back and pick up
    // the rest of the string.
    text_cat.warning()
      << "pop_properties encountered without preceding push_properties.\n";
    scan_wtext(_text_string, si, wtext.end(), _initial_cprops);
  }

  // Then apply any wordwrap requirements.
  return wordwrap_text();
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::set_wsubstr
//       Access: Published
//  Description: Replaces the 'count' characters from 'start' of the
//               current text with the indicated replacement text.  If
//               the replacement text does not have count characters,
//               the length of the string will be changed accordingly.
//
//               The substring may include nested formatting
//               characters, but they must be self-contained and
//               self-closed.  The formatting characters are not
//               literally saved in the internal string; they are
//               parsed at the time of the set_wsubstr() call.
//
//               The return value is true if all the text is accepted,
//               or false if some was truncated (see set_max_rows()).
////////////////////////////////////////////////////////////////////
bool TextAssembler::
set_wsubstr(const wstring &wtext, int start, int count) {
  nassertr(start >= 0 && start <= (int)_text_string.size(), false);
  nassertr(count >= 0 && start + count <= (int)_text_string.size(), false);

  // Use scan_wtext to unroll the substring we wish to insert, as in
  // set_wtext(), above.
  TextString substr;
  wstring::const_iterator si = wtext.begin();
  scan_wtext(substr, si, wtext.end(), _initial_cprops);
  while (si != wtext.end()) {
    text_cat.warning()
      << "pop_properties encountered without preceding push_properties.\n";
    scan_wtext(substr, si, wtext.end(), _initial_cprops);
  }

  _text_string.erase(_text_string.begin() + start, _text_string.begin() + start + count);
  _text_string.insert(_text_string.begin() + start, substr.begin(), substr.end());

  return wordwrap_text();
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::get_plain_wtext
//       Access: Published
//  Description: Returns a wstring that represents the contents of the
//               text, without any embedded properties characters.  If
//               there is an embedded graphic object, a zero value is
//               inserted in that position.
//
//               This string has the same length as
//               get_num_characters(), and the characters in this
//               string correspond one-to-one with the characters
//               returned by get_character(n).
////////////////////////////////////////////////////////////////////
wstring TextAssembler::
get_plain_wtext() const {
  wstring wtext;

  TextString::const_iterator si;
  for (si = _text_string.begin(); si != _text_string.end(); ++si) {
    const TextCharacter &tch = (*si);
    if (tch._graphic == (TextGraphic *)NULL) {
      wtext += tch._character;
    } else {
      wtext.push_back(0);
    }
  }
  
  return wtext;
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::get_wordwrapped_plain_wtext
//       Access: Published
//  Description: Returns a wstring that represents the contents of the
//               text, with newlines inserted according to the
//               wordwrapping.  The string will contain no embedded
//               properties characters.  If there is an embedded
//               graphic object, a zero value is inserted in that
//               position.
//
//               This string has the same number of newline characters
//               as get_num_rows(), and the characters in this string
//               correspond one-to-one with the characters returned by
//               get_character(r, c).
////////////////////////////////////////////////////////////////////
wstring TextAssembler::
get_wordwrapped_plain_wtext() const {
  wstring wtext;

  TextBlock::const_iterator bi;
  for (bi = _text_block.begin(); bi != _text_block.end(); ++bi) {
    const TextRow &row = (*bi);
    if (bi != _text_block.begin()) {
      wtext += '\n';
    }
      
    TextString::const_iterator si;
    for (si = row._string.begin(); si != row._string.end(); ++si) {
      const TextCharacter &tch = (*si);
      if (tch._graphic == (TextGraphic *)NULL) {
        wtext += tch._character;
      } else {
        wtext.push_back(0);
      }
    }
  }

  return wtext;
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::get_wtext
//       Access: Published
//  Description: Returns a wstring that represents the contents of the
//               text.
//
//               The string will contain embedded properties
//               characters, which may not exactly match the embedded
//               properties characters of the original string, but it
//               will encode the same way.
////////////////////////////////////////////////////////////////////
wstring TextAssembler::
get_wtext() const {
  wstring wtext;
  PT(ComputedProperties) current_cprops = _initial_cprops;

  TextString::const_iterator si;
  for (si = _text_string.begin(); si != _text_string.end(); ++si) {
    const TextCharacter &tch = (*si);
    current_cprops->append_delta(wtext, tch._cprops);
    if (tch._graphic == (TextGraphic *)NULL) {
      wtext += tch._character;
    } else {
      wtext.push_back(text_embed_graphic_key);
      wtext += tch._graphic_wname;
      wtext.push_back(text_embed_graphic_key);
    }
    current_cprops = tch._cprops;
  }
  current_cprops->append_delta(wtext, _initial_cprops);

  return wtext;
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::get_wordwrapped_wtext
//       Access: Published
//  Description: Returns a wstring that represents the contents of the
//               text, with newlines inserted according to the
//               wordwrapping.
//
//               The string will contain embedded properties
//               characters, which may not exactly match the embedded
//               properties characters of the original string, but it
//               will encode the same way.
//
//               Embedded properties characters will be closed before
//               every newline, then reopened (if necessary) on the
//               subsequent character following the newline.  This
//               means it will be safe to divide the text up at the
//               newline characters and treat each line as an
//               independent piece.
////////////////////////////////////////////////////////////////////
wstring TextAssembler::
get_wordwrapped_wtext() const {
  wstring wtext;

  PT(ComputedProperties) current_cprops = _initial_cprops;
  
  TextBlock::const_iterator bi;
  for (bi = _text_block.begin(); bi != _text_block.end(); ++bi) {
    const TextRow &row = (*bi);
    if (bi != _text_block.begin()) {
      current_cprops->append_delta(wtext, _initial_cprops);
      current_cprops = _initial_cprops;
      wtext += '\n';
    }
    
    TextString::const_iterator si;
    for (si = row._string.begin(); si != row._string.end(); ++si) {
      const TextCharacter &tch = (*si);
      current_cprops->append_delta(wtext, tch._cprops);
      if (tch._graphic == (TextGraphic *)NULL) {
        wtext += tch._character;
      } else {
        wtext.push_back(text_embed_graphic_key);
        wtext += tch._graphic_wname;
        wtext.push_back(text_embed_graphic_key);
      }
      current_cprops = tch._cprops;
    }
  }
  current_cprops->append_delta(wtext, _initial_cprops);

  return wtext;
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::calc_r_c
//       Access: Published
//  Description: Computes the row and column index of the nth
//               character or graphic object in the text.  Fills r and
//               c accordingly.
//
//               Returns true if the nth character is valid and has a
//               corresponding r and c position, false otherwise (for
//               instance, a soft-hyphen character, or a newline
//               character, may not have a corresponding position).
//               In either case, r and c will be filled in sensibly.
////////////////////////////////////////////////////////////////////
bool TextAssembler::
calc_r_c(int &r, int &c, int n) const {
  nassertr(n >= 0 && n <= (int)_text_string.size(), false);

  if (n == (int)_text_string.size()) {
    // A special case for one past the last character.
    if (_text_string.empty()) {
      r = 0;
      c = 0;
    } else {
      r = _text_block.size() - 1;
      c = _text_block[r]._string.size();
    }
    return true;

  } else if (n == 0) {
    // Another special case for the beginning.
    r = 0;
    c = 0;
    return true;
  }

  r = 0;
  while (r + 1 < (int)_text_block.size() &&
         _text_block[r + 1]._row_start < n) {
    r += 1;
  }

  const TextRow &row = _text_block[r];
  bool is_real_char = true;

  nassertr(n > 0, false);
  if (row._got_soft_hyphens) {
    // If there are any soft hyphen or soft break keys in the source
    // text, we have to scan past them to get c precisely.
    c = 0;
    int i = row._row_start;
    while (i < n - 1) {
      if (_text_string[i]._character != text_soft_hyphen_key && 
          _text_string[i]._character != text_soft_break_key) {
        ++c;
      }
      ++i;
    }
    if (_text_string[n - 1]._character != text_soft_hyphen_key && 
        _text_string[n - 1]._character != text_soft_break_key) {
      ++c;
      if (_text_string[n - 1]._character == '\n') {
        is_real_char = false;
      }
    } else {
      is_real_char = false;
    }

  } else {
    // If there are no soft characters, then the string maps
    // one-to-one.
    c = min(n - row._row_start, (int)row._string.size());
    if (_text_string[n - 1]._character == '\n') {
      is_real_char = false;
    }
  }

  return is_real_char;
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::calc_index
//       Access: Published
//  Description: Computes the character index of the character at the
//               rth row and cth column position.  This is the inverse
//               of calc_r_c().
//
//               It is legal for c to exceed the index number of the
//               last column by 1, and it is legal for r to exceed the
//               index number of the last row by 1, if c is 0.
////////////////////////////////////////////////////////////////////
int TextAssembler::
calc_index(int r, int c) const {
  nassertr(r >= 0 && r <= (int)_text_block.size(), 0);
  if (r == (int)_text_block.size()) {
    nassertr(c == 0, 0);
    return _text_string.size();

  } else {
    nassertr(c >= 0 && c <= (int)_text_block[r]._string.size(), 0);
    const TextRow &row = _text_block[r];

    if (row._got_soft_hyphens) {
      // If there are any soft hyphen or soft break keys in the source
      // text, we have to scan past them to get n precisely.
      int n = row._row_start;
      while (c > 0) {
        if (_text_string[n]._character != text_soft_hyphen_key && 
            _text_string[n]._character != text_soft_break_key) {
          --c;
        }
        ++n;
      }
      return n;

    } else {
      // If there are no soft characters, then the string maps
      // one-to-one.
      return row._row_start + c;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::get_xpos
//       Access: Published
//  Description: Returns the x position of the origin of the character
//               or graphic object at the indicated position in the
//               indicated row.
//
//               It is legal for c to exceed the index number of the
//               last column by 1, and it is legal for r to exceed the
//               index number of the last row by 1, if c is 0.
////////////////////////////////////////////////////////////////////
PN_stdfloat TextAssembler::
get_xpos(int r, int c) const {
  nassertr(r >= 0 && r <= (int)_text_block.size(), 0.0f);
  if (r == (int)_text_block.size()) {
    nassertr(c == 0, 0.0f);
    return 0.0f;

  } else {
    nassertr(c >= 0 && c <= (int)_text_block[r]._string.size(), 0.0f);
    const TextRow &row = _text_block[r];
    PN_stdfloat xpos = row._xpos;
    for (int i = 0; i < c; ++i) {
      xpos += calc_width(row._string[i]);
    }
    return xpos;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::assemble_text
//       Access: Published
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
  assemble_paragraph(placed_glyphs);

  // Now that we have a bunch of GlyphPlacements, pull out the Geoms
  // and put them under a common node.
  PT(PandaNode) parent_node = new PandaNode("common");

  PT(PandaNode) shadow_node = new PandaNode("shadow");
  PT(GeomNode) shadow_geom_node = new GeomNode("shadow_geom");
  shadow_node->add_child(shadow_geom_node);

  PT(PandaNode) text_node = new PandaNode("text");
  PT(GeomNode) text_geom_node = new GeomNode("text_geom");
  text_node->add_child(text_geom_node);

  const TextProperties *properties = NULL;
  CPT(RenderState) text_state;
  CPT(RenderState) shadow_state;
  LMatrix4 shadow_xform;

  bool any_shadow = false;

  GeomCollectorMap geom_collector_map;
  GeomCollectorMap geom_shadow_collector_map;

  PlacedGlyphs::const_iterator pgi;
  for (pgi = placed_glyphs.begin(); pgi != placed_glyphs.end(); ++pgi) {
    const GlyphPlacement *placement = (*pgi);

    if (placement->_properties != properties) {
      // Get a new set of properties for future glyphs.
      properties = placement->_properties;
      text_state = RenderState::make_empty();
      shadow_state = RenderState::make_empty();
      shadow_xform = LMatrix4::ident_mat();

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
        shadow_state = shadow_state->add_attrib(ColorAttrib::make_flat(properties->get_shadow_color()));
        if (properties->get_shadow_color()[3] != 1.0) {
          shadow_state = shadow_state->add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha));
        }

        if (properties->has_bin()) {
          shadow_state = shadow_state->add_attrib(CullBinAttrib::make(properties->get_bin(), properties->get_draw_order() + 1));
        }

        LVector2 offset = properties->get_shadow();
        shadow_xform = LMatrix4::translate_mat(offset[0], 0.0f, -offset[1]);
      }
    }

    // We have to place the shadow first, because it copies as it
    // goes, while the place-text function just stomps on the
    // vertices.
    if (properties->has_shadow()) {
      if (_dynamic_merge) {
        placement->assign_append_to(geom_shadow_collector_map, shadow_state, shadow_xform);
      } else {
        placement->assign_copy_to(shadow_geom_node, shadow_state, shadow_xform);
      }

      // Don't shadow the graphics.  That can result in duplication of
      // button objects, plus it looks weird.  If you want a shadowed
      // graphic, you can shadow it yourself before you add it.
      //placement->copy_graphic_to(shadow_node, shadow_state, shadow_xform);
      any_shadow = true;
    }

    if (_dynamic_merge) {
      placement->assign_append_to(geom_collector_map, text_state, LMatrix4::ident_mat());
    } else {
      placement->assign_to(text_geom_node, text_state);
    }
    placement->copy_graphic_to(text_node, text_state, LMatrix4::ident_mat());
    delete placement;
  }  
  placed_glyphs.clear();

  if (any_shadow) {
    // The shadow_geom_node must appear first to guarantee the correct
    // rendering order.
    parent_node->add_child(shadow_node);
  }

  GeomCollectorMap::iterator gc;
  for (gc = geom_collector_map.begin(); gc != geom_collector_map.end(); ++gc) {
    (*gc).second.append_geom(text_geom_node, (*gc).first._state);
  }

  if (any_shadow) {
    for (gc = geom_shadow_collector_map.begin(); 
         gc != geom_shadow_collector_map.end();
         ++gc) {
      (*gc).second.append_geom(shadow_geom_node, (*gc).first._state);
    }
  }
  
  parent_node->add_child(text_node);

  return parent_node;
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::calc_width
//       Access: Published, Static
//  Description: Returns the width of a single character, according to
//               its associated font.  This also correctly calculates
//               the width of cheesy ligatures and accented
//               characters, which may not exist in the font as such.
////////////////////////////////////////////////////////////////////
PN_stdfloat TextAssembler::
calc_width(wchar_t character, const TextProperties &properties) {
  if (character == ' ') {
    // A space is a special case.
    TextFont *font = properties.get_font();
    nassertr(font != (TextFont *)NULL, 0.0f);
    return font->get_space_advance() * properties.get_glyph_scale() * properties.get_text_scale();
  }

  bool got_glyph;
  const TextGlyph *first_glyph = NULL;
  const TextGlyph *second_glyph = NULL;
  UnicodeLatinMap::AccentType accent_type;
  int additional_flags;
  PN_stdfloat glyph_scale;
  PN_stdfloat advance_scale;
  get_character_glyphs(character, &properties, 
                       got_glyph, first_glyph, second_glyph, accent_type,
                       additional_flags, glyph_scale, advance_scale);

  PN_stdfloat advance = 0.0f;
  
  if (first_glyph != (TextGlyph *)NULL) {
    advance = first_glyph->get_advance() * advance_scale;
  }
  if (second_glyph != (TextGlyph *)NULL) {
    advance += second_glyph->get_advance();
  }

  glyph_scale *= properties.get_glyph_scale() * properties.get_text_scale();

  return advance * glyph_scale;
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::calc_width
//       Access: Published, Static
//  Description: Returns the width of a single TextGraphic image.
////////////////////////////////////////////////////////////////////
PN_stdfloat TextAssembler::
calc_width(const TextGraphic *graphic, const TextProperties &properties) {
  LVecBase4 frame = graphic->get_frame();
  return (frame[1] - frame[0]) * properties.get_glyph_scale() * properties.get_text_scale();
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::has_exact_character
//       Access: Published, Static
//  Description: Returns true if the named character exists in the
//               font exactly as named, false otherwise.  Note that
//               because Panda can assemble glyphs together
//               automatically using cheesy accent marks, this is not
//               a reliable indicator of whether a suitable glyph can
//               be rendered for the character.  For that, use
//               has_character() instead.
//
//               This returns true for whitespace and Unicode
//               whitespace characters (if they exist in the font),
//               but returns false for characters that would render
//               with the "invalid glyph".  It also returns false for
//               characters that would be synthesized within Panda,
//               but see has_character().
////////////////////////////////////////////////////////////////////
bool TextAssembler::
has_exact_character(wchar_t character, const TextProperties &properties) {
  if (character == ' ' || character == '\n') {
    // A space is a special case.  Every font implicitly has a space.
    // We also treat newlines specially.
    return true;
  }

  TextFont *font = properties.get_font();
  nassertr(font != (TextFont *)NULL, false);

  const TextGlyph *glyph = NULL;
  return font->get_glyph(character, glyph);
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::has_character
//       Access: Published, Static
//  Description: Returns true if the named character exists in the
//               font or can be synthesized by Panda, false otherwise.
//               (Panda can synthesize some accented characters by
//               combining similar-looking glyphs from the font.)
//
//               This returns true for whitespace and Unicode
//               whitespace characters (if they exist in the font),
//               but returns false for characters that would render
//               with the "invalid glyph".
////////////////////////////////////////////////////////////////////
bool TextAssembler::
has_character(wchar_t character, const TextProperties &properties) {
  if (character == ' ' || character == '\n') {
    // A space is a special case.  Every font implicitly has a space.
    // We also treat newlines specially.
    return true;
  }

  bool got_glyph;
  const TextGlyph *first_glyph = NULL;
  const TextGlyph *second_glyph = NULL;
  UnicodeLatinMap::AccentType accent_type;
  int additional_flags;
  PN_stdfloat glyph_scale;
  PN_stdfloat advance_scale;
  get_character_glyphs(character, &properties, 
                       got_glyph, first_glyph, second_glyph, accent_type,
                       additional_flags, glyph_scale, advance_scale);
  return got_glyph;
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::is_whitespace
//       Access: Published, Static
//  Description: Returns true if the indicated character represents
//               whitespace in the font, or false if anything visible
//               will be rendered for it.
//
//               This returns true for whitespace and Unicode
//               whitespace characters (if they exist in the font),
//               and returns false for any other characters, including
//               characters that do not exist in the font (these would
//               be rendered with the "invalid glyph", which is
//               visible).
//
//               Note that this function can be reliably used to
//               identify Unicode whitespace characters only if the
//               font has all of the whitespace characters defined.
//               It will return false for any character not in the
//               font, even if it is an official Unicode whitespace
//               character.
////////////////////////////////////////////////////////////////////
bool TextAssembler::
is_whitespace(wchar_t character, const TextProperties &properties) {
  if (character == ' ' || character == '\n') {
    // A space or a newline is a special case.
    return true;
  }


  TextFont *font = properties.get_font();
  nassertr(font != (TextFont *)NULL, false);

  const TextGlyph *glyph = NULL;
  if (!font->get_glyph(character, glyph)) {
    return false;
  }

  return glyph->is_whitespace();
}

#ifndef CPPPARSER  // interrogate has a bit of trouble with wstring.
////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::scan_wtext
//       Access: Private
//  Description: Scans through the text string, decoding embedded
//               references to TextProperties.  The decoded string is
//               copied character-by-character into _text_string.
////////////////////////////////////////////////////////////////////
void TextAssembler::
scan_wtext(TextAssembler::TextString &output_string,
           wstring::const_iterator &si, 
           const wstring::const_iterator &send,
           TextAssembler::ComputedProperties *current_cprops) {
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
      
      // Define the new properties by extending the current properties.
      PT(ComputedProperties) new_cprops = 
        new ComputedProperties(current_cprops, wname, _encoder);
      
      // And recursively scan with the nested properties.
      scan_wtext(output_string, si, send, new_cprops);

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

    } else if ((*si) == text_embed_graphic_key) {
      // This indicates an embedded graphic.  Pull off the name of the
      // TextGraphic structure, which is everything until the next
      // text_embed_graphic_key.

      wstring graphic_wname;
      ++si;
      while (si != send && (*si) != text_embed_graphic_key) {
        graphic_wname += (*si);
        ++si;
      }

      if (si == send) {
        // We didn't close the text_embed_graphic_key.  That's an
        // error.
        text_cat.warning()
          << "Unclosed embed_graphic in text.\n";
        return;
      }

      ++si;

      // Now we have to encode the wstring into a string, for lookup
      // in the TextPropertiesManager.
      string graphic_name = _encoder->encode_wtext(graphic_wname);
      
      TextPropertiesManager *manager = 
        TextPropertiesManager::get_global_ptr();
      
      // Get the graphic image.
      const TextGraphic *named_graphic = manager->get_graphic_ptr(graphic_name);
      if (named_graphic != (TextGraphic *)NULL) {
        output_string.push_back(TextCharacter(named_graphic, graphic_wname, current_cprops));

      } else {
        text_cat.warning()
          << "Unknown TextGraphic: " << graphic_name << "\n";
      }

    } else {
      // A normal character.  Apply it.
      output_string.push_back(TextCharacter(*si, current_cprops));
      ++si;
    }
  }
}
#endif  // CPPPARSER

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::wordwrap_text
//       Access: Private
//  Description: Inserts newlines into the _text_string at the
//               appropriate places in order to make each line be the
//               longest possible line that is not longer than
//               wordwrap_width (and does not break any words, if
//               possible).  Stores the result in _text_block.
//
//               If _max_rows is greater than zero, no more than
//               _max_rows will be accepted.  Text beyond that will be
//               truncated.
//
//               The return value is true if all the text is accepted,
//               or false if some was truncated.
////////////////////////////////////////////////////////////////////
bool TextAssembler::
wordwrap_text() {
  _text_block.clear();

  if (_text_string.empty()) {
    // A special case: empty text means no rows.
    return true;
  }

  size_t p = 0;

  _text_block.push_back(TextRow(p));

  // Preserve any initial whitespace and newlines.
  PN_stdfloat initial_width = 0.0f;
  while (p < _text_string.size() && isspacew(_text_string[p]._character)) {
    if (_text_string[p]._character == '\n') {
      initial_width = 0.0f;
      if (_max_rows > 0 && (int)_text_block.size() >= _max_rows) {
        // Truncate.
        return false;
      }
      _text_block.back()._eol_cprops = _text_string[p]._cprops;
      _text_block.push_back(TextRow(p + 1));
    } else {
      initial_width += calc_width(_text_string[p]);
      _text_block.back()._string.push_back(_text_string[p]);
    }
    p++;
  }
  bool needs_newline = false;

  while (p < _text_string.size()) {
    nassertr(!isspacew(_text_string[p]._character), false);

    // Scan the next n characters, until the end of the string or an
    // embedded newline character, or we exceed wordwrap_width.

    size_t q = p;
    bool any_spaces = false;
    size_t last_space = 0;
    PN_stdfloat last_space_width = 0.0f;

    bool any_hyphens = false;
    size_t last_hyphen = 0;
    bool output_hyphen = false;

    bool overflow = false;
    PN_stdfloat wordwrap_width = -1.0f;

    bool last_was_space = false;
    PN_stdfloat width = initial_width;
    while (q < _text_string.size() && _text_string[q]._character != '\n') {
      if (_text_string[q]._cprops->_properties.has_wordwrap()) {
        wordwrap_width = _text_string[q]._cprops->_properties.get_wordwrap();
      } else {
        wordwrap_width = -1.0f;
      }

      if (isspacew(_text_string[q]._character) || 
          _text_string[q]._character == text_soft_break_key) {
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
      if (_text_string[q]._character == text_soft_hyphen_key) {
        if (wordwrap_width > 0.0f) {
          // We only consider this as a possible hyphenation point if
          // (a) it is not the very first character, and (b) there is
          // enough room for a hyphen character to be printed following
          // it.
          if (q != p && width + calc_hyphen_width(_text_string[q]) <= wordwrap_width) {
            any_hyphens = true;
            last_hyphen = q;
          }
        }
      } else {
        // Some normal, printable character.
        width += calc_width(_text_string[q]);
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
               get_text_never_break_before().find(_text_string[q - i]._character) != wstring::npos) {
          i++;
        }
        if ((int)i < text_max_never_break) {
          q -= i;
        }
      }
    }

    // Skip additional whitespace between the lines.
    size_t next_start = q;
    while (next_start < _text_string.size() && 
           isbreakpoint(_text_string[next_start]._character)) {
      next_start++;
    }

    // Trim off any more blanks on the end.
    while (q > p && isspacew(_text_string[q - 1]._character)) {
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
        while (next_start < _text_string.size() && 
               isbreakpoint(_text_string[next_start]._character)) {
          next_start++;
        }
      }
    }
    
    if (needs_newline) {
      if (_max_rows > 0 && (int)_text_block.size() >= _max_rows) {
        // Truncate.
        return false;
      }
      _text_block.push_back(TextRow(p));
    }
    if (get_multiline_mode()){
        needs_newline = true;
    }

    if (_text_string[next_start - 1]._cprops->_properties.get_preserve_trailing_whitespace()) {
      q = next_start;
    }

    for (size_t pi = p; pi < q; pi++) {
      if (_text_string[pi]._character != text_soft_hyphen_key && 
          _text_string[pi]._character != text_soft_break_key) {
        _text_block.back()._string.push_back(_text_string[pi]);
      } else {
        _text_block.back()._got_soft_hyphens = true;
      }
    }
    if (output_hyphen) {
      wstring text_soft_hyphen_output = get_text_soft_hyphen_output();
      wstring::const_iterator wi;
      for (wi = text_soft_hyphen_output.begin();
           wi != text_soft_hyphen_output.end();
           ++wi) {
        _text_block.back()._string.push_back(TextCharacter(*wi, _text_string[last_hyphen]._cprops));
      }
    }

    // Now prepare to wrap the next line.

    if (next_start < _text_string.size() && _text_string[next_start]._character == '\n') {
      // Preserve a single embedded newline.
      if (_max_rows > 0 && (int)_text_block.size() >= _max_rows) {
        // Truncate.
        return false;
      }
      _text_block.back()._eol_cprops = _text_string[next_start]._cprops;
      next_start++;
      _text_block.push_back(TextRow(next_start));
      needs_newline = false;
    }
    p = next_start;

    // Preserve any initial whitespace and newlines.
    initial_width = 0.0f;
    while (p < _text_string.size() && isspacew(_text_string[p]._character)) {
      if (_text_string[p]._character == '\n') {
        initial_width = 0.0f;
        if (_max_rows > 0 && (int)_text_block.size() >= _max_rows) {
          // Truncate.
          return false;
        }
        _text_block.back()._eol_cprops = _text_string[p]._cprops;
        _text_block.push_back(TextRow(p + 1));
      } else {
        initial_width += calc_width(_text_string[p]);
        _text_block.back()._string.push_back(_text_string[p]);
      }
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
PN_stdfloat TextAssembler::
calc_hyphen_width(const TextCharacter &tch) {
  TextFont *font = tch._cprops->_properties.get_font();
  nassertr(font != (TextFont *)NULL, 0.0f);

  PN_stdfloat hyphen_width = 0.0f;
  wstring text_soft_hyphen_output = get_text_soft_hyphen_output();
  wstring::const_iterator wi;
  for (wi = text_soft_hyphen_output.begin();
       wi != text_soft_hyphen_output.end();
       ++wi) {
    hyphen_width += calc_width(*wi, tch._cprops->_properties);
  }
  
  return hyphen_width;
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::assemble_paragraph
//       Access: Private
//  Description: Fills up placed_glyphs, _ul, _lr with
//               the contents of _text_block.  Also updates _xpos and
//               _ypos within the _text_block structure.
////////////////////////////////////////////////////////////////////
void TextAssembler::
assemble_paragraph(TextAssembler::PlacedGlyphs &placed_glyphs) {
  _ul.set(0.0f, 0.0f);
  _lr.set(0.0f, 0.0f);
  int num_rows = 0;

  PN_stdfloat ypos = 0.0f;
  _next_row_ypos = 0.0f;
  TextBlock::iterator bi;
  for (bi = _text_block.begin(); bi != _text_block.end(); ++bi) {
    TextRow &row = (*bi);

    // First, assemble all the glyphs of this row.
    PlacedGlyphs row_placed_glyphs;
    PN_stdfloat row_width, line_height, wordwrap;
    TextProperties::Alignment align;
    assemble_row(row, row_placed_glyphs,
                 row_width, line_height, align, wordwrap);
    // Now move the row to its appropriate position.  This might
    // involve a horizontal as well as a vertical translation.
    LMatrix4 mat = LMatrix4::ident_mat();

    if (num_rows == 0) {
      // If this is the first row, account for its space.
      _ul[1] = 0.8f * line_height;

    } else {
      // If it is not the first row, shift the text downward by
      // line_height from the previous row.
      ypos -= line_height;
    }
    _lr[1] = ypos - 0.2 * line_height;

    // Apply the requested horizontal alignment to the row.
    //[fabius] added a different concept of text alignment based upon a boxed region where his width is defined by the wordwrap size with the upper left corner starting from 0,0,0
    // if the wordwrap size is unspecified the alignment could eventually result wrong.
    PN_stdfloat xpos;
    switch (align) {
    case TextProperties::A_left:
      xpos = 0.0f;
      _lr[0] = max(_lr[0], row_width);
      break;

    case TextProperties::A_right:
      xpos = -row_width;
      _ul[0] = min(_ul[0], xpos);
      break;

    case TextProperties::A_center:
      xpos = -0.5f * row_width;
      _ul[0] = min(_ul[0], xpos);
      _lr[0] = max(_lr[0], -xpos);
      break;

    case TextProperties::A_boxed_left:
      xpos = 0.0f;
      _lr[0] = max(_lr[0], max(row_width, wordwrap));
      break;

    case TextProperties::A_boxed_right:
      xpos = wordwrap - row_width;
      _ul[0] = min(_ul[0], xpos);
      break;

    case TextProperties::A_boxed_center:
      xpos = -0.5f * row_width;
      if (wordwrap > row_width) xpos += (wordwrap * 0.5f);
      _ul[0] = min(_ul[0], max(xpos,(wordwrap * 0.5f)));
      _lr[0] = max(_lr[0], min(-xpos,-(wordwrap * 0.5f)));
      break;
    }

    mat.set_row(3, LVector3(xpos, 0.0f, ypos));
    row._xpos = xpos;
    row._ypos = ypos;

    // Now store the geoms we assembled.
    PlacedGlyphs::iterator pi;
    for (pi = row_placed_glyphs.begin(); pi != row_placed_glyphs.end(); ++pi) {
      (*pi)->_xform *= mat;
      placed_glyphs.push_back(*pi);
    }

    // Advance to the next line.
    num_rows++;
    _next_row_ypos = ypos - line_height;
  }

  // num_rows may be smaller than _text_block.size(), if there are
  // trailing newlines on the string.
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
assemble_row(TextAssembler::TextRow &row,
             TextAssembler::PlacedGlyphs &row_placed_glyphs,
             PN_stdfloat &row_width, PN_stdfloat &line_height, 
             TextProperties::Alignment &align, PN_stdfloat &wordwrap) {
  Thread *current_thread = Thread::get_current_thread();

  line_height = 0.0f;
  PN_stdfloat xpos = 0.0f;
  align = TextProperties::A_left;

  bool underscore = false;
  PN_stdfloat underscore_start = 0.0f;
  const TextProperties *underscore_properties = NULL;

  TextString::const_iterator si;
  for (si = row._string.begin(); si != row._string.end(); ++si) {
    const TextCharacter &tch = (*si);
    wchar_t character = tch._character;
    const TextGraphic *graphic = tch._graphic;
    const TextProperties *properties = &(tch._cprops->_properties);

    if (properties->get_underscore() != underscore ||
        (underscore && (properties->get_text_color() != underscore_properties->get_text_color() ||
                        properties->get_underscore_height() != underscore_properties->get_underscore_height()))) {
      // Change the underscore status.
      if (underscore && underscore_start != xpos) {
        draw_underscore(row_placed_glyphs, underscore_start, xpos,
                        underscore_properties);
      }
      underscore = properties->get_underscore();
      underscore_start = xpos;
      underscore_properties = properties;
    }

    TextFont *font = properties->get_font();
    nassertv(font != (TextFont *)NULL);

    // We get the row's alignment property from the first character of the row
    if ((align == TextProperties::A_left) &&
        (properties->get_align() != TextProperties::A_left)) {
      align = properties->get_align();
    }

    // And the height of the row is the maximum of all the fonts used
    // within the row.
    if (graphic != (TextGraphic *)NULL) {
      LVecBase4 frame = graphic->get_frame();
      line_height = max(line_height, frame[3] - frame[2]);
    } else {
      //[fabius] this is not the right place to calc line height (see below)
      //       line_height = max(line_height, font->get_line_height());
    }

    if (character == ' ') {
      // A space is a special case.
      xpos += properties->get_glyph_scale() * properties->get_text_scale() * font->get_space_advance();

    } else if (character == '\t') {
      // So is a tab character.
      PN_stdfloat tab_width = properties->get_tab_width();
      xpos = (floor(xpos / tab_width) + 1.0f) * tab_width;

    } else if (character == text_soft_hyphen_key) {
      // And so is the 'soft-hyphen' key character.

    } else if (graphic != (TextGraphic *)NULL) {
      // A special embedded graphic.
      GlyphPlacement *placement = new GlyphPlacement;
      row_placed_glyphs.push_back(placement);

      PT(PandaNode) model = graphic->get_model().node();
      if (graphic->get_instance_flag()) {
        // Instance the model in.  Create a ModelNode so it doesn't
        // get flattened.
        PT(ModelNode) model_node = new ModelNode("");
        model_node->set_preserve_transform(ModelNode::PT_no_touch);
        model_node->add_child(model);
        placement->_graphic_model = model_node.p();
      } else {
        // Copy the model in.  This the preferred way; it's a little
        // cheaper to render than instancing (because flattening is
        // more effective).
        placement->_graphic_model = model->copy_subgraph();
      }

      LVecBase4 frame = graphic->get_frame();
      PN_stdfloat glyph_scale = properties->get_glyph_scale() * properties->get_text_scale();

      PN_stdfloat advance = (frame[1] - frame[0]);

      // Now compute the matrix that will transform the glyph (or
      // glyphs) into position.
      LMatrix4 glyph_xform = LMatrix4::scale_mat(glyph_scale);

      glyph_xform(3, 0) += (xpos - frame[0]);
      glyph_xform(3, 2) += (properties->get_glyph_shift() - frame[2]);

      if (properties->has_slant()) {
        LMatrix4 shear(1.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.0f,
                        properties->get_slant(), 0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 1.0f);
        glyph_xform = shear * glyph_xform;
      }
      
      placement->_xform = glyph_xform;
      placement->_properties = properties;

      xpos += advance * glyph_scale;

    } else {
      // A printable character.
      bool got_glyph;
      const TextGlyph *first_glyph;
      const TextGlyph *second_glyph;
      UnicodeLatinMap::AccentType accent_type;
      int additional_flags;
      PN_stdfloat glyph_scale;
      PN_stdfloat advance_scale;
      get_character_glyphs(character, properties, 
                           got_glyph, first_glyph, second_glyph, accent_type,
                           additional_flags, glyph_scale, advance_scale);

      if (!got_glyph) {
        char buffer[512];
        sprintf(buffer, "U+%04x", character);
        text_cat.warning()
          << "No definition in " << font->get_name() 
          << " for character " << buffer;
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

      PN_stdfloat advance = 0.0f;

      if (first_glyph != (TextGlyph *)NULL) {
        PT(Geom) first_char_geom = first_glyph->get_geom(_usage_hint);
        if (first_char_geom != (Geom *)NULL) {
          placement->add_piece(first_char_geom, first_glyph->get_state());
        }
        advance = first_glyph->get_advance() * advance_scale;
      }
      if (second_glyph != (TextGlyph *)NULL) {
        PT(Geom) second_char_geom = second_glyph->get_geom(_usage_hint);
        if (second_char_geom != (Geom *)NULL) {
          second_char_geom->transform_vertices(LMatrix4::translate_mat(advance, 0.0f, 0.0f));
          placement->add_piece(second_char_geom, second_glyph->get_state());
        }
        advance += second_glyph->get_advance();
      }

      glyph_scale *= properties->get_glyph_scale() * properties->get_text_scale();
      //[fabius] a good place to take wordwrap size
      if (properties->get_wordwrap() > 0.0f) {
        wordwrap = properties->get_wordwrap();
      }
      // Now compute the matrix that will transform the glyph (or
      // glyphs) into position.
      LMatrix4 glyph_xform = LMatrix4::scale_mat(glyph_scale);

      if (accent_type != UnicodeLatinMap::AT_none || additional_flags != 0) {
        // If we have some special handling to perform, do so now.
        // This will probably require the bounding volume of the
        // glyph, so go get that.
        LPoint3 min_vert, max_vert;
        bool found_any = false;
        placement->calc_tight_bounds(min_vert, max_vert, found_any,
                                     current_thread);

        if (found_any) {
          LPoint3 centroid = (min_vert + max_vert) / 2.0f;
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
            LMatrix4 rotate =
              LMatrix4::translate_mat(-centroid) *
              LMatrix4::rotate_mat_normaxis(180.0f, LVecBase3(0.0f, -1.0f, 0.0f)) *
              LMatrix4::translate_mat(centroid);
            glyph_xform *= rotate;
          }
        }
      }

      glyph_xform(3, 0) += xpos;
      glyph_xform(3, 2) += properties->get_glyph_shift();

      if (properties->has_slant()) {
        LMatrix4 shear(1.0f, 0.0f, 0.0f, 0.0f,
                        0.0f, 1.0f, 0.0f, 0.0f,
                        properties->get_slant(), 0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 0.0f, 1.0f);
        glyph_xform = shear * glyph_xform;
      }
      
      placement->_xform = glyph_xform;
      placement->_properties = properties;

      xpos += advance * glyph_scale;
      line_height = max(line_height, font->get_line_height() * glyph_scale);
    }
  }

  if (underscore && underscore_start != xpos) {
    draw_underscore(row_placed_glyphs, underscore_start, xpos,
                    underscore_properties);
  }

  row_width = xpos;

  if (row._eol_cprops != (ComputedProperties *)NULL) {
    // If there's an _eol_cprops, it represents the cprops of the
    // newline character that ended the line, which should also
    // contribute towards the line_height.

    const TextProperties *properties = &(row._eol_cprops->_properties);
    TextFont *font = properties->get_font();
    nassertv(font != (TextFont *)NULL);

    if (line_height == 0.0f) {
      PN_stdfloat glyph_scale = properties->get_glyph_scale() * properties->get_text_scale();
      line_height = max(line_height, font->get_line_height() * glyph_scale);
    }
  }
}
  
////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::draw_underscore
//       Access: Private, Static
//  Description: Creates the geometry to render the underscore line
//               for the indicated range of glyphs in this row.
////////////////////////////////////////////////////////////////////
void TextAssembler::
draw_underscore(TextAssembler::PlacedGlyphs &row_placed_glyphs,
                PN_stdfloat underscore_start, PN_stdfloat underscore_end, 
                const TextProperties *underscore_properties) {
  CPT(GeomVertexFormat) format = GeomVertexFormat::get_v3cp();
  PT(GeomVertexData) vdata = 
    new GeomVertexData("text", format, Geom::UH_static);
  vdata->reserve_num_rows(2);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter color(vdata, InternalName::get_color());

  PN_stdfloat y = underscore_properties->get_underscore_height();
  vertex.add_data3(underscore_start, 0.0f, y);
  color.add_data4(underscore_properties->get_text_color());
  vertex.add_data3(underscore_end, 0.0f, y);
  color.add_data4(underscore_properties->get_text_color());

  PT(GeomLines) lines = new GeomLines(Geom::UH_static);
  lines->add_vertices(0, 1);
  lines->close_primitive();

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(lines);

  GlyphPlacement *placement = new GlyphPlacement;
  placement->add_piece(geom, RenderState::make_empty());
  placement->_xform = LMatrix4::ident_mat();
  placement->_properties = underscore_properties;

  row_placed_glyphs.push_back(placement);
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
                     PN_stdfloat &glyph_scale, PN_stdfloat &advance_scale) {
  TextFont *font = properties->get_font();
  nassertv_always(font != (TextFont *)NULL);

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
//       Access: Private
//  Description: This is a cheesy attempt to tack on an accent to an
//               ASCII letter for which we don't have the appropriate
//               already-accented glyph in the font.
////////////////////////////////////////////////////////////////////
void TextAssembler::
tack_on_accent(UnicodeLatinMap::AccentType accent_type,
               const LPoint3 &min_vert, const LPoint3 &max_vert,
               const LPoint3 &centroid,
               const TextProperties *properties, 
               TextAssembler::GlyphPlacement *placement) const {
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
//       Access: Private
//  Description: Generates a cheesy accent mark above (or below, etc.)
//               the character.  Returns true if successful, or false
//               if the named accent character doesn't exist in the
//               font.
////////////////////////////////////////////////////////////////////
bool TextAssembler::
tack_on_accent(char accent_mark, TextAssembler::CheesyPosition position,
               TextAssembler::CheesyTransform transform,
               const LPoint3 &min_vert, const LPoint3 &max_vert,
               const LPoint3 &centroid,
               const TextProperties *properties,
               TextAssembler::GlyphPlacement *placement) const {
  TextFont *font = properties->get_font();
  nassertr(font != (TextFont *)NULL, false);

  Thread *current_thread = Thread::get_current_thread();

  const TextGlyph *accent_glyph;
  if (font->get_glyph(accent_mark, accent_glyph) ||
      font->get_glyph(toupper(accent_mark), accent_glyph)) {
    PT(Geom) accent_geom = accent_glyph->get_geom(_usage_hint);
    if (accent_geom != (Geom *)NULL) {
      LPoint3 min_accent, max_accent;
      bool found_any = false;
      accent_geom->calc_tight_bounds(min_accent, max_accent, found_any,
                                     current_thread);
      if (found_any) {
        PN_stdfloat t, u;
        LMatrix4 accent_mat;

        // This gets set to true if the glyph gets mirrored and needs
        // to have backface culling disabled.
        bool mirrored = false;

        switch (transform) {
        case CT_none:
          accent_mat = LMatrix4::ident_mat();
          break;

        case CT_mirror_x:
          accent_mat = LMatrix4::scale_mat(-1.0f, 1.0f, 1.0f);
          t = min_accent[0];
          min_accent[0] = -max_accent[0];
          max_accent[0] = -t;
          mirrored = true;
          break;

        case CT_mirror_y:
          accent_mat = LMatrix4::scale_mat(1.0f, 1.0f, -1.0f);
          t = min_accent[2];
          min_accent[2] = -max_accent[2];
          max_accent[2] = -t;
          mirrored = true;
          break;

        case CT_rotate_90:
          accent_mat.set_rotate_mat_normaxis(90.0f, LVecBase3(0.0f, -1.0f, 0.0f));
          // rotate min, max
          t = min_accent[0];
          u = max_accent[0];
          max_accent[0] = -min_accent[2];
          min_accent[0] = -max_accent[2];
          max_accent[2] = u;
          min_accent[2] = t;
          break;

        case CT_rotate_180:
          accent_mat = LMatrix4::scale_mat(-1.0f, -1.0f, 1.0f);
          
          t = min_accent[0];
          min_accent[0] = -max_accent[0];
          max_accent[0] = -t;
          t = min_accent[2];
          min_accent[2] = -max_accent[2];
          max_accent[2] = -t;
          break;

        case CT_rotate_270:
          accent_mat.set_rotate_mat_normaxis(270.0f, LVecBase3(0.0f, -1.0f, 0.0f));
          // rotate min, max
          t = min_accent[0];
          u = max_accent[0];
          min_accent[0] = min_accent[2];
          max_accent[0] = max_accent[2];
          min_accent[2] = -u;
          max_accent[2] = -t;
          break;

        case CT_squash:
          accent_mat = LMatrix4::scale_mat(squash_accent_scale_x, 1.0f, squash_accent_scale_y);
          min_accent[0] *= squash_accent_scale_x;
          max_accent[0] *= squash_accent_scale_x;
          min_accent[2] *= squash_accent_scale_y;
          max_accent[2] *= squash_accent_scale_y;
          break;

        case CT_squash_mirror_y:
          accent_mat = LMatrix4::scale_mat(squash_accent_scale_x, 1.0f, -squash_accent_scale_y);
          min_accent[0] *= squash_accent_scale_x;
          max_accent[0] *= squash_accent_scale_x;
          t = min_accent[2];
          min_accent[2] = -max_accent[2] * squash_accent_scale_y;
          max_accent[2] = -t * squash_accent_scale_y;
          mirrored = true;
          break;

        case CT_squash_mirror_diag:
          accent_mat =
            LMatrix4::rotate_mat_normaxis(270.0f, LVecBase3(0.0f, -1.0f, 0.0f)) *
            LMatrix4::scale_mat(-squash_accent_scale_x, 1.0f, squash_accent_scale_y);
          
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
          accent_mat = LMatrix4::scale_mat(small_squash_accent_scale_x, 1.0f, small_squash_accent_scale_y);
          min_accent[0] *= small_squash_accent_scale_x;
          max_accent[0] *= small_squash_accent_scale_x;
          min_accent[2] *= small_squash_accent_scale_y;
          max_accent[2] *= small_squash_accent_scale_y;
          break;

        case CT_small_squash_mirror_y:
          accent_mat = LMatrix4::scale_mat(small_squash_accent_scale_x, 1.0f, -small_squash_accent_scale_y);
          min_accent[0] *= small_squash_accent_scale_x;
          max_accent[0] *= small_squash_accent_scale_x;
          t = min_accent[2];
          min_accent[2] = -max_accent[2] * small_squash_accent_scale_y;
          max_accent[2] = -t * small_squash_accent_scale_y;
          mirrored = true;
          break;

        case CT_small_squash_mirror_diag:
          accent_mat =
            LMatrix4::rotate_mat_normaxis(270.0f, LVecBase3(0.0f, -1.0f, 0.0f)) *
            LMatrix4::scale_mat(-small_squash_accent_scale_x, 1.0f, small_squash_accent_scale_y);
          
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
          accent_mat = LMatrix4::scale_mat(small_accent_scale);
          min_accent *= small_accent_scale;
          max_accent *= small_accent_scale;
          break;

        case CT_small_rotate_270:
          accent_mat =
            LMatrix4::rotate_mat_normaxis(270.0f, LVecBase3(0.0f, -1.0f, 0.0f)) *
            LMatrix4::scale_mat(small_accent_scale);

          // rotate min, max
          t = min_accent[0];
          u = max_accent[0];
          min_accent[0] = min_accent[2] * small_accent_scale;
          max_accent[0] = max_accent[2] * small_accent_scale;
          min_accent[2] = -u * small_accent_scale;
          max_accent[2] = -t * small_accent_scale;
          break;

        case CT_tiny:
          accent_mat = LMatrix4::scale_mat(tiny_accent_scale);
          min_accent *= tiny_accent_scale;
          max_accent *= tiny_accent_scale;
          break;

        case CT_tiny_mirror_x:
          accent_mat = LMatrix4::scale_mat(-tiny_accent_scale, 1.0f, tiny_accent_scale);
          
          t = min_accent[0];
          min_accent[0] = -max_accent[0] * tiny_accent_scale;
          max_accent[0] = -t * tiny_accent_scale;
          min_accent[2] *= tiny_accent_scale;
          max_accent[2] *= tiny_accent_scale;
          mirrored = true;
          break;

        case CT_tiny_rotate_270:
          accent_mat =
            LMatrix4::rotate_mat_normaxis(270.0f, LVecBase3(0.0f, -1.0f, 0.0f)) *
            LMatrix4::scale_mat(tiny_accent_scale);

          // rotate min, max
          t = min_accent[0];
          u = max_accent[0];
          min_accent[0] = min_accent[2] * tiny_accent_scale;
          max_accent[0] = max_accent[2] * tiny_accent_scale;
          min_accent[2] = -u * tiny_accent_scale;
          max_accent[2] = -t * tiny_accent_scale;
          break;
        }

        LPoint3 accent_centroid = (min_accent + max_accent) / 2.0f;
        PN_stdfloat accent_height = max_accent[2] - min_accent[2];
        LVector3 trans;
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
//     Function: TextAssembler::ComputedProperties::append_delta
//       Access: Public
//  Description: Appends to wtext the control sequences necessary to
//               change from this ComputedProperties to the indicated
//               ComputedProperties.
////////////////////////////////////////////////////////////////////
void TextAssembler::ComputedProperties::
append_delta(wstring &wtext, TextAssembler::ComputedProperties *other) {
  if (this != other) {
    if (_depth > other->_depth) {
      // Back up a level from this properties.
      nassertv(_based_on != NULL);

      wtext.push_back(text_pop_properties_key);
      _based_on->append_delta(wtext, other);
      
    } else if (other->_depth > _depth) {
      // Back up a level from the other properties.
      nassertv(other->_based_on != NULL);

      append_delta(wtext, other->_based_on);
      wtext.push_back(text_push_properties_key);
      wtext += other->_wname;
      wtext.push_back(text_push_properties_key);
      
    } else if (_depth != 0) {
      // Back up a level from both properties.
      nassertv(_based_on != NULL && other->_based_on != NULL);
      
      wtext.push_back(text_pop_properties_key);
      _based_on->append_delta(wtext, other->_based_on);
      wtext.push_back(text_push_properties_key);
      wtext += other->_wname;
      wtext.push_back(text_push_properties_key);
    }
  }
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
calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                  bool &found_any, Thread *current_thread) const {
  Pieces::const_iterator pi;
  for (pi = _pieces.begin(); pi != _pieces.end(); ++pi) {
    (*pi)._geom->calc_tight_bounds(min_point, max_point, found_any,
                                   current_thread);
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
               const LMatrix4 &extra_xform) const {
  LMatrix4 new_xform = _xform * extra_xform;
  Pieces::const_iterator pi;
  for (pi = _pieces.begin(); pi != _pieces.end(); ++pi) {
    const Geom *geom = (*pi)._geom;
    PT(Geom) new_geom = geom->make_copy();
    new_geom->transform_vertices(new_xform);
    geom_node->add_geom(new_geom, state->compose((*pi)._state));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::GlyphPlacement::assign_append_to
//       Access: Private
//  Description: Puts the pieces of the GlyphPlacement in the
//               indicated GeomNode.  This flavor will append the
//               Geoms with the additional transform applied to the
//               vertices.
////////////////////////////////////////////////////////////////////
void TextAssembler::GlyphPlacement::
assign_append_to(GeomCollectorMap &geom_collector_map, 
                 const RenderState *state,
                 const LMatrix4 &extra_xform) const {
  LMatrix4 new_xform = _xform * extra_xform;
  Pieces::const_iterator pi;

  int p, sp, s, e, i;
  for (pi = _pieces.begin(); pi != _pieces.end(); ++pi) {
    const Geom *geom = (*pi)._geom;
    const GeomVertexData *vdata = geom->get_vertex_data();
    CPT(RenderState) rs = (*pi)._state->compose(state);
    GeomCollectorKey key(rs, vdata->get_format());

    GeomCollectorMap::iterator mi = geom_collector_map.find(key);
    if (mi == geom_collector_map.end()) {
      mi = geom_collector_map.insert(GeomCollectorMap::value_type(key, GeomCollector(vdata->get_format()))).first;
    }
    GeomCollector &geom_collector = (*mi).second;
    geom_collector.count_geom(geom);

    // We use this map to keep track of vertex indices we have already
    // added, so that we don't needlessly duplicate vertices into our
    // output vertex data.
    VertexIndexMap vimap;

    for (p = 0; p < geom->get_num_primitives(); p++) {
      CPT(GeomPrimitive) primitive = geom->get_primitive(p)->decompose();

      // Get a new GeomPrimitive of the corresponding type.
      GeomPrimitive *new_prim = geom_collector.get_primitive(primitive->get_type());

      // Walk through all of the components (e.g. triangles) of the
      // primitive.
      for (sp = 0; sp < primitive->get_num_primitives(); sp++) {
        s = primitive->get_primitive_start(sp);
        e = primitive->get_primitive_end(sp);

        // Walk through all of the vertices in the component.
        for (i = s; i < e; i++) {
          int vi = primitive->get_vertex(i);

          // Attempt to insert number "vi" into the map.
          pair<VertexIndexMap::iterator, bool> added = vimap.insert(VertexIndexMap::value_type(vi, 0));
          int new_vertex;
          if (added.second) {
            // The insert succeeded.  That means this is the first
            // time we have encountered this vertex.
            new_vertex = geom_collector.append_vertex(vdata, vi, new_xform);
            // Update the map with the newly-created target vertex index.
            (*(added.first)).second = new_vertex;

          } else {
            // The insert failed.  This means we have previously
            // encountered this vertex, and we have already entered
            // its target vertex index into the vimap.  Extract that
            // vertex index, so we can reuse it.
            new_vertex = (*(added.first)).second;
          }
          new_prim->add_vertex(new_vertex);
        }
        new_prim->close_primitive();
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::GlyphPlacement::copy_graphic_to
//       Access: Private
//  Description: If the GlyphPlacement includes a special graphic,
//               copies it to the indicated node.
////////////////////////////////////////////////////////////////////
void TextAssembler::GlyphPlacement::
copy_graphic_to(PandaNode *node, const RenderState *state,
                const LMatrix4 &extra_xform) const {
  if (_graphic_model != (PandaNode *)NULL) {
    LMatrix4 new_xform = _xform * extra_xform;

    // We need an intermediate node to hold the transform and state.
    PT(PandaNode) intermediate_node = new PandaNode("");
    node->add_child(intermediate_node);

    intermediate_node->set_transform(TransformState::make_mat(new_xform));
    intermediate_node->set_state(state);
    intermediate_node->add_child(_graphic_model);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::GeomCollector Constructor
//       Access: Public
//  Description: constructs the GeomCollector class 
//               (Geom, GeomTriangles, vertexWriter, texcoordWriter..)
////////////////////////////////////////////////////////////////////
TextAssembler::GeomCollector::
GeomCollector(const GeomVertexFormat *format) :
  _vdata(new GeomVertexData("merged_geom", format, Geom::UH_static)),
  _geom(new GeomTextGlyph(_vdata))
{
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::GeomCollector Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TextAssembler::GeomCollector::
GeomCollector(const TextAssembler::GeomCollector &copy) :
  _vdata(copy._vdata),
  _geom(copy._geom)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::GeomCollector::get_primitive
//       Access: Public
//  Description: Returns a GeomPrimitive of the appropriate type.  If
//               one has not yet been created, returns a newly-created
//               one; if one has previously been created of this type,
//               returns the previously-created one.
////////////////////////////////////////////////////////////////////
GeomPrimitive *TextAssembler::GeomCollector::
get_primitive(TypeHandle prim_type) {
  if (prim_type == GeomTriangles::get_class_type()) {
    if (_triangles == (GeomPrimitive *)NULL) {
      _triangles = new GeomTriangles(Geom::UH_static);
      _geom->add_primitive(_triangles);
    }
    return _triangles;

  } else if (prim_type == GeomLines::get_class_type()) {
    if (_lines == (GeomPrimitive *)NULL) {
      _lines = new GeomLines(Geom::UH_static);
      _geom->add_primitive(_lines);
    }
    return _lines;

  } else if (prim_type == GeomPoints::get_class_type()) {
    if (_points == (GeomPrimitive *)NULL) {
      _points = new GeomPoints(Geom::UH_static);
      _geom->add_primitive(_points);
    }
    return _points;
  }

  nassertr(false, NULL);
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::GeomCollector::append_vertex
//       Access: Public
//  Description: Adds one vertex to the GeomVertexData.
//               Returns the row number of the added vertex.
////////////////////////////////////////////////////////////////////
int TextAssembler::GeomCollector::
append_vertex(const GeomVertexData *orig_vdata, int orig_row,
              const LMatrix4 &xform) {
  int new_row = _vdata->get_num_rows();
  _vdata->copy_row_from(new_row, orig_vdata, orig_row, Thread::get_current_thread());

  GeomVertexRewriter vertex_rewriter(_vdata, InternalName::get_vertex());
  vertex_rewriter.set_row_unsafe(new_row);
  LPoint3 point = vertex_rewriter.get_data3();
  vertex_rewriter.set_data3(point * xform);

  return new_row;
}


////////////////////////////////////////////////////////////////////
//     Function: TextAssembler::GeomCollector::append_geom
//       Access: Public
//  Description: closes the geomTriangles and appends the geom to 
//               the given GeomNode
////////////////////////////////////////////////////////////////////
void TextAssembler::GeomCollector::
append_geom(GeomNode *geom_node, const RenderState *state) {
  if (_geom->get_num_primitives() > 0) {
    geom_node->add_geom(_geom, state);
  }
}

