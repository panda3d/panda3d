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

Configure(config_text);
NotifyCategoryDef(text, "");

ConfigureFn(config_text) {
  init_libtext();
}

const bool text_flatten = config_text.GetBool("text-flatten", true);
const bool text_update_cleared_glyphs = config_text.GetBool("text-update-cleared-glyphs", false);
const int text_anisotropic_degree = config_text.GetInt("text-anisotropic-degree", 1);
const int text_texture_margin = config_text.GetInt("text-texture-margin", 2);
const float text_poly_margin = config_text.GetFloat("text-poly-margin", 0.6f);
const int text_page_x_size = config_text.GetInt("text-page-x-size", 256);
const int text_page_y_size = config_text.GetInt("text-page-y-size", 256);
const float text_point_size = config_text.GetFloat("text-point-size", 10.0f);
const float text_pixels_per_unit = config_text.GetFloat("text-pixels-per-unit", 30.0f);
const float text_scale_factor = config_text.GetFloat("text-scale-factor", 2.0f);
const bool text_small_caps = config_text.GetBool("text-small-caps", false);
const float text_small_caps_scale = config_text.GetFloat("text-small-caps-scale", 0.8f);
const string text_default_font = config_text.GetString("text-default-font", "");

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

  string text_encoding = config_text.GetString("text-encoding", "iso8859");
  if (text_encoding == "iso8859") {
    TextNode::set_default_encoding(TextNode::E_iso8859);
  } else if (text_encoding == "utf8") {
    TextNode::set_default_encoding(TextNode::E_utf8);
  } else if (text_encoding == "unicode") {
    TextNode::set_default_encoding(TextNode::E_unicode);
  } else {
    text_cat.error()
      << "Invalid text-encoding: " << text_encoding << "\n";
  }

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
  
}
