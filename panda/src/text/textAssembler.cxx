/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textAssembler.cxx
 * @author drose
 * @date 2004-04-06
 */

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
#include "dynamicTextFont.h"

#include <ctype.h>
#include <stdio.h>  // for sprintf

#ifdef HAVE_HARFBUZZ
#include <hb.h>
#endif

using std::max;
using std::min;
using std::move;
using std::wstring;

// This is the factor by which CT_small scales the character down.
static const PN_stdfloat small_accent_scale = 0.6f;

// This is the factor by which CT_tiny scales the character down.
static const PN_stdfloat tiny_accent_scale = 0.4;

// This is the factor by which CT_squash scales the character in X and Y.
static const PN_stdfloat squash_accent_scale_x = 0.8f;
static const PN_stdfloat squash_accent_scale_y = 0.5f;

// This is the factor by which CT_small_squash scales the character in X and
// Y.
static const PN_stdfloat small_squash_accent_scale_x = 0.6f;
static const PN_stdfloat small_squash_accent_scale_y = 0.3;

// This is the factor by which the advance is reduced for the first character
// of a two-character ligature.
static const PN_stdfloat ligature_advance_scale = 0.6f;


/**
 * An internal function that works like isspace() but is safe to call for a
 * wide character.
 */
static INLINE bool
isspacew(unsigned int ch) {
  return isascii(ch) && isspace(ch);
}

/**
 * An internal function, similar to isspace(), except it does not consider
 * newlines to be whitespace.  It also includes the soft-hyphen character.
 */
static INLINE bool
isbreakpoint(unsigned int ch) {
  return (ch == ' ' || ch == '\t' ||
          ch == (unsigned int)text_soft_hyphen_key ||
          ch == (unsigned int)text_soft_break_key);
}


/**
 *
 */
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

/**
 *
 */
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

/**
 *
 */
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

/**
 *
 */
TextAssembler::
~TextAssembler() {
}

/**
 * Reinitializes the contents of the TextAssembler.
 */
void TextAssembler::
clear() {
  _ul.set(0.0f, 0.0f);
  _lr.set(0.0f, 0.0f);
  _next_row_ypos = 0.0f;

  _text_string.clear();
  _text_block.clear();
}

/**
 * Accepts a new text string and associated properties structure, and
 * precomputes the wordwrapping layout appropriately.  After this call,
 * get_wordwrapped_wtext() and get_num_rows() can be called.
 *
 * The return value is true if all the text is accepted, or false if some was
 * truncated (see set_max_rows()).
 */
bool TextAssembler::
set_wtext(const wstring &wtext) {
  clear();

  // First, expand all of the embedded TextProperties references within the
  // string.
  wstring::const_iterator si = wtext.begin();
  scan_wtext(_text_string, si, wtext.end(), _initial_cprops);

  while (si != wtext.end()) {
    // If we returned without consuming the whole string, it means there was
    // an embedded text_pop_properties_key that didn't match the push.  That's
    // worth a warning, and then go back and pick up the rest of the string.
    text_cat.warning()
      << "pop_properties encountered without preceding push_properties.\n";
    scan_wtext(_text_string, si, wtext.end(), _initial_cprops);
  }

  // Then apply any wordwrap requirements.
  return wordwrap_text();
}

/**
 * Replaces the 'count' characters from 'start' of the current text with the
 * indicated replacement text.  If the replacement text does not have count
 * characters, the length of the string will be changed accordingly.
 *
 * The substring may include nested formatting characters, but they must be
 * self-contained and self-closed.  The formatting characters are not
 * literally saved in the internal string; they are parsed at the time of the
 * set_wsubstr() call.
 *
 * The return value is true if all the text is accepted, or false if some was
 * truncated (see set_max_rows()).
 */
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

/**
 * Returns a wstring that represents the contents of the text, without any
 * embedded properties characters.  If there is an embedded graphic object, a
 * zero value is inserted in that position.
 *
 * This string has the same length as get_num_characters(), and the characters
 * in this string correspond one-to-one with the characters returned by
 * get_character(n).
 */
wstring TextAssembler::
get_plain_wtext() const {
  wstring wtext;

  TextString::const_iterator si;
  for (si = _text_string.begin(); si != _text_string.end(); ++si) {
    const TextCharacter &tch = (*si);
    if (tch._graphic == nullptr) {
      wtext += tch._character;
    } else {
      wtext.push_back(0);
    }
  }

  return wtext;
}

/**
 * Returns a wstring that represents the contents of the text, with newlines
 * inserted according to the wordwrapping.  The string will contain no
 * embedded properties characters.  If there is an embedded graphic object, a
 * zero value is inserted in that position.
 *
 * This string has the same number of newline characters as get_num_rows(),
 * and the characters in this string correspond one-to-one with the characters
 * returned by get_character(r, c).
 */
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
      if (tch._graphic == nullptr) {
        wtext += tch._character;
      } else {
        wtext.push_back(0);
      }
    }
  }

  return wtext;
}

/**
 * Returns a wstring that represents the contents of the text.
 *
 * The string will contain embedded properties characters, which may not
 * exactly match the embedded properties characters of the original string,
 * but it will encode the same way.
 */
wstring TextAssembler::
get_wtext() const {
  wstring wtext;
  PT(ComputedProperties) current_cprops = _initial_cprops;

  TextString::const_iterator si;
  for (si = _text_string.begin(); si != _text_string.end(); ++si) {
    const TextCharacter &tch = (*si);
    current_cprops->append_delta(wtext, tch._cprops);
    if (tch._graphic == nullptr) {
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

/**
 * Returns a wstring that represents the contents of the text, with newlines
 * inserted according to the wordwrapping.
 *
 * The string will contain embedded properties characters, which may not
 * exactly match the embedded properties characters of the original string,
 * but it will encode the same way.
 *
 * Embedded properties characters will be closed before every newline, then
 * reopened (if necessary) on the subsequent character following the newline.
 * This means it will be safe to divide the text up at the newline characters
 * and treat each line as an independent piece.
 */
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
      if (tch._graphic == nullptr) {
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

/**
 * Computes the row and column index of the nth character or graphic object in
 * the text.  Fills r and c accordingly.
 *
 * Returns true if the nth character is valid and has a corresponding r and c
 * position, false otherwise (for instance, a soft-hyphen character, or a
 * newline character, may not have a corresponding position). In either case,
 * r and c will be filled in sensibly.
 */
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
    // If there are any soft hyphen or soft break keys in the source text, we
    // have to scan past them to get c precisely.
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
    // If there are no soft characters, then the string maps one-to-one.
    c = min(n - row._row_start, (int)row._string.size());
    if (_text_string[n - 1]._character == '\n') {
      is_real_char = false;
    }
  }

  return is_real_char;
}

/**
 * Computes the character index of the character at the rth row and cth column
 * position.  This is the inverse of calc_r_c().
 *
 * It is legal for c to exceed the index number of the last column by 1, and
 * it is legal for r to exceed the index number of the last row by 1, if c is
 * 0.
 */
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
      // If there are any soft hyphen or soft break keys in the source text,
      // we have to scan past them to get n precisely.
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
      // If there are no soft characters, then the string maps one-to-one.
      return row._row_start + c;
    }
  }
}

/**
 * Returns the x position of the origin of the character or graphic object at
 * the indicated position in the indicated row.
 *
 * It is legal for c to exceed the index number of the last column by 1, and
 * it is legal for r to exceed the index number of the last row by 1, if c is
 * 0.
 */
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

/**
 * Actually assembles all of the text into a GeomNode, and returns the node
 * (or possibly a parent of the node, to keep the shadow separate).  Once this
 * has been called, you may query the extents of the text via get_ul(),
 * get_lr().
 */
PT(PandaNode) TextAssembler::
assemble_text() {
  // Now assemble the text into glyphs.
  PlacedGlyphs placed_glyphs;
  assemble_paragraph(placed_glyphs);

  // Now that we have a bunch of GlyphPlacements, pull out the Geoms and put
  // them under a common node.
  PT(PandaNode) parent_node = new PandaNode("common");

  PT(PandaNode) shadow_node = new PandaNode("shadow");
  PT(GeomNode) shadow_geom_node = new GeomNode("shadow_geom");
  shadow_node->add_child(shadow_geom_node);

  PT(PandaNode) text_node = new PandaNode("text");
  PT(GeomNode) text_geom_node = new GeomNode("text_geom");
  text_node->add_child(text_geom_node);

  const TextProperties *properties = nullptr;
  CPT(RenderState) text_state;
  CPT(RenderState) shadow_state;
  LVector2 shadow(0);

  bool any_shadow = false;

  GeomCollectorMap geom_collector_map;
  GeomCollectorMap geom_shadow_collector_map;
  QuadMap quad_map;
  QuadMap quad_shadow_map;

  PlacedGlyphs::const_iterator pgi;
  for (pgi = placed_glyphs.begin(); pgi != placed_glyphs.end(); ++pgi) {
    const GlyphPlacement &placement = (*pgi);

    if (placement._properties != properties) {
      // Get a new set of properties for future glyphs.
      properties = placement._properties;
      text_state = properties->get_text_state();

      if (properties->has_shadow()) {
        shadow = properties->get_shadow();
        shadow_state = properties->get_shadow_state();
      } else {
        shadow.set(0, 0);
        shadow_state.clear();
      }
    }

    if (!placement._glyph.is_null()) {
      if (properties->has_shadow()) {
        if (_dynamic_merge) {
          if (placement._glyph->has_quad()) {
            placement.assign_quad_to(quad_shadow_map, shadow_state, shadow);
          } else {
            placement.assign_append_to(geom_shadow_collector_map, shadow_state, shadow);
          }
        } else {
          placement.assign_to(shadow_geom_node, shadow_state, shadow);
        }

        // Don't shadow the graphics.  That can result in duplication of button
        // objects, plus it looks weird.  If you want a shadowed graphic, you
        // can shadow it yourself before you add it.
        // placement.copy_graphic_to(shadow_node, shadow_state, shadow);
        any_shadow = true;
      }

      if (_dynamic_merge) {
        if (placement._glyph->has_quad()) {
          placement.assign_quad_to(quad_map, text_state);
        } else {
          placement.assign_append_to(geom_collector_map, text_state);
        }
      } else {
        placement.assign_to(text_geom_node, text_state);
      }
    }
    placement.copy_graphic_to(text_node, text_state);
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

  generate_quads(text_geom_node, quad_map);

  if (any_shadow) {
    for (gc = geom_shadow_collector_map.begin();
         gc != geom_shadow_collector_map.end();
         ++gc) {
      (*gc).second.append_geom(shadow_geom_node, (*gc).first._state);
    }

    generate_quads(shadow_geom_node, quad_shadow_map);
  }

  parent_node->add_child(text_node);

  return parent_node;
}

/**
 * Returns the width of a single character, according to its associated font.
 * This also correctly calculates the width of cheesy ligatures and accented
 * characters, which may not exist in the font as such.
 *
 * This does not take kerning into account, however.
 */
PN_stdfloat TextAssembler::
calc_width(wchar_t character, const TextProperties &properties) {
  if (character == ' ') {
    // A space is a special case.
    TextFont *font = properties.get_font();
    nassertr(font != nullptr, 0.0f);
    return font->get_space_advance() * properties.get_glyph_scale() * properties.get_text_scale();
  }

  bool got_glyph;
  CPT(TextGlyph) first_glyph;
  CPT(TextGlyph) second_glyph;
  UnicodeLatinMap::AccentType accent_type;
  int additional_flags;
  PN_stdfloat glyph_scale;
  PN_stdfloat advance_scale;
  get_character_glyphs(character, &properties,
                       got_glyph, first_glyph, second_glyph, accent_type,
                       additional_flags, glyph_scale, advance_scale);

  PN_stdfloat advance = 0.0f;

  if (first_glyph != nullptr) {
    advance = first_glyph->get_advance() * advance_scale;
  }
  if (second_glyph != nullptr) {
    advance += second_glyph->get_advance();
  }

  glyph_scale *= properties.get_glyph_scale() * properties.get_text_scale();

  return advance * glyph_scale;
}

/**
 * Returns the width of a single TextGraphic image.
 */
PN_stdfloat TextAssembler::
calc_width(const TextGraphic *graphic, const TextProperties &properties) {
  LVecBase4 frame = graphic->get_frame();
  return (frame[1] - frame[0]) * properties.get_glyph_scale() * properties.get_text_scale();
}

/**
 * Returns true if the named character exists in the font exactly as named,
 * false otherwise.  Note that because Panda can assemble glyphs together
 * automatically using cheesy accent marks, this is not a reliable indicator
 * of whether a suitable glyph can be rendered for the character.  For that,
 * use has_character() instead.
 *
 * This returns true for whitespace and Unicode whitespace characters (if they
 * exist in the font), but returns false for characters that would render with
 * the "invalid glyph".  It also returns false for characters that would be
 * synthesized within Panda, but see has_character().
 */
bool TextAssembler::
has_exact_character(wchar_t character, const TextProperties &properties) {
  if (character == ' ' || character == '\n') {
    // A space is a special case.  Every font implicitly has a space.  We also
    // treat newlines specially.
    return true;
  }

  TextFont *font = properties.get_font();
  nassertr(font != nullptr, false);

  CPT(TextGlyph) glyph;
  return font->get_glyph(character, glyph);
}

/**
 * Returns true if the named character exists in the font or can be
 * synthesized by Panda, false otherwise.  (Panda can synthesize some accented
 * characters by combining similar-looking glyphs from the font.)
 *
 * This returns true for whitespace and Unicode whitespace characters (if they
 * exist in the font), but returns false for characters that would render with
 * the "invalid glyph".
 */
bool TextAssembler::
has_character(wchar_t character, const TextProperties &properties) {
  if (character == ' ' || character == '\n') {
    // A space is a special case.  Every font implicitly has a space.  We also
    // treat newlines specially.
    return true;
  }

  bool got_glyph;
  CPT(TextGlyph) first_glyph;
  CPT(TextGlyph) second_glyph;
  UnicodeLatinMap::AccentType accent_type;
  int additional_flags;
  PN_stdfloat glyph_scale;
  PN_stdfloat advance_scale;
  get_character_glyphs(character, &properties,
                       got_glyph, first_glyph, second_glyph, accent_type,
                       additional_flags, glyph_scale, advance_scale);
  return got_glyph;
}

/**
 * Returns true if the indicated character represents whitespace in the font,
 * or false if anything visible will be rendered for it.
 *
 * This returns true for whitespace and Unicode whitespace characters (if they
 * exist in the font), and returns false for any other characters, including
 * characters that do not exist in the font (these would be rendered with the
 * "invalid glyph", which is visible).
 *
 * Note that this function can be reliably used to identify Unicode whitespace
 * characters only if the font has all of the whitespace characters defined.
 * It will return false for any character not in the font, even if it is an
 * official Unicode whitespace character.
 */
bool TextAssembler::
is_whitespace(wchar_t character, const TextProperties &properties) {
  if (character == ' ' || character == '\n') {
    // A space or a newline is a special case.
    return true;
  }


  TextFont *font = properties.get_font();
  nassertr(font != nullptr, false);

  CPT(TextGlyph) glyph;
  if (!font->get_glyph(character, glyph)) {
    return false;
  }

  return glyph->is_whitespace();
}

/**
 * Scans through the text string, decoding embedded references to
 * TextProperties.  The decoded string is copied character-by-character into
 * _text_string.
 */
void TextAssembler::
scan_wtext(TextAssembler::TextString &output_string,
           wstring::const_iterator &si,
           const wstring::const_iterator &send,
           TextAssembler::ComputedProperties *current_cprops) {
  while (si != send) {
    if ((*si) == text_push_properties_key) {
      // This indicates a nested properties structure.  Pull off the name of
      // the TextProperties structure, which is everything until the next
      // text_push_properties_key.
      wstring wname;
      ++si;
      while (si != send && (*si) != text_push_properties_key) {
        wname += (*si);
        ++si;
      }

      if (si == send) {
        // We didn't close the text_push_properties_key.  That's an error.
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
          // The push was not closed by a pop.  That's not an error, since we
          // allow people to be sloppy about that; but we'll print a debug
          // message at least.
          text_cat.debug()
            << "push_properties not matched by pop_properties.\n";
        }
      }

    } else if ((*si) == text_pop_properties_key) {
      // This indicates the undoing of a previous push_properties_key.  We
      // simply return to the previous level.
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
        // We didn't close the text_embed_graphic_key.  That's an error.
        text_cat.warning()
          << "Unclosed embed_graphic in text.\n";
        return;
      }

      ++si;

      // Now we have to encode the wstring into a string, for lookup in the
      // TextPropertiesManager.
      std::string graphic_name = _encoder->encode_wtext(graphic_wname);

      TextPropertiesManager *manager =
        TextPropertiesManager::get_global_ptr();

      // Get the graphic image.
      const TextGraphic *named_graphic = manager->get_graphic_ptr(graphic_name);
      if (named_graphic != nullptr) {
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

/**
 * Inserts newlines into the _text_string at the appropriate places in order
 * to make each line be the longest possible line that is not longer than
 * wordwrap_width (and does not break any words, if possible).  Stores the
 * result in _text_block.
 *
 * If _max_rows is greater than zero, no more than _max_rows will be accepted.
 * Text beyond that will be truncated.
 *
 * The return value is true if all the text is accepted, or false if some was
 * truncated.
 */
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

    // Scan the next n characters, until the end of the string or an embedded
    // newline character, or we exceed wordwrap_width.

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
          // character to the right of the rightmost space.  Each time we
          // encounter a space, we reset this counter.
          any_hyphens = false;
          last_space = q;
          last_space_width = width;
          last_was_space = true;
        }
      } else {
        last_was_space = false;
      }

      // A soft hyphen character is not printed, but marks a point at which we
      // might hyphenate a word if we need to.
      if (_text_string[q]._character == text_soft_hyphen_key) {
        if (wordwrap_width > 0.0f) {
          // We only consider this as a possible hyphenation point if (a) it
          // is not the very first character, and (b) there is enough room for
          // a hyphen character to be printed following it.
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
      // If we stopped because we exceeded the wordwrap width, then try to
      // find an appropriate place to wrap the line or to hyphenate, if
      // necessary.
      nassertr(wordwrap_width > 0.0f, false);

      if (any_spaces && last_space_width / wordwrap_width >= text_hyphen_ratio) {
        // If we have a space that ended up within our safety margin, don't
        // use any soft-hyphen characters.
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
        // Otherwise, this is a forced break.  Accept the longest line we can
        // that does not leave the next line beginning with one of our
        // forbidden characters.
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
      // No characters got in at all.  This could only happen if the wordwrap
      // width is narrower than a single character, or if we have a
      // substantial number of leading spaces in a line.

      if (initial_width == 0.0f) {
        // There was no leading whitespace on the line, so the character
        // itself didn't fit within the margins.  Let it in anyway; what else
        // can we do?
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

/**
 * Returns the width of the soft-hyphen replacement string, according to the
 * indicated character's associated font.
 */
PN_stdfloat TextAssembler::
calc_hyphen_width(const TextCharacter &tch) {
  TextFont *font = tch._cprops->_properties.get_font();
  nassertr(font != nullptr, 0.0f);

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

/**
 * Generates Geoms for the given quads and adds them to the GeomNode.
 */
void TextAssembler::
generate_quads(GeomNode *geom_node, const QuadMap &quad_map) {
  QuadMap::const_iterator qmi;
  for (qmi = quad_map.begin(); qmi != quad_map.end(); ++qmi) {
    const QuadDefs &quads = qmi->second;
    GeomTextGlyph::Glyphs glyphs;
    glyphs.reserve(quads.size());

    const GeomVertexFormat *format = GeomVertexFormat::get_v3t2();
    PT(GeomVertexData) vdata = new GeomVertexData("text", format, Geom::UH_static);

    Thread *current_thread = Thread::get_current_thread();

    // This is quite a critical loop and GeomVertexWriter quickly becomes the
    // bottleneck.  So, I've written this out the hard way instead.  Two
    // versions of the loop: one for 32-bit floats, the other for 64-bit.
    {
      PT(GeomVertexArrayDataHandle) vtx_handle = vdata->modify_array_handle(0);
      vtx_handle->unclean_set_num_rows(quads.size() * 4);

      unsigned char *write_ptr = vtx_handle->get_write_pointer();

      if (format->get_vertex_column()->get_numeric_type() == GeomEnums::NT_float32) {
        // 32-bit vertex case.
        size_t stride = format->get_array(0)->get_stride() / sizeof(PN_float32);

        PN_float32 *vtx_ptr = (PN_float32 *)
          (write_ptr + format->get_column(InternalName::get_vertex())->get_start());
        PN_float32 *tex_ptr = (PN_float32 *)
          (write_ptr + format->get_column(InternalName::get_texcoord())->get_start());

        for (const QuadDef &quad : quads) {
          vtx_ptr[0] = quad._dimensions[0] + quad._slanth;
          vtx_ptr[1] = 0;
          vtx_ptr[2] = quad._dimensions[3];
          vtx_ptr += stride;

          tex_ptr[0] = quad._uvs[0];
          tex_ptr[1] = quad._uvs[3];
          tex_ptr += stride;

          vtx_ptr[0] = quad._dimensions[0] + quad._slantl;
          vtx_ptr[1] = 0;
          vtx_ptr[2] = quad._dimensions[1];
          vtx_ptr += stride;

          tex_ptr[0] = quad._uvs[0];
          tex_ptr[1] = quad._uvs[1];
          tex_ptr += stride;

          vtx_ptr[0] = quad._dimensions[2] + quad._slanth;
          vtx_ptr[1] = 0;
          vtx_ptr[2] = quad._dimensions[3];
          vtx_ptr += stride;

          tex_ptr[0] = quad._uvs[2];
          tex_ptr[1] = quad._uvs[3];
          tex_ptr += stride;

          vtx_ptr[0] = quad._dimensions[2] + quad._slantl;
          vtx_ptr[1] = 0;
          vtx_ptr[2] = quad._dimensions[1];
          vtx_ptr += stride;

          tex_ptr[0] = quad._uvs[2];
          tex_ptr[1] = quad._uvs[1];
          tex_ptr += stride;

          glyphs.push_back(move(quad._glyph));
        }
      } else {
        // 64-bit vertex case.
        size_t stride = format->get_array(0)->get_stride() / sizeof(PN_float64);

        PN_float64 *vtx_ptr = (PN_float64 *)
          (write_ptr + format->get_column(InternalName::get_vertex())->get_start());
        PN_float64 *tex_ptr = (PN_float64 *)
          (write_ptr + format->get_column(InternalName::get_texcoord())->get_start());

        for (const QuadDef &quad : quads) {
          vtx_ptr[0] = quad._dimensions[0] + quad._slanth;
          vtx_ptr[1] = 0;
          vtx_ptr[2] = quad._dimensions[3];
          vtx_ptr += stride;

          tex_ptr[0] = quad._uvs[0];
          tex_ptr[1] = quad._uvs[3];
          tex_ptr += stride;

          vtx_ptr[0] = quad._dimensions[0] + quad._slantl;
          vtx_ptr[1] = 0;
          vtx_ptr[2] = quad._dimensions[1];
          vtx_ptr += stride;

          tex_ptr[0] = quad._uvs[0];
          tex_ptr[1] = quad._uvs[1];
          tex_ptr += stride;

          vtx_ptr[0] = quad._dimensions[2] + quad._slanth;
          vtx_ptr[1] = 0;
          vtx_ptr[2] = quad._dimensions[3];
          vtx_ptr += stride;

          tex_ptr[0] = quad._uvs[2];
          tex_ptr[1] = quad._uvs[3];
          tex_ptr += stride;

          vtx_ptr[0] = quad._dimensions[2] + quad._slantl;
          vtx_ptr[1] = 0;
          vtx_ptr[2] = quad._dimensions[1];
          vtx_ptr += stride;

          tex_ptr[0] = quad._uvs[2];
          tex_ptr[1] = quad._uvs[1];
          tex_ptr += stride;

          glyphs.push_back(move(quad._glyph));
        }
      }
    }

    // Now write the indices.  Two cases: 32-bit indices and 16-bit indices.
    int vtx_count = quads.size() * 4;
    PT(GeomTriangles) tris = new GeomTriangles(Geom::UH_static);
    if (vtx_count > 65535) {
      tris->set_index_type(GeomEnums::NT_uint32);
    } else {
      tris->set_index_type(GeomEnums::NT_uint16);
    }
    {
      PT(GeomVertexArrayDataHandle) idx_handle = tris->modify_vertices_handle(current_thread);
      idx_handle->unclean_set_num_rows(quads.size() * 6);
      if (tris->get_index_type() == GeomEnums::NT_uint16) {
        // 16-bit index case.
        uint16_t *idx_ptr = (uint16_t *)idx_handle->get_write_pointer();

        for (int i = 0; i < vtx_count; i += 4) {
          *(idx_ptr++) = i + 0;
          *(idx_ptr++) = i + 1;
          *(idx_ptr++) = i + 2;
          *(idx_ptr++) = i + 2;
          *(idx_ptr++) = i + 1;
          *(idx_ptr++) = i + 3;
        }
      } else {
        // 32-bit index case.
        uint32_t *idx_ptr = (uint32_t *)idx_handle->get_write_pointer();

        for (int i = 0; i < vtx_count; i += 4) {
          *(idx_ptr++) = i + 0;
          *(idx_ptr++) = i + 1;
          *(idx_ptr++) = i + 2;
          *(idx_ptr++) = i + 2;
          *(idx_ptr++) = i + 1;
          *(idx_ptr++) = i + 3;
        }
      }
    }

    // We can compute this value much faster than GeomPrimitive can.
    tris->set_minmax(0, vtx_count - 1, nullptr, nullptr);

    PT(GeomTextGlyph) geom = new GeomTextGlyph(vdata);
    geom->_glyphs.swap(glyphs);
    geom->add_primitive(tris);
    geom_node->add_geom(geom, qmi->first);
  }
}

/**
 * Fills up placed_glyphs, _ul, _lr with the contents of _text_block.  Also
 * updates _xpos and _ypos within the _text_block structure.
 */
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

    // Store the index of the first glyph we're going to place.
    size_t first_glyph = placed_glyphs.size();

    // First, assemble all the glyphs of this row.
    PN_stdfloat row_width, line_height, wordwrap;
    TextProperties::Alignment align;
    assemble_row(row, placed_glyphs,
                 row_width, line_height, align, wordwrap);

    // Now move the row to its appropriate position.  This might involve a
    // horizontal as well as a vertical translation.
    if (num_rows == 0) {
      // If this is the first row, account for its space.
      _ul[1] = 0.8f * line_height;

    } else {
      // If it is not the first row, shift the text downward by line_height
      // from the previous row.
      ypos -= line_height;
    }
    _lr[1] = ypos - 0.2 * line_height;

    // Apply the requested horizontal alignment to the row.  [fabius] added a
    // different concept of text alignment based upon a boxed region where his
    // width is defined by the wordwrap size with the upper left corner
    // starting from 0,0,0 if the wordwrap size is unspecified the alignment
    // could eventually result wrong.
    PN_stdfloat xpos = 0;
    switch (align) {
    case TextProperties::A_left:
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

    row._xpos = xpos;
    row._ypos = ypos;

    // Now adjust the geoms we've assembled.
    for (size_t i = first_glyph; i < placed_glyphs.size(); ++i) {
      placed_glyphs[i]._xpos += xpos;
      placed_glyphs[i]._ypos += ypos;
    }

    // Advance to the next line.
    num_rows++;
    _next_row_ypos = ypos - line_height;
  }

  // num_rows may be smaller than _text_block.size(), if there are trailing
  // newlines on the string.
}

/**
 * Assembles the letters in the source string, up until the first newline or
 * the end of the string into a single row (which is parented to _geom_node),
 * and computes the length of the row and the maximum line_height of all the
 * fonts used in the row.  The source pointer is moved to the terminating
 * character.
 */
void TextAssembler::
assemble_row(TextAssembler::TextRow &row,
             TextAssembler::PlacedGlyphs &placed_glyphs,
             PN_stdfloat &row_width, PN_stdfloat &line_height,
             TextProperties::Alignment &align, PN_stdfloat &wordwrap) {
  Thread *current_thread = Thread::get_current_thread();

  line_height = 0.0f;
  PN_stdfloat xpos = 0.0f;
  align = TextProperties::A_left;

  // Remember previous character, for kerning.
  int prev_char = -1;

  bool underscore = false;
  PN_stdfloat underscore_start = 0.0f;
  const TextProperties *underscore_properties = nullptr;

#ifdef HAVE_HARFBUZZ
  const ComputedProperties *prev_cprops = nullptr;
  hb_buffer_t *harfbuff = nullptr;
#endif

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
        draw_underscore(placed_glyphs, underscore_start, xpos,
                        underscore_properties);
      }
      underscore = properties->get_underscore();
      underscore_start = xpos;
      underscore_properties = properties;
    }

    TextFont *font = properties->get_font();
    nassertv(font != nullptr);

    // We get the row's alignment property from the first character of the row
    if ((align == TextProperties::A_left) &&
        (properties->get_align() != TextProperties::A_left)) {
      align = properties->get_align();
    }

    // [fabius] a good place to take wordwrap size
    if (properties->get_wordwrap() > 0.0f) {
      wordwrap = properties->get_wordwrap();
    }

    // And the height of the row is the maximum of all the fonts used within
    // the row.
    if (graphic != nullptr) {
      LVecBase4 frame = graphic->get_frame();
      line_height = max(line_height, frame[3] - frame[2]);
    } else {
      line_height = max(line_height, font->get_line_height() * properties->get_glyph_scale() * properties->get_text_scale());
    }

#ifdef HAVE_HARFBUZZ
    if (tch._cprops != prev_cprops || graphic != nullptr) {
      if (harfbuff != nullptr && hb_buffer_get_length(harfbuff) > 0) {
        // Shape the buffer accumulated so far.
        shape_buffer(harfbuff, placed_glyphs, xpos, prev_cprops->_properties);
        hb_buffer_reset(harfbuff);

      } else if (harfbuff == nullptr && text_use_harfbuzz &&
                 font->is_of_type(DynamicTextFont::get_class_type())) {
        harfbuff = hb_buffer_create();
      }
      prev_cprops = tch._cprops;
    }

    if (graphic == nullptr && harfbuff != nullptr) {
      hb_buffer_add(harfbuff, character, character);
      continue;
    }
#endif

    if (character == ' ') {
      // A space is a special case.
      xpos += properties->get_glyph_scale() * properties->get_text_scale() * font->get_space_advance();
      prev_char = -1;

    } else if (character == '\t') {
      // So is a tab character.
      PN_stdfloat tab_width = properties->get_tab_width();
      xpos = (floor(xpos / tab_width) + 1.0f) * tab_width;
      prev_char = -1;

    } else if (character == text_soft_hyphen_key) {
      // And so is the 'soft-hyphen' key character.

    } else if (graphic != nullptr) {
      // A special embedded graphic.
      GlyphPlacement placement;

      PT(PandaNode) model = graphic->get_model().node();
      if (graphic->get_instance_flag()) {
        // Instance the model in.  Create a ModelNode so it doesn't get
        // flattened.
        PT(ModelNode) model_node = new ModelNode("");
        model_node->set_preserve_transform(ModelNode::PT_no_touch);
        model_node->add_child(model);
        placement._graphic_model = model_node.p();
      } else {
        // Copy the model in.  This the preferred way; it's a little cheaper
        // to render than instancing (because flattening is more effective).
        placement._graphic_model = model->copy_subgraph();
      }

      LVecBase4 frame = graphic->get_frame();
      PN_stdfloat glyph_scale = properties->get_glyph_scale() * properties->get_text_scale();

      PN_stdfloat advance = (frame[1] - frame[0]);

      // Now compute the matrix that will transform the glyph (or glyphs) into
      // position.
      placement._scale = properties->get_glyph_scale();
      placement._xpos = (xpos - frame[0]);
      placement._ypos = (properties->get_glyph_shift() - frame[2]);
      placement._slant = properties->get_slant();
      placement._properties = properties;

      placed_glyphs.push_back(placement);

      xpos += advance * glyph_scale;
      prev_char = -1;

    } else {
      // A printable character.
      bool got_glyph;
      CPT(TextGlyph) first_glyph;
      CPT(TextGlyph) second_glyph;
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

      glyph_scale *= properties->get_glyph_scale() * properties->get_text_scale();

      // Add the kerning delta.
      if (text_kerning) {
        if (prev_char != -1) {
          xpos += font->get_kerning(prev_char, character) * glyph_scale;
        }
        prev_char = character;
      }

      // Build up a GlyphPlacement, indicating all of the Geoms that go into
      // this character.  Normally, there is only one Geom per character, but
      // it may involve multiple Geoms if we need to add cheesy accents or
      // ligatures.
      GlyphPlacement placement;

      placement._glyph = nullptr;
      placement._scale = glyph_scale;
      placement._xpos = xpos;
      placement._ypos = properties->get_glyph_shift();
      placement._slant = properties->get_slant();
      placement._properties = properties;

      PN_stdfloat advance = 0.0f;

      if (accent_type != UnicodeLatinMap::AT_none || additional_flags != 0) {
        // If we have some special handling to perform, do so now.  This will
        // probably require the bounding volume of the glyph, so go get that.
        LPoint3 min_vert, max_vert;
        bool found_any = false;
        if (first_glyph != nullptr) {
          first_glyph->calc_tight_bounds(min_vert, max_vert, found_any,
                                         current_thread);
        }
        if (second_glyph != nullptr) {
          second_glyph->calc_tight_bounds(min_vert, max_vert, found_any,
                                          current_thread);
        }

        if (found_any) {
          LPoint3 centroid = (min_vert + max_vert) / 2.0f;

          if ((additional_flags & UnicodeLatinMap::AF_turned) != 0) {
            // Invert the character.  Should we also invert the accent mark,
            // so that an accent that would have been above the glyph will now
            // be below it?  That's what we do here, which is probably the
            // right thing to do for n-tilde, but not for most of the rest of
            // the accent marks.  For now we'll assume there are no characters
            // with accent marks that also have the turned flag.

            // We rotate the character around its centroid, which may not
            // always be the right point, but it's the best we've got and it's
            // probably pretty close.
            placement._scale *= -1;
            placement._xpos += centroid[0] * 2;
            placement._ypos += centroid[2] * 2;
          }

          if (accent_type != UnicodeLatinMap::AT_none) {
            GlyphPlacement accent_placement(placement);
            tack_on_accent(accent_type, min_vert, max_vert, centroid,
                           properties, accent_placement);
            placed_glyphs.push_back(accent_placement);
          }
        }
      }

      if (first_glyph != nullptr) {
        advance = first_glyph->get_advance() * advance_scale;
        if (!first_glyph->is_whitespace()) {
          std::swap(placement._glyph, first_glyph);
          placed_glyphs.push_back(placement);
        }
      }

      // Check if there is a second glyph to create a hacky ligature or some
      // such nonsense.
      if (second_glyph != nullptr) {
        placement._xpos += advance * glyph_scale;
        advance += second_glyph->get_advance();
        std::swap(placement._glyph, second_glyph);
        placed_glyphs.push_back(placement);
      }

      xpos += advance * glyph_scale;
    }
  }

#ifdef HAVE_HARFBUZZ
  if (harfbuff != nullptr && hb_buffer_get_length(harfbuff) > 0) {
    shape_buffer(harfbuff, placed_glyphs, xpos, prev_cprops->_properties);
  }
  hb_buffer_destroy(harfbuff);
#endif

  if (underscore && underscore_start != xpos) {
    draw_underscore(placed_glyphs, underscore_start, xpos,
                    underscore_properties);
  }

  row_width = xpos;

  if (row._eol_cprops != nullptr) {
    // If there's an _eol_cprops, it represents the cprops of the newline
    // character that ended the line, which should also contribute towards the
    // line_height.

    const TextProperties *properties = &(row._eol_cprops->_properties);
    TextFont *font = properties->get_font();
    nassertv(font != nullptr);

    if (line_height == 0.0f) {
      PN_stdfloat glyph_scale = properties->get_glyph_scale() * properties->get_text_scale();
      line_height = max(line_height, font->get_line_height() * glyph_scale);
    }
  }
}

/**
 * Places the glyphs collected from a HarfBuzz buffer.
 */
void TextAssembler::
shape_buffer(hb_buffer_t *buf, PlacedGlyphs &placed_glyphs, PN_stdfloat &xpos,
             const TextProperties &properties) {

#ifdef HAVE_HARFBUZZ
  // If we did not specify a text direction, harfbuzz will guess it based on
  // the script we are using.
  hb_direction_t direction = HB_DIRECTION_INVALID;
  if (properties.has_direction()) {
    switch (properties.get_direction()) {
    case TextProperties::D_ltr:
      direction = HB_DIRECTION_LTR;
      break;
    case TextProperties::D_rtl:
      direction = HB_DIRECTION_RTL;
      break;
    }
  }
  hb_buffer_set_content_type(buf, HB_BUFFER_CONTENT_TYPE_UNICODE);
  hb_buffer_set_direction(buf, direction);
  hb_buffer_guess_segment_properties(buf);

  DynamicTextFont *font = DCAST(DynamicTextFont, properties.get_font());
  hb_font_t *hb_font = font->get_hb_font();
  hb_shape(hb_font, buf, nullptr, 0);

  PN_stdfloat glyph_scale = properties.get_glyph_scale() * properties.get_text_scale();
  PN_stdfloat scale = glyph_scale / (font->get_pixels_per_unit() * font->get_scale_factor() * 64.0);

  unsigned int glyph_count;
  hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(buf, &glyph_count);
  hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);

  for (unsigned int i = 0; i < glyph_count; ++i) {
    int character = glyph_info[i].cluster;
    int glyph_index = glyph_info[i].codepoint;

    CPT(TextGlyph) glyph;
    if (!font->get_glyph_by_index(character, glyph_index, glyph)) {
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

    PN_stdfloat advance = glyph_pos[i].x_advance * scale;
    if (glyph->is_whitespace()) {
      // A space is a special case.
      xpos += advance;
      continue;
    }

    PN_stdfloat x_offset = glyph_pos[i].x_offset * scale;
    PN_stdfloat y_offset = glyph_pos[i].y_offset * scale;

    // Build up a GlyphPlacement, indicating all of the Geoms that go into
    // this character.  Normally, there is only one Geom per character, but
    // it may involve multiple Geoms if we need to add cheesy accents or
    // ligatures.
    GlyphPlacement placement;
    placement._glyph = move(glyph);
    placement._scale = glyph_scale;
    placement._xpos = xpos + x_offset;
    placement._ypos = properties.get_glyph_shift() + y_offset;
    placement._slant = properties.get_slant();
    placement._properties = &properties;
    placed_glyphs.push_back(placement);

    xpos += advance;
  }
#endif
}

/**
 * Creates the geometry to render the underscore line for the indicated range
 * of glyphs in this row.
 */
void TextAssembler::
draw_underscore(TextAssembler::PlacedGlyphs &placed_glyphs,
                PN_stdfloat underscore_start, PN_stdfloat underscore_end,
                const TextProperties *underscore_properties) {

  CPT(GeomVertexFormat) format = GeomVertexFormat::get_v3cp();
  PT(GeomVertexData) vdata =
    new GeomVertexData("underscore", format, Geom::UH_static);
  vdata->unclean_set_num_rows(2);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter color(vdata, InternalName::get_color());

  PN_stdfloat y = underscore_properties->get_underscore_height();
  vertex.set_data3(underscore_start, 0.0f, y);
  color.set_data4(underscore_properties->get_text_color());
  vertex.set_data3(underscore_end, 0.0f, y);
  color.set_data4(underscore_properties->get_text_color());

  PT(GeomLines) lines = new GeomLines(Geom::UH_static);
  lines->add_next_vertices(2);
  lines->close_primitive();

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(lines);

  PT(TextGlyph) glyph = new TextGlyph(0, geom, RenderState::make_empty(), 0);

  // Eventually we should probably replace this with the set_quad approach, or
  // better, for improved performance.
  // glyph->set_quad(LVecBase4(underscore_start, y, underscore_end, y+0.1),
  // LVecBase4(0), RenderState::make_empty());

  GlyphPlacement placement;
  placement._glyph = move(glyph);
  placement._xpos = 0;
  placement._ypos = 0;
  placement._scale = 1;
  placement._slant = 0;
  placement._properties = underscore_properties;
  placed_glyphs.push_back(placement);
}

/**
 * Looks up the glyph(s) from the font for the appropriate character.  If the
 * desired glyph isn't available (especially in the case of an accented
 * letter), tries to find a suitable replacement.  Normally, only one glyph is
 * returned per character, but in the case in which we have to simulate a
 * missing ligature in the font, two glyphs might be returned.
 *
 * All parameters except the first two are output parameters.  got_glyph is
 * set true if the glyph (or an acceptable substitute) is successfully found,
 * false otherwise; but even if it is false, glyph might still be non-NULL,
 * indicating a stand-in glyph for a missing character.
 */
void TextAssembler::
get_character_glyphs(int character, const TextProperties *properties,
                     bool &got_glyph, CPT(TextGlyph) &glyph,
                     CPT(TextGlyph) &second_glyph,
                     UnicodeLatinMap::AccentType &accent_type,
                     int &additional_flags,
                     PN_stdfloat &glyph_scale, PN_stdfloat &advance_scale) {
  TextFont *font = properties->get_font();
  nassertv_always(font != nullptr);

  got_glyph = false;
  glyph = nullptr;
  second_glyph = nullptr;
  accent_type = UnicodeLatinMap::AT_none;
  additional_flags = 0;
  glyph_scale = 1.0f;
  advance_scale = 1.0f;

  // Maybe we should remap the character to something else--e.g.  a small
  // capital.
  const UnicodeLatinMap::Entry *map_entry =
    UnicodeLatinMap::look_up((char32_t)character);
  if (map_entry != nullptr) {
    if (properties->get_small_caps() &&
        map_entry->_toupper_character != (char32_t)character) {
      character = map_entry->_toupper_character;
      map_entry = UnicodeLatinMap::look_up(character);
      glyph_scale = properties->get_small_caps_scale();
    }
  }

  got_glyph = font->get_glyph(character, glyph);
  if (!got_glyph && map_entry != nullptr && map_entry->_ascii_equiv != 0) {
    // If we couldn't find the Unicode glyph, try the ASCII equivalent
    // (without the accent marks).
    if (map_entry->_ascii_equiv == 'i') {
      // Special case for the i: we want to try the dotless variant first.
      got_glyph = font->get_glyph(0x0131, glyph) ||
                  font->get_glyph('i', glyph);

    } else if (map_entry->_ascii_equiv == 'j') {
      // And the dotless j as well.
      got_glyph = font->get_glyph(0x0237, glyph) ||
                  font->get_glyph('j', glyph);

    } else {
      got_glyph = font->get_glyph(map_entry->_ascii_equiv, glyph);
    }

    if (!got_glyph && map_entry->_toupper_character != (char32_t)character) {
      // If we still couldn't find it, try the uppercase equivalent.
      character = map_entry->_toupper_character;
      map_entry = UnicodeLatinMap::look_up(character);
      if (map_entry != nullptr) {
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
        // If we have two letters that are supposed to be in a ligature, just
        // jam them together.
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

/**
 * This is a cheesy attempt to tack on an accent to an ASCII letter for which
 * we don't have the appropriate already-accented glyph in the font.
 */
void TextAssembler::
tack_on_accent(UnicodeLatinMap::AccentType accent_type,
               const LPoint3 &min_vert, const LPoint3 &max_vert,
               const LPoint3 &centroid,
               const TextProperties *properties,
               TextAssembler::GlyphPlacement &placement) const {

  // Look for a combining accent mark character.
  wchar_t combine_char = UnicodeLatinMap::get_combining_accent(accent_type);
  if (combine_char != 0 &&
      tack_on_accent(combine_char, CP_above, CT_none, min_vert, max_vert,
                     centroid, properties, placement)) {
    return;
  }


  switch (accent_type) {
  case UnicodeLatinMap::AT_grave:
    // We use the slash as the grave and acute accents.  ASCII does have a
    // grave accent character, but a lot of fonts put the reverse apostrophe
    // there instead.  And some fonts (particularly fonts from mf) don't even
    // do backslash.
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
   tack_on_accent(0xb8, CP_below, CT_none, min_vert, max_vert, centroid,
                   properties, placement) ||
      tack_on_accent('c', CP_bottom, CT_tiny_mirror_x, min_vert, max_vert, centroid,
                     properties, placement);
    // tack_on_accent(',', CP_bottom, CT_none, min_vert, max_vert, centroid,
    // properties, placement);
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

/**
 * Generates a cheesy accent mark above (or below, etc.) the character.
 * Returns true if successful, or false if the named accent character doesn't
 * exist in the font.
 */
bool TextAssembler::
tack_on_accent(wchar_t accent_mark, TextAssembler::CheesyPosition position,
               TextAssembler::CheesyTransform transform,
               const LPoint3 &min_vert, const LPoint3 &max_vert,
               const LPoint3 &centroid,
               const TextProperties *properties,
               TextAssembler::GlyphPlacement &placement) const {
  TextFont *font = properties->get_font();
  nassertr(font != nullptr, false);

  Thread *current_thread = Thread::get_current_thread();

  CPT(TextGlyph) accent_glyph;
  if (font->get_glyph(accent_mark, accent_glyph) ||
      font->get_glyph(toupper(accent_mark), accent_glyph)) {
    if (!accent_glyph->is_whitespace()) {
      LPoint3 min_accent, max_accent;
      bool found_any = false;
      accent_glyph->calc_tight_bounds(min_accent, max_accent, found_any,
                                      current_thread);
      if (found_any) {
        PN_stdfloat t, u;
        LMatrix4 accent_mat;
        bool has_mat = true;

        switch (transform) {
        case CT_none:
          has_mat = false;
          break;

        case CT_mirror_x:
          accent_mat = LMatrix4::scale_mat(-1.0f, -1.0f, 1.0f);
          t = min_accent[0];
          min_accent[0] = -max_accent[0];
          max_accent[0] = -t;
          break;

        case CT_mirror_y:
          accent_mat = LMatrix4::scale_mat(1.0f, -1.0f, -1.0f);
          t = min_accent[2];
          min_accent[2] = -max_accent[2];
          max_accent[2] = -t;
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
          has_mat = false;
          placement._scale *= -1;
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
          accent_mat = LMatrix4::scale_mat(squash_accent_scale_x, -1.0f, -squash_accent_scale_y);
          min_accent[0] *= squash_accent_scale_x;
          max_accent[0] *= squash_accent_scale_x;
          t = min_accent[2];
          min_accent[2] = -max_accent[2] * squash_accent_scale_y;
          max_accent[2] = -t * squash_accent_scale_y;
          break;

        case CT_squash_mirror_diag:
          accent_mat =
            LMatrix4::rotate_mat_normaxis(270.0f, LVecBase3(0.0f, -1.0f, 0.0f)) *
            LMatrix4::scale_mat(-squash_accent_scale_x, -1.0f, squash_accent_scale_y);

          // rotate min, max
          t = min_accent[0];
          u = max_accent[0];
          min_accent[0] = min_accent[2] * -squash_accent_scale_x;
          max_accent[0] = max_accent[2] * -squash_accent_scale_x;
          min_accent[2] = -u * squash_accent_scale_y;
          max_accent[2] = -t * squash_accent_scale_y;
          break;

        case CT_small_squash:
          accent_mat = LMatrix4::scale_mat(small_squash_accent_scale_x, 1.0f, small_squash_accent_scale_y);
          min_accent[0] *= small_squash_accent_scale_x;
          max_accent[0] *= small_squash_accent_scale_x;
          min_accent[2] *= small_squash_accent_scale_y;
          max_accent[2] *= small_squash_accent_scale_y;
          break;

        case CT_small_squash_mirror_y:
          accent_mat = LMatrix4::scale_mat(small_squash_accent_scale_x, -1.0f, -small_squash_accent_scale_y);
          min_accent[0] *= small_squash_accent_scale_x;
          max_accent[0] *= small_squash_accent_scale_x;
          t = min_accent[2];
          min_accent[2] = -max_accent[2] * small_squash_accent_scale_y;
          max_accent[2] = -t * small_squash_accent_scale_y;
          break;

        case CT_small_squash_mirror_diag:
          accent_mat =
            LMatrix4::rotate_mat_normaxis(270.0f, LVecBase3(0.0f, -1.0f, 0.0f)) *
            LMatrix4::scale_mat(-small_squash_accent_scale_x, -1.0f, small_squash_accent_scale_y);

          // rotate min, max
          t = min_accent[0];
          u = max_accent[0];
          min_accent[0] = min_accent[2] * -small_squash_accent_scale_x;
          max_accent[0] = max_accent[2] * -small_squash_accent_scale_x;
          min_accent[2] = -u * small_squash_accent_scale_y;
          max_accent[2] = -t * small_squash_accent_scale_y;
          break;

        case CT_small:
          has_mat = false;
          placement._scale *= small_accent_scale;
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
          has_mat = false;
          placement._scale *= tiny_accent_scale;
          min_accent *= tiny_accent_scale;
          max_accent *= tiny_accent_scale;
          break;

        case CT_tiny_mirror_x:
          accent_mat = LMatrix4::scale_mat(-tiny_accent_scale, -1.0f, tiny_accent_scale);

          t = min_accent[0];
          min_accent[0] = -max_accent[0] * tiny_accent_scale;
          max_accent[0] = -t * tiny_accent_scale;
          min_accent[2] *= tiny_accent_scale;
          max_accent[2] *= tiny_accent_scale;
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

        default:
          has_mat = false;
        }

        PN_stdfloat total_margin = font->get_total_poly_margin();

        LPoint3 accent_centroid = (min_accent + max_accent) / 2.0f;
        PN_stdfloat accent_height = max_accent[2] - min_accent[2] - total_margin * 2;
        PN_stdfloat accent_x = centroid[0] - accent_centroid[0];
        PN_stdfloat accent_y = 0;
        PN_stdfloat min_y = min_vert[2] + total_margin;
        PN_stdfloat max_y = max_vert[2] - total_margin;

        switch (position) {
        case CP_above:
          // A little above the character.
          accent_y = max_y - accent_centroid[2] + accent_height * 0.75f;
          break;

        case CP_below:
          // A little below the character.
          accent_y = min_y - accent_centroid[2] - accent_height * 0.75f;
          break;

        case CP_top:
          // Touching the top of the character.
          accent_y = max_y - accent_centroid[2];
          break;

        case CP_bottom:
          // Touching the bottom of the character.
          accent_y = min_y - accent_centroid[2];
          break;

        case CP_within:
          // Centered within the character.
          accent_y = centroid[2] - accent_centroid[2];
          break;
        }

        placement._xpos += placement._scale * (accent_x + placement._slant * accent_y);
        placement._ypos += placement._scale * accent_y;

        if (has_mat) {
          // Some non-trivial transformation.  Apply it to the Geom.
          PT(Geom) accent_geom = accent_glyph->get_geom(_usage_hint);
          accent_geom->transform_vertices(accent_mat);
          placement._glyph = new TextGlyph(0, accent_geom, accent_glyph->get_state(), 0);
        } else {
          // A trivial transformation.
          placement._glyph = accent_glyph;
        }

        return true;
      }
    }
  }
  return false;
}

/**
 * Appends to wtext the control sequences necessary to change from this
 * ComputedProperties to the indicated ComputedProperties.
 */
void TextAssembler::ComputedProperties::
append_delta(wstring &wtext, TextAssembler::ComputedProperties *other) {
  if (this != other) {
    if (_depth > other->_depth) {
      // Back up a level from this properties.
      nassertv(_based_on != nullptr);

      wtext.push_back(text_pop_properties_key);
      _based_on->append_delta(wtext, other);

    } else if (other->_depth > _depth) {
      // Back up a level from the other properties.
      nassertv(other->_based_on != nullptr);

      append_delta(wtext, other->_based_on);
      wtext.push_back(text_push_properties_key);
      wtext += other->_wname;
      wtext.push_back(text_push_properties_key);

    } else if (_depth != 0) {
      // Back up a level from both properties.
      nassertv(_based_on != nullptr && other->_based_on != nullptr);

      wtext.push_back(text_pop_properties_key);
      _based_on->append_delta(wtext, other->_based_on);
      wtext.push_back(text_push_properties_key);
      wtext += other->_wname;
      wtext.push_back(text_push_properties_key);
    }
  }
}

/**
 * Puts the pieces of the GlyphPlacement in the indicated GeomNode.  The
 * vertices of the Geoms are modified by this operation.
 */
void TextAssembler::GlyphPlacement::
assign_to(GeomNode *geom_node, const RenderState *state,
          const LVector2 &offset) const {

  LMatrix4 xform(_scale, 0.0f, 0.0f, 0.0f,
                 0.0f, 1.0f, 0.0f, 0.0f,
                 _slant * _scale, 0.0f, _scale, 0.0f,
                 _xpos + offset[0], 0.0f, _ypos - offset[1], 1.0f);

  PT(Geom) geom = _glyph->get_geom(GeomEnums::UH_static);
  geom->transform_vertices(xform);
  geom_node->add_geom(geom, state->compose(_glyph->get_state()));
}

/**
 * Puts the pieces of the GlyphPlacement in the indicated GeomNode.  This
 * flavor will append the Geoms with the additional transform applied to the
 * vertices.
 */
void TextAssembler::GlyphPlacement::
assign_append_to(GeomCollectorMap &geom_collector_map,
                 const RenderState *state,
                 const LVector2 &offset) const {

  LMatrix4 xform(_scale, 0.0f, 0.0f, 0.0f,
                 0.0f, 1.0f, 0.0f, 0.0f,
                 _slant * _scale, 0.0f, _scale, 0.0f,
                 _xpos + offset[0], 0.0f, _ypos - offset[1], 1.0f);

  PT(Geom) geom = _glyph->get_geom(GeomEnums::UH_static);

  int sp, s, e, i;

  const GeomVertexData *vdata = geom->get_vertex_data();
  CPT(RenderState) rs = _glyph->get_state()->compose(state);
  GeomCollectorKey key(rs, vdata->get_format());

  GeomCollectorMap::iterator mi = geom_collector_map.find(key);
  if (mi == geom_collector_map.end()) {
    mi = geom_collector_map.insert(GeomCollectorMap::value_type(key, GeomCollector(vdata->get_format()))).first;
  }
  GeomCollector &geom_collector = (*mi).second;
  geom_collector.count_geom(geom);

  // We use this map to keep track of vertex indices we have already added, so
  // that we don't needlessly duplicate vertices into our output vertex data.
  VertexIndexMap vimap;

  for (size_t p = 0; p < geom->get_num_primitives(); ++p) {
    CPT(GeomPrimitive) primitive = geom->get_primitive(p)->decompose();

    // Get a new GeomPrimitive of the corresponding type.
    GeomPrimitive *new_prim = geom_collector.get_primitive(primitive->get_type());

    // Walk through all of the components (e.g.  triangles) of the primitive.
    for (sp = 0; sp < primitive->get_num_primitives(); sp++) {
      s = primitive->get_primitive_start(sp);
      e = primitive->get_primitive_end(sp);

      // Walk through all of the vertices in the component.
      for (i = s; i < e; i++) {
        int vi = primitive->get_vertex(i);

        // Attempt to insert number "vi" into the map.
        std::pair<VertexIndexMap::iterator, bool> added = vimap.insert(VertexIndexMap::value_type(vi, 0));
        int new_vertex;
        if (added.second) {
          // The insert succeeded.  That means this is the first time we have
          // encountered this vertex.
          new_vertex = geom_collector.append_vertex(vdata, vi, xform);
          // Update the map with the newly-created target vertex index.
          (*(added.first)).second = new_vertex;

        } else {
          // The insert failed.  This means we have previously encountered
          // this vertex, and we have already entered its target vertex index
          // into the vimap.  Extract that vertex index, so we can reuse it.
          new_vertex = (*(added.first)).second;
        }
        new_prim->add_vertex(new_vertex);
      }
      new_prim->close_primitive();
    }
  }
}

/**
 * If this glyph is representable as a single quad, assigns it to the
 * appropriate position in the map.
 */
void TextAssembler::GlyphPlacement::
assign_quad_to(QuadMap &quad_map, const RenderState *state,
               const LVector2 &offset) const {

  QuadDef quad;
  if (_glyph->get_quad(quad._dimensions, quad._uvs)) {
    quad._dimensions *= _scale;
    quad._slantl = quad._dimensions[1] * _slant;
    quad._slanth = quad._dimensions[3] * _slant;
    quad._dimensions += LVecBase4(_xpos, _ypos, _xpos, _ypos);
    quad._dimensions += LVecBase4(offset[0], -offset[1], offset[0], -offset[1]);
    quad._glyph = _glyph;

    quad_map[state->compose(_glyph->get_state())].push_back(move(quad));
  }
}

/**
 * If the GlyphPlacement includes a special graphic, copies it to the
 * indicated node.
 */
void TextAssembler::GlyphPlacement::
copy_graphic_to(PandaNode *node, const RenderState *state) const {
  if (_graphic_model != nullptr) {
    // We need an intermediate node to hold the transform and state.
    PT(PandaNode) intermediate_node = new PandaNode("");
    node->add_child(intermediate_node);

    intermediate_node->set_transform(
      TransformState::make_pos_hpr_scale_shear(
        LVecBase3(_xpos, 0, _ypos),
        LVecBase3::zero(),
        LVecBase3(_scale, 1, _scale),
        LVecBase3(0, _slant, 0)
      )
    );
    intermediate_node->set_state(state);
    intermediate_node->add_child(_graphic_model);
  }
}

/**
 * constructs the GeomCollector class (Geom, GeomTriangles, vertexWriter,
 * texcoordWriter..)
 */
TextAssembler::GeomCollector::
GeomCollector(const GeomVertexFormat *format) :
  _vdata(new GeomVertexData("merged_geom", format, Geom::UH_static)),
  _geom(new GeomTextGlyph(_vdata))
{
}

/**
 *
 */
TextAssembler::GeomCollector::
GeomCollector(const TextAssembler::GeomCollector &copy) :
  _vdata(copy._vdata),
  _geom(copy._geom)
{
}

/**
 * Returns a GeomPrimitive of the appropriate type.  If one has not yet been
 * created, returns a newly-created one; if one has previously been created of
 * this type, returns the previously-created one.
 */
GeomPrimitive *TextAssembler::GeomCollector::
get_primitive(TypeHandle prim_type) {
  if (prim_type == GeomTriangles::get_class_type()) {
    if (_triangles == nullptr) {
      _triangles = new GeomTriangles(Geom::UH_static);
      _geom->add_primitive(_triangles);
    }
    return _triangles;

  } else if (prim_type == GeomLines::get_class_type()) {
    if (_lines == nullptr) {
      _lines = new GeomLines(Geom::UH_static);
      _geom->add_primitive(_lines);
    }
    return _lines;

  } else if (prim_type == GeomPoints::get_class_type()) {
    if (_points == nullptr) {
      _points = new GeomPoints(Geom::UH_static);
      _geom->add_primitive(_points);
    }
    return _points;
  }

  nassert_raise("unexpected primitive type");
  return nullptr;
}

/**
 * Adds one vertex to the GeomVertexData.  Returns the row number of the added
 * vertex.
 */
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


/**
 * closes the geomTriangles and appends the geom to the given GeomNode
 */
void TextAssembler::GeomCollector::
append_geom(GeomNode *geom_node, const RenderState *state) {
  if (_geom->get_num_primitives() > 0) {
    geom_node->add_geom(_geom, state);
  }
}
