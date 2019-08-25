/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textProperties.cxx
 * @author drose
 * @date 2004-04-06
 */

#include "textProperties.h"
#include "config_text.h"
#include "default_font.h"
#include "dynamicTextFont.h"
#include "staticTextFont.h"
#include "bamFile.h"
#include "fontPool.h"
#include "colorAttrib.h"
#include "cullBinAttrib.h"
#include "transparencyAttrib.h"
#include "zStream.h"

PT(TextFont) TextProperties::_default_font;
bool TextProperties::_loaded_default_font = false;

TypeHandle TextProperties::_type_handle;

/**
 *
 */
TextProperties::
TextProperties() :
  _specified(0),

  _small_caps(text_small_caps),
  _small_caps_scale(text_small_caps_scale),
  _slant(0.0f),
  _underscore(false),
  _underscore_height(0.0f),
  _align(A_left),
  _indent_width(0.0f),
  _wordwrap_width(0.0f),
  _preserve_trailing_whitespace(false),
  _text_color(1.0f, 1.0f, 1.0f, 1.0f),
  _shadow_color(0.0f, 0.0f, 0.0f, 1.0f),
  _shadow_offset(0.0f, 0.0f),
  _draw_order(1),
  _tab_width(text_tab_width),
  _glyph_scale(1.0f),
  _glyph_shift(0.0f),
  _text_scale(1.0f),
  _direction(D_rtl) {
}

/**
 *
 */
TextProperties::
TextProperties(const TextProperties &copy) {
  (*this) = copy;
  _text_state = copy._text_state;
  _shadow_state = copy._shadow_state;
}

/**
 *
 */
void TextProperties::
operator = (const TextProperties &copy) {
  _specified = copy._specified;

  _font = copy._font;
  _small_caps = copy._small_caps;
  _small_caps_scale = copy._small_caps_scale;
  _slant = copy._slant;
  _underscore = copy._underscore;
  _underscore_height = copy._underscore_height;
  _align = copy._align;
  _indent_width = copy._indent_width;
  _wordwrap_width = copy._wordwrap_width;
  _preserve_trailing_whitespace = copy._preserve_trailing_whitespace;
  _text_color = copy._text_color;
  _shadow_color = copy._shadow_color;
  _shadow_offset = copy._shadow_offset;
  _bin = copy._bin;
  _draw_order = copy._draw_order;
  _tab_width = copy._tab_width;
  _glyph_scale = copy._glyph_scale;
  _glyph_shift = copy._glyph_shift;
  _text_scale = copy._text_scale;
  _direction = copy._direction;

  _text_state.clear();
  _shadow_state.clear();
}

/**
 *
 */
bool TextProperties::
operator == (const TextProperties &other) const {
  if (_specified != other._specified) {
    return false;
  }

  if ((_specified & F_has_font) && _font != other._font) {
    return false;
  }
  if ((_specified & F_has_small_caps) && _small_caps != other._small_caps) {
    return false;
  }
  if ((_specified & F_has_small_caps_scale) && _small_caps_scale != other._small_caps_scale) {
    return false;
  }
  if ((_specified & F_has_slant) && _slant != other._slant) {
    return false;
  }
  if ((_specified & F_has_underscore) && _underscore != other._underscore) {
    return false;
  }
  if ((_specified & F_has_underscore_height) && _underscore_height != other._underscore_height) {
    return false;
  }
  if ((_specified & F_has_align) && _align != other._align) {
    return false;
  }
  if ((_specified & F_has_indent) && _indent_width != other._indent_width) {
    return false;
  }
  if ((_specified & F_has_wordwrap) && _wordwrap_width != other._wordwrap_width) {
    return false;
  }
  if ((_specified & F_has_preserve_trailing_whitespace) && _preserve_trailing_whitespace != other._preserve_trailing_whitespace) {
    return false;
  }
  if ((_specified & F_has_text_color) && _text_color != other._text_color) {
    return false;
  }
  if ((_specified & F_has_text_color) && _text_color != other._text_color) {
    return false;
  }
  if ((_specified & F_has_shadow_color) && _shadow_color != other._shadow_color) {
    return false;
  }
  if ((_specified & F_has_shadow) && _shadow_offset != other._shadow_offset) {
    return false;
  }
  if ((_specified & F_has_bin) && _bin != other._bin) {
    return false;
  }
  if ((_specified & F_has_draw_order) && _draw_order != other._draw_order) {
    return false;
  }
  if ((_specified & F_has_tab_width) && _tab_width != other._tab_width) {
    return false;
  }
  if ((_specified & F_has_glyph_scale) && _glyph_scale != other._glyph_scale) {
    return false;
  }
  if ((_specified & F_has_glyph_shift) && _glyph_shift != other._glyph_shift) {
    return false;
  }
  if ((_specified & F_has_text_scale) && _text_scale != other._text_scale) {
    return false;
  }
  if ((_specified & F_has_direction) && _direction != other._direction) {
    return false;
  }
  return true;
}

/**
 * Unsets all properties that have been specified so far, and resets the
 * TextProperties structure to its initial empty state.
 */
void TextProperties::
clear() {
  (*this) = TextProperties();
}

/**
 * Sets any properties that are explicitly specified in other on this object.
 * Leaves other properties unchanged.
 */
void TextProperties::
add_properties(const TextProperties &other) {
  if (other.has_font()) {
    set_font(other.get_font());
  }
  if (other.has_small_caps()) {
    set_small_caps(other.get_small_caps());
    set_small_caps_scale(other.get_small_caps_scale());
  }
  if (other.has_slant()) {
    set_slant(other.get_slant());
  }
  if (other.has_underscore()) {
    set_underscore(other.get_underscore());
  }
  if (other.has_underscore_height()) {
    set_underscore_height(other.get_underscore_height());
  }
  if (other.has_align()) {
    set_align(other.get_align());
  }
  if (other.has_indent()) {
    set_indent(other.get_indent());
  }
  if (other.has_wordwrap()) {
    set_wordwrap(other.get_wordwrap());
  }
  if (other.has_text_color()) {
    set_text_color(other.get_text_color());
  }
  if (other.has_shadow_color()) {
    set_shadow_color(other.get_shadow_color());
  }
  if (other.has_shadow()) {
    set_shadow(other.get_shadow());
  }
  if (other.has_bin()) {
    set_bin(other.get_bin());
  }
  if (other.has_draw_order()) {
    set_draw_order(other.get_draw_order());
  }
  if (other.has_tab_width()) {
    set_tab_width(other.get_tab_width());
  }

  // The glyph scale and shift are a special case: rather than replacing the
  // previous value, they modify it, so that they apply cumulatively to nested
  // TextProperties.
  if (other.has_glyph_shift()) {
    set_glyph_shift(other.get_glyph_shift() * get_glyph_scale() + get_glyph_shift());
  }
  if (other.has_glyph_scale()) {
    set_glyph_scale(other.get_glyph_scale() * get_glyph_scale());
  }

  if (other.has_text_scale()) {
    set_text_scale(other.get_text_scale());
  }
  if (other.has_direction()) {
    set_direction(other.get_direction());
  }
}


/**
 *
 */
void TextProperties::
write(std::ostream &out, int indent_level) const {
  if (!is_any_specified()) {
    indent(out, indent_level)
      << "default properties\n";
  }
  if (has_font()) {
    if (get_font() != nullptr) {
      indent(out, indent_level)
        << "with font " << _font->get_name() << "\n";
    } else {
      indent(out, indent_level)
        << "with NULL font\n";
    }
  }
  if (has_small_caps()) {
    indent(out, indent_level)
      << "small caps = " << get_small_caps() << "\n";
  }
  if (has_small_caps_scale()) {
    indent(out, indent_level)
      << "small caps scale = " << get_small_caps_scale() << "\n";
  }
  if (has_slant()) {
    indent(out, indent_level)
      << "slant = " << get_slant() << "\n";
  }
  if (has_underscore()) {
    indent(out, indent_level)
      << "underscore = " << get_underscore() << "\n";
  }
  if (has_underscore_height()) {
    indent(out, indent_level)
      << "underscore_height = " << get_underscore_height() << "\n";
  }

  if (has_align()) {
    indent(out, indent_level)
      << "alignment is ";
    switch (get_align()) {
    case A_left:
      out << "A_left\n";
      break;

    case A_right:
      out << "A_right\n";
      break;

    case A_center:
      out << "A_center\n";
      break;

    case A_boxed_left:
      out << "A_boxed_left\n";
      break;

    case A_boxed_right:
      out << "A_boxed_right\n";
      break;

    case A_boxed_center:
      out << "A_boxed_center\n";
      break;
    }
  }

  if (has_indent()) {
    indent(out, indent_level)
      << "indent at " << get_indent() << " units.\n";
  }

  if (has_wordwrap()) {
    indent(out, indent_level)
      << "word-wrapping at " << get_wordwrap() << " units.\n";
  }

  if (has_text_color()) {
    indent(out, indent_level)
      << "text color is " << get_text_color() << "\n";
  }

  if (has_shadow()) {
    indent(out, indent_level)
      << "shadow at " << get_shadow() << "\n";
  }
  if (has_shadow_color()) {
    indent(out, indent_level)
      << "shadow color is " << get_shadow_color() << "\n";
  }

  if (has_bin()) {
    indent(out, indent_level)
      << "bin is " << get_bin() << "\n";
  }
  if (has_draw_order()) {
    indent(out, indent_level)
      << "draw order is " << get_draw_order() << "\n";
  }

  if (has_tab_width()) {
    indent(out, indent_level)
      << "tab width is " << get_tab_width() << "\n";
  }

  if (has_glyph_scale()) {
    indent(out, indent_level)
      << "glyph scale is " << get_glyph_scale() << "\n";
  }
  if (has_glyph_shift()) {
    indent(out, indent_level)
      << "glyph shift is " << get_glyph_shift() << "\n";
  }

  if (has_text_scale()) {
    indent(out, indent_level)
      << "text scale is " << get_text_scale() << "\n";
  }

  if (has_direction()) {
    indent(out, indent_level)
      << "direction is ";
    switch (get_direction()) {
    case D_ltr:
      out << "D_ltr\n";
      break;

    case D_rtl:
      out << "D_rtl\n";
      break;
    }
  }
}

/**
 * Returns a RenderState object suitable for rendering text with these
 * properties.
 */
const RenderState *TextProperties::
get_text_state() const {
  if (!_text_state.is_null()) {
    return _text_state;
  }

  CPT(RenderState) state = RenderState::make_empty();

  if (has_text_color()) {
    state = state->add_attrib(ColorAttrib::make_flat(get_text_color()));
    if (get_text_color()[3] != 1.0) {
      state = state->add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha));
    }
  }

  if (has_bin()) {
    state = state->add_attrib(CullBinAttrib::make(get_bin(), get_draw_order() + 2));
  }

  std::swap(_text_state, state);
  return _text_state;
}

/**
 * Returns a RenderState object suitable for rendering the shadow of this text
 * with these properties.
 */
const RenderState *TextProperties::
get_shadow_state() const {
  if (!_shadow_state.is_null()) {
    return _shadow_state;
  }

  CPT(RenderState) state = RenderState::make_empty();

  state = state->add_attrib(ColorAttrib::make_flat(get_shadow_color()));
  if (get_shadow_color()[3] != 1.0) {
    state = state->add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha));
  }

  if (has_bin()) {
    state = state->add_attrib(CullBinAttrib::make(get_bin(), get_draw_order() + 1));
  }

  std::swap(_shadow_state, state);
  return _shadow_state;
}

/**
 * This function is called once (or never), the first time someone attempts to
 * render a TextNode using the default font.  It should attempt to load the
 * default font, using the compiled-in version if it is available, or whatever
 * system file may be named in Configrc.
 */
void TextProperties::
load_default_font() {
  _loaded_default_font = true;

  if (!text_default_font.empty()) {
    // First, attempt to load the user-specified filename.
    _default_font = FontPool::load_font(text_default_font.get_value());
    if (_default_font != nullptr && _default_font->is_valid()) {
      return;
    }
  }

  // Then, attempt to load the compiled-in font, if we have one.
#ifdef COMPILE_IN_DEFAULT_FONT
#ifdef HAVE_FREETYPE
  // Loading the compiled-in FreeType font is relatively easy.
  _default_font = new DynamicTextFont((const char *)default_font_data,
                                      default_font_size, 0);
  // The compiled-in font seems to confuse FreeType about its winding order.
  ((DynamicTextFont *)_default_font.p())->set_winding_order(DynamicTextFont::WO_left);

#else
  // The compiled-in Bam font requires creating a BamFile object to decode it.
  std::string data((const char *)default_font_data, default_font_size);

#ifdef HAVE_ZLIB
  // The font data is stored compressed; decompress it on-the-fly.
  std::istringstream inz(data);
  IDecompressStream in(&inz, false);

#else
  // The font data is stored uncompressed, so just load it.
  std::istringstream in(data);
#endif  // HAVE_ZLIB

  BamFile bam_file;
  if (bam_file.open_read(in, "default font stream")) {
    PT(PandaNode) node = bam_file.read_node();
    if (node != nullptr) {
      _default_font = new StaticTextFont(node);
    }
  }

#endif  // HAVE_FREETYPE
#endif  // COMPILE_IN_DEFAULT_FONT
}
