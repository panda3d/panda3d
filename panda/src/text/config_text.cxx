// Filename: config_text.cxx
// Created by:  drose (02Mar00)
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

#include "config_text.h"
#include "staticTextFont.h"
#include "textFont.h"
#include "textNode.h"
#include "dynamicTextFont.h"
#include "dynamicTextPage.h"
#include "geomTextGlyph.h"
#include "unicodeLatinMap.h"

#include "dconfig.h"
#include "config_express.h"

Configure(config_text);
NotifyCategoryDef(text, "");

ConfigureFn(config_text) {
  init_libtext();
}

const bool text_flatten = config_text.GetBool("text-flatten", true);
const bool text_update_cleared_glyphs = config_text.GetBool("text-update-cleared-glyphs", false);
const int text_anisotropic_degree = config_text.GetInt("text-anisotropic-degree", 1);
const int text_texture_margin = config_text.GetInt("text-texture-margin", 2);
const float text_poly_margin = config_text.GetFloat("text-poly-margin", 0.0f);
const int text_page_x_size = config_text.GetInt("text-page-x-size", 256);
const int text_page_y_size = config_text.GetInt("text-page-y-size", 256);
const bool text_small_caps = config_text.GetBool("text-small-caps", false);
const float text_small_caps_scale = config_text.GetFloat("text-small-caps-scale", 0.8f);
const string text_default_font = config_text.GetString("text-default-font", "");
const float text_tab_width = config_text.GetFloat("text-tab-width", 5.0f);


// This is the decimal character number that, embedded in a string, is
// identified as the soft-hyphen character.
const int text_soft_hyphen_key = config_text.GetInt("text-soft-hyphen-key", 3);

// This is the string that is output, encoded in the default encoding,
// to represent the soft-hyphen character.
wstring *text_soft_hyphen_output;

// If the rightmost whitespace character falls before this fraction of
// the line, hyphenate a word to the right of that if possible.
const float text_hyphen_ratio = config_text.GetFloat("text-hyphen-ratio", 0.7);

Texture::FilterType text_minfilter = Texture::FT_invalid;
Texture::FilterType text_magfilter = Texture::FT_invalid;


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

#ifdef HAVE_FREETYPE
  DynamicTextFont::init_type();
  DynamicTextPage::init_type();
  GeomTextGlyph::init_type();
  GeomTextGlyph::register_with_read_factory();
#endif

  // FT_linear_mipmap_nearest (that is, choose the nearest mipmap
  // level and bilinear filter the pixels from there) gives us some
  // mipmapping to avoid dropping pixels, but avoids the hideous
  // artifacts that we get from some cards (notably TNT2) from
  // filtering between two different mipmap levels.

  // But, full mipmapping still gives smoother blending from small to
  // large, so maybe we'll use it as the default anyway.
  string text_minfilter_str = config_text.GetString("text-minfilter", "linear_mipmap_linear");
  string text_magfilter_str = config_text.GetString("text-magfilter", "linear");

  text_minfilter = Texture::string_filter_type(text_minfilter_str);
  if (text_minfilter == Texture::FT_invalid) {
    text_cat.error()
      << "Invalid text-minfilter: " << text_minfilter_str << "\n";
    text_minfilter = Texture::FT_linear;
  }

  text_magfilter = Texture::string_filter_type(text_magfilter_str);
  if (text_magfilter == Texture::FT_invalid) {
    text_cat.error()
      << "Invalid text-magfilter: " << text_magfilter_str << "\n";
    text_magfilter = Texture::FT_linear;
  }

  // Make sure libexpress is initialized before we ask something of
  // TextEncoder.
  init_libexpress();
  string encoded = config_text.GetString("text-soft-hyphen-output", "-");
  TextEncoder encoder;
  text_soft_hyphen_output = new wstring(encoder.decode_text(encoded));
}
