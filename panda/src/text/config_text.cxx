/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_text.cxx
 * @author drose
 * @date 2000-03-02
 */

#include "config_text.h"
#include "staticTextFont.h"
#include "textFont.h"
#include "textGlyph.h"
#include "textNode.h"
#include "textProperties.h"
#include "dynamicTextFont.h"
#include "dynamicTextGlyph.h"
#include "dynamicTextPage.h"
#include "geomTextGlyph.h"
#include "unicodeLatinMap.h"
#include "pandaSystem.h"

#include "dconfig.h"
#include "config_express.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_TEXT)
  #error Buildsystem error: BUILDING_PANDA_TEXT not defined
#endif

using std::wstring;

Configure(config_text);
NotifyCategoryDef(text, "");

ConfigureFn(config_text) {
  init_libtext();
}

ConfigVariableBool text_flatten
("text-flatten", true,
 PRC_DESC("Set this true to flatten text when it is generated, or false to "
          "keep it as a deep hierarchy.  Usually it's a performance "
          "advantage to keep this true, but this also depends on the setting "
          "of text-dynamic-merge.  See TextNode::set_flatten_flags()."));

ConfigVariableBool text_dynamic_merge
("text-dynamic-merge", true,
 PRC_DESC("Set this true to merge generated glyphs into the GeomVertexData "
          "as the text is assembled, or false to wait for the flatten "
          "operation.  Usually it's a performance "
          "advantage to keep this true.  See TextNode::set_flatten_flags()."));

ConfigVariableBool text_kerning
("text-kerning", false,
 PRC_DESC("Set this true to enable kerning when the font provides kerning "
          "tables.  This can result in more aesthetically pleasing spacing "
          "between individual glyphs.  Has no effect when text-use-harfbuzz "
          "is true, since HarfBuzz offers superior kerning support."));

ConfigVariableBool text_use_harfbuzz
("text-use-harfbuzz", false,
 PRC_DESC("Set this true to enable HarfBuzz support, which offers superior "
          "text shaping and better support for non-Latin text."));

ConfigVariableInt text_anisotropic_degree
("text-anisotropic-degree", 1,
 PRC_DESC("This is the default anisotropic-degree that is set on dynamic "
          "font textures.  Setting this to a value greater than 1 can help "
          "smooth out the antialiasing for small letters."));

ConfigVariableInt text_texture_margin
("text-texture-margin", 2,
 PRC_DESC("This is the number of texels of empty space reserved around each "
          "glyph in the texture.  Setting this value larger will decrease "
          "the tendency for adjacent glyphs to bleed into each other at "
          "small sizes, but it will increase amount of wasted texture "
          "memory."));

ConfigVariableDouble text_poly_margin
("text-poly-margin", 0.0f,
 PRC_DESC("This is the amount by which to make each glyph polygon larger "
          "than strictly necessary, in screen units that are added to each "
          "margin.  Increasing this value will decrease the tendency for "
          "letters to get chopped off at the edges, but it will also "
          "increase the tendency for adjacent glyphs to bleed into each "
          "other (unless you also increase text-texture-margin)."));

ConfigVariableInt text_page_size
("text-page-size", "256 256",
 PRC_DESC("This is the default size for new textures created for dynamic "
          "fonts."));

ConfigVariableBool text_small_caps
("text-small-caps", false,
 PRC_DESC("This controls the default setting for "
          "TextNode::set_small_caps()."));

ConfigVariableDouble text_small_caps_scale
("text-small-caps-scale", 0.8f,
 PRC_DESC("This controls the default setting for "
          "TextNode::set_small_caps_scale()."));

ConfigVariableFilename text_default_font
("text-default-font", "",
 PRC_DESC("This names a filename that will be loaded at startup time as "
          "the default font for any TextNode that does not specify a font "
          "otherwise.  The default is to use a special font that is "
          "compiled into Panda, if available."));

ConfigVariableDouble text_tab_width
("text-tab-width", 5.0f,
 PRC_DESC("This controls the default setting for "
          "TextNode::set_tab_width()."));

ConfigVariableInt text_push_properties_key
("text-push-properties-key", 1,
 PRC_DESC("This is the decimal character number that, embedded in "
          "a string, is used to bracket the name of a TextProperties "
          "structure added to the TextPropertiesManager object, to "
          "control the appearance of subsequent text."));

ConfigVariableInt text_pop_properties_key
("text-pop-properties-key", 2,
 PRC_DESC("This is the decimal character number that undoes the "
          "effect of a previous appearance of text_push_properties_key."));

ConfigVariableInt text_soft_hyphen_key
("text-soft-hyphen-key", 3,
 PRC_DESC("This is the decimal character number that, embedded in a "
          "string, is identified as the soft-hyphen character."));

ConfigVariableInt text_soft_break_key
("text-soft-break-key", 4,
 PRC_DESC("This is similar to text-soft-hyphen-key, except that "
          "when it is used as a break point, no character is "
          "introduced in its place."));

ConfigVariableInt text_embed_graphic_key
("text-embed-graphic-key", 5,
 PRC_DESC("This is the decimal character number that, embedded in "
          "a string, is used to bracket the name of a model "
          "added to the TextPropertiesManager object, to "
          "embed an arbitrary graphic image within a paragraph."));

wstring
get_text_soft_hyphen_output() {
  static wstring *text_soft_hyphen_output = nullptr;
  static ConfigVariableString
    cv("text-soft-hyphen-output", "-",
       PRC_DESC("This is the string that is output, encoded in the default "
                "encoding, to represent the hyphen character that is "
                "introduced when the line is broken at a soft-hyphen key."));

  if (text_soft_hyphen_output == nullptr) {
    TextEncoder encoder;
    text_soft_hyphen_output = new wstring(encoder.decode_text(cv));
  }

  return *text_soft_hyphen_output;
}

ConfigVariableDouble text_hyphen_ratio
("text-hyphen-ratio", 0.7,
 PRC_DESC("If the rightmost whitespace character falls before this "
          "fraction of the line, hyphenate a word to the right of that "
          "if possible."));

wstring
get_text_never_break_before() {
  static wstring *text_never_break_before = nullptr;
  static ConfigVariableString
    cv("text-never-break-before", ",.-:?!;",
       PRC_DESC("This string represents a list of individual characters "
                "that should never appear at the beginning of a line "
                "following a forced break.  Typically these will be "
                "punctuation characters."));

  if (text_never_break_before == nullptr) {
    TextEncoder encoder;
    text_never_break_before = new wstring(encoder.decode_text(cv));
  }

  return *text_never_break_before;
}

ConfigVariableInt text_max_never_break
("text-max-never-break", 3,
 PRC_DESC("If we have more than this number of text-never-break-before "
          "characters in a row, do not treat any of them as special and "
          "instead break the line wherever we can."));

ConfigVariableDouble text_default_underscore_height
("text-default-underscore-height", -0.2,
 PRC_DESC("Specifies the default height of the underscore line, relative "
          "to the text baseline, when underscoring is enabled."));

ConfigVariableEnum<SamplerState::FilterType> text_minfilter
("text-minfilter", SamplerState::FT_linear,
 PRC_DESC("The default texture minfilter type for dynamic text fonts"));
ConfigVariableEnum<SamplerState::FilterType> text_magfilter
("text-magfilter", SamplerState::FT_linear,
 PRC_DESC("The default texture magfilter type for dynamic text fonts"));
ConfigVariableEnum<SamplerState::WrapMode> text_wrap_mode
("text-wrap-mode", SamplerState::WM_border_color,
 PRC_DESC("The default wrap mode for dynamic text fonts"));
ConfigVariableEnum<Texture::QualityLevel> text_quality_level
("text-quality-level", Texture::QL_best,
 PRC_DESC("The default quality level for dynamic text fonts; see Texture::set_quality_level()."));

ConfigVariableEnum<TextFont::RenderMode> text_render_mode
("text-render-mode", TextFont::RM_texture,
 PRC_DESC("The default render mode for dynamic text fonts"));



/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libtext() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  GeomTextGlyph::init_type();
  StaticTextFont::init_type();
  TextFont::init_type();
  TextGlyph::init_type();
  TextNode::init_type();
  TextProperties::init_type();

  GeomTextGlyph::register_with_read_factory();

#ifdef HAVE_FREETYPE
  DynamicTextFont::init_type();
  DynamicTextGlyph::init_type();
  DynamicTextPage::init_type();

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("Freetype");
#endif
}
