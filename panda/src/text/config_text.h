// Filename: config_text.h
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

#ifndef CONFIG_TEXT_H
#define CONFIG_TEXT_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "texture.h"
#include "configVariableBool.h"
#include "configVariableInt.h"
#include "configVariableDouble.h"
#include "configVariableFilename.h"
#include "configVariableEnum.h"

class DSearchPath;

NotifyCategoryDecl(text, EXPCL_PANDA, EXPTP_PANDA);

extern ConfigVariableBool text_flatten;
extern ConfigVariableBool text_update_cleared_glyphs;
extern ConfigVariableInt text_anisotropic_degree;
extern ConfigVariableInt text_texture_margin;
extern ConfigVariableDouble text_poly_margin;
extern ConfigVariableInt text_page_x_size;
extern ConfigVariableInt text_page_y_size;
extern ConfigVariableBool text_small_caps;
extern ConfigVariableDouble text_small_caps_scale;
extern ConfigVariableFilename text_default_font;
extern ConfigVariableDouble text_tab_width;
extern ConfigVariableInt text_push_properties_key;
extern ConfigVariableInt text_pop_properties_key;
extern ConfigVariableInt text_soft_hyphen_key;
extern ConfigVariableInt text_soft_break_key;
extern wstring get_text_soft_hyphen_output();
extern ConfigVariableDouble text_hyphen_ratio;
extern wstring get_text_never_break_before();
extern ConfigVariableInt text_max_never_break;

extern ConfigVariableEnum<Texture::FilterType> text_minfilter;
extern ConfigVariableEnum<Texture::FilterType> text_magfilter;

extern EXPCL_PANDA void init_libtext();

#endif
