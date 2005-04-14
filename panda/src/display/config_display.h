// Filename: config_display.h
// Created by:  drose (06Oct99)
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

#ifndef CONFIG_DISPLAY_H
#define CONFIG_DISPLAY_H

#include "pandabase.h"
#include "windowProperties.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableString.h"
#include "configVariableList.h"
#include "configVariableInt.h"
#include "configVariableEnum.h"
#include "configVariableFilename.h"
#include "dconfig.h"

#include "pvector.h"

ConfigureDecl(config_display, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(display, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDecl(gsg, EXPCL_PANDA, EXPTP_PANDA);

extern EXPCL_PANDA ConfigVariableBool view_frustum_cull;
extern EXPCL_PANDA ConfigVariableBool pstats_unused_states;

extern EXPCL_PANDA ConfigVariableString threading_model;
extern EXPCL_PANDA ConfigVariableBool auto_flip;
extern EXPCL_PANDA ConfigVariableBool yield_timeslice;

extern EXPCL_PANDA ConfigVariableString screenshot_filename;
extern EXPCL_PANDA ConfigVariableString screenshot_extension;

extern EXPCL_PANDA ConfigVariableBool show_buffers;

extern EXPCL_PANDA ConfigVariableBool prefer_texture_buffer;
extern EXPCL_PANDA ConfigVariableBool prefer_parasite_buffer;
extern EXPCL_PANDA ConfigVariableBool prefer_single_buffer;

extern EXPCL_PANDA ConfigVariableBool support_render_texture;
extern EXPCL_PANDA ConfigVariableBool copy_texture_inverted;
extern EXPCL_PANDA ConfigVariableBool window_inverted;
extern EXPCL_PANDA ConfigVariableBool depth_offset_decals;
extern EXPCL_PANDA ConfigVariableBool auto_generate_mipmaps;

extern EXPCL_PANDA ConfigVariableInt win_size;
extern EXPCL_PANDA ConfigVariableInt win_origin;
extern EXPCL_PANDA ConfigVariableInt win_width;
extern EXPCL_PANDA ConfigVariableInt win_height;
extern EXPCL_PANDA ConfigVariableInt win_origin_x;
extern EXPCL_PANDA ConfigVariableInt win_origin_y;
extern EXPCL_PANDA ConfigVariableBool fullscreen;
extern EXPCL_PANDA ConfigVariableBool undecorated;
extern EXPCL_PANDA ConfigVariableBool cursor_hidden;
extern EXPCL_PANDA ConfigVariableFilename icon_filename;
extern EXPCL_PANDA ConfigVariableFilename cursor_filename;
extern EXPCL_PANDA ConfigVariableEnum<WindowProperties::ZOrder> z_order;
extern EXPCL_PANDA ConfigVariableString window_title;

extern EXPCL_PANDA ConfigVariableString framebuffer_mode;
extern EXPCL_PANDA ConfigVariableInt depth_bits;
extern EXPCL_PANDA ConfigVariableInt color_bits;
extern EXPCL_PANDA ConfigVariableInt alpha_bits;
extern EXPCL_PANDA ConfigVariableInt stencil_bits;
extern EXPCL_PANDA ConfigVariableInt multisamples;

extern EXPCL_PANDA ConfigVariableDouble background_color;
extern EXPCL_PANDA ConfigVariableBool sync_video;


extern EXPCL_PANDA void init_libdisplay();

#endif /* CONFIG_DISPLAY_H */
