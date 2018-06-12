/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_text.h
 * @author drose
 * @date 2000-03-02
 */

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
#include "textFont.h"

class DSearchPath;

NotifyCategoryDecl(text, EXPCL_PANDA_TEXT, EXPTP_PANDA_TEXT);

extern ConfigVariableBool text_flatten;
extern ConfigVariableBool text_dynamic_merge;
extern ConfigVariableBool text_kerning;
extern ConfigVariableBool text_use_harfbuzz;
extern ConfigVariableInt text_anisotropic_degree;
extern ConfigVariableInt text_texture_margin;
extern ConfigVariableDouble text_poly_margin;
extern ConfigVariableInt text_page_size;
extern ConfigVariableBool text_small_caps;
extern EXPCL_PANDA_TEXT ConfigVariableDouble text_small_caps_scale;
extern ConfigVariableFilename text_default_font;
extern EXPCL_PANDA_TEXT ConfigVariableDouble text_tab_width;
extern EXPCL_PANDA_TEXT ConfigVariableInt text_push_properties_key;
extern EXPCL_PANDA_TEXT ConfigVariableInt text_pop_properties_key;
extern ConfigVariableInt text_soft_hyphen_key;
extern ConfigVariableInt text_soft_break_key;
extern ConfigVariableInt text_embed_graphic_key;
extern std::wstring get_text_soft_hyphen_output();
extern ConfigVariableDouble text_hyphen_ratio;
extern std::wstring get_text_never_break_before();
extern ConfigVariableInt text_max_never_break;
extern EXPCL_PANDA_TEXT ConfigVariableDouble text_default_underscore_height;

extern ConfigVariableEnum<SamplerState::FilterType> text_minfilter;
extern ConfigVariableEnum<SamplerState::FilterType> text_magfilter;
extern ConfigVariableEnum<SamplerState::WrapMode> text_wrap_mode;
extern ConfigVariableEnum<Texture::QualityLevel> text_quality_level;
extern ConfigVariableEnum<TextFont::RenderMode> text_render_mode;

extern EXPCL_PANDA_TEXT void init_libtext();

#endif
