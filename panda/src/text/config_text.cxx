// Filename: config_text.cxx
// Created by:  drose (02Mar00)
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

#include "config_text.h"
#include "staticTextFont.h"
#include "textFont.h"
#include "textNode.h"
#include "textProperties.h"
#include "dynamicTextFont.h"
#include "dynamicTextPage.h"
#include "geomTextGlyph.h"
#include "unicodeLatinMap.h"
#include "pandaSystem.h"

#include "dconfig.h"
#include "config_express.h"

Configure(config_text);
NotifyCategoryDef(text, "");

ConfigureFn(config_text) {
  init_libtext();
}

ConfigVariableBool text_flatten
("text-flatten", true);

ConfigVariableBool text_update_cleared_glyphs
("text-update-cleared-glyphs", false);

ConfigVariableInt text_anisotropic_degree
("text-anisotropic-degree", 1);

ConfigVariableInt text_texture_margin
("text-texture-margin", 2);

ConfigVariableDouble text_poly_margin
("text-poly-margin", 0.0f);

ConfigVariableInt text_page_x_size
("text-page-x-size", 256);

ConfigVariableInt text_page_y_size
("text-page-y-size", 256);

ConfigVariableBool text_small_caps
("text-small-caps", false);

ConfigVariableDouble text_small_caps_scale
("text-small-caps-scale", 0.8f);

ConfigVariableFilename text_default_font
("text-default-font", "");

ConfigVariableDouble text_tab_width
("text-tab-width", 5.0f);


// This is the decimal character number that, embedded in a string, is
// used to bracket the name of a TextProperties structure added to the
// TextPropertiesManager object, to control the appearance of
// subsequent text.
ConfigVariableInt text_push_properties_key
("text-push-properties-key", 1);

// This is the decimal character number that undoes the effect of a
// previous appearance of text_push_properties_key.
ConfigVariableInt text_pop_properties_key
("text-pop-properties-key", 2);

// This is the decimal character number that, embedded in a string, is
// identified as the soft-hyphen character.
ConfigVariableInt text_soft_hyphen_key
("text-soft-hyphen-key", 3);

// This is similar to the soft-hyphen key, above, except that when it
// is used as a break point, no character is introduced in its place.
ConfigVariableInt text_soft_break_key
("text-soft-break-key", 4);

// This is the string that is output, encoded in the default encoding,
// to represent the hyphen character that is introduced when the line
// is broken at a soft-hyphen key.
wstring
get_text_soft_hyphen_output() {
  static wstring *text_soft_hyphen_output = NULL;
  static ConfigVariableString cv("text-soft-hyphen-output", "-");

  if (text_soft_hyphen_output == NULL) {
    TextEncoder encoder;
    text_soft_hyphen_output = new wstring(encoder.decode_text(cv));
  }

  return *text_soft_hyphen_output;
}

// If the rightmost whitespace character falls before this fraction of
// the line, hyphenate a word to the right of that if possible.
ConfigVariableDouble text_hyphen_ratio
("text-hyphen-ratio", 0.7);

// This string represents a list of individual characters that should
// never appear at the beginning of a line following a forced break.
// Typically these will be punctuation characters.
wstring
get_text_never_break_before() {
  static wstring *text_never_break_before = NULL;
  static ConfigVariableString cv("text-never-break-before", ",.-:?!;");

  if (text_never_break_before == NULL) {
    TextEncoder encoder;
    text_never_break_before = new wstring(encoder.decode_text(cv));
  }

  return *text_never_break_before;
}

// Unless we have more than this number of text_never_break_before
// characters in a row, in which case forget it and break wherever we
// can.
ConfigVariableInt text_max_never_break
("text-max-never-break", 3);

ConfigVariableEnum<Texture::FilterType> text_minfilter
("text-minfilter", Texture::FT_linear_mipmap_linear);
ConfigVariableEnum<Texture::FilterType> text_magfilter
("text-magfilter", Texture::FT_linear);


////////////////////////////////////////////////////////////////////
//     Function: init_libtext
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libtext() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  StaticTextFont::init_type();
  TextFont::init_type();
  TextNode::init_type();
  TextProperties::init_type();

#ifdef HAVE_FREETYPE
  DynamicTextFont::init_type();
  DynamicTextPage::init_type();
  GeomTextGlyph::init_type();
  GeomTextGlyph::register_with_read_factory();

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("Freetype");
#endif
}
