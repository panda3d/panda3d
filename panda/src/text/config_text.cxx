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

#include <dconfig.h>

Configure(config_text);
NotifyCategoryDef(text, "");

ConfigureFn(config_text) {
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
    TextNode::_default_encoding = TextNode::E_iso8859;
  } else if (text_encoding == "utf8") {
    TextNode::_default_encoding = TextNode::E_utf8;
  } else if (text_encoding == "unicode") {
    TextNode::_default_encoding = TextNode::E_unicode;
  } else {
    text_cat.error()
      << "Invalid text-encoding: " << text_encoding << "\n";
  }
}

const bool text_flatten = config_text.GetBool("text-flatten", true);
const bool text_update_cleared_glyphs = config_text.GetBool("text-update-cleared-glyphs", false);
const bool text_mipmap = config_text.GetBool("text-mipmap", true);
const int text_anisotropic_degree = config_text.GetInt("text-anisotropic-degree", 1);
const int text_texture_margin = config_text.GetInt("text-texture-margin", 2);
const float text_poly_margin = config_text.GetFloat("text-poly-margin", 1.0f);
const int text_page_x_size = config_text.GetInt("text-page-x-size", 256);
const int text_page_y_size = config_text.GetInt("text-page-y-size", 256);
const float text_point_size = config_text.GetFloat("text-point-size", 10.0f);
const float text_pixels_per_unit = config_text.GetFloat("text-pixels-per-unit", 30.0f);
const bool text_small_caps = config_text.GetBool("text-small-caps", false);
const float text_small_caps_scale = config_text.GetFloat("text-small-caps-scale", 0.8f);
