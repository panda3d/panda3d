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

extern ConfigVariableBool view_frustum_cull;
extern ConfigVariableBool pstats_unused_states;

extern ConfigVariableString threading_model;
extern ConfigVariableBool auto_flip;
extern ConfigVariableBool yield_timeslice;

extern ConfigVariableString screenshot_filename;
extern ConfigVariableString screenshot_extension;

extern ConfigVariableBool show_buffers;

extern ConfigVariableBool prefer_parasite_buffer;
extern ConfigVariableBool prefer_single_buffer;

extern ConfigVariableBool copy_texture_inverted;
extern ConfigVariableBool window_inverted;
extern ConfigVariableBool depth_offset_decals;

extern ConfigVariableInt win_size;
extern ConfigVariableInt win_origin;
extern ConfigVariableInt win_width;
extern ConfigVariableInt win_height;
extern ConfigVariableInt win_origin_x;
extern ConfigVariableInt win_origin_y;
extern ConfigVariableBool fullscreen;
extern ConfigVariableBool undecorated;
extern ConfigVariableBool cursor_hidden;
extern ConfigVariableFilename icon_filename;
extern ConfigVariableFilename cursor_filename;
extern ConfigVariableEnum<WindowProperties::ZOrder> z_order;
extern ConfigVariableString window_title;

extern ConfigVariableString framebuffer_mode;
extern ConfigVariableInt depth_bits;
extern ConfigVariableInt color_bits;
extern ConfigVariableInt alpha_bits;
extern ConfigVariableInt stencil_bits;
extern ConfigVariableInt multisamples;

extern ConfigVariableDouble background_color;


extern EXPCL_PANDA void init_libdisplay();

#endif /* CONFIG_DISPLAY_H */
