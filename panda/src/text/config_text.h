// Filename: config_text.h
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

#ifndef CONFIG_TEXT_H
#define CONFIG_TEXT_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "texture.h"

class DSearchPath;

NotifyCategoryDecl(text, EXPCL_PANDA, EXPTP_PANDA);

extern const bool text_flatten;
extern const bool text_update_cleared_glyphs;
extern const int text_anisotropic_degree;
extern const int text_texture_margin;
extern const float text_poly_margin;
extern const int text_page_x_size;
extern const int text_page_y_size;
extern const float text_point_size;
extern const float text_pixels_per_unit;
extern const float text_scale_factor;
extern const bool text_small_caps;
extern const float text_small_caps_scale;
extern const string text_default_font;

extern Texture::FilterType text_minfilter;
extern Texture::FilterType text_magfilter;

extern EXPCL_PANDA void init_libtext();

#endif
