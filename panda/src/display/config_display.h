/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_display.h
 * @author drose
 * @date 1999-10-06
 */

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
#include "configVariableColor.h"
#include "coordinateSystem.h"
#include "dconfig.h"

#include "pvector.h"

ConfigureDecl(config_display, EXPCL_PANDA_DISPLAY, EXPTP_PANDA_DISPLAY);
NotifyCategoryDecl(display, EXPCL_PANDA_DISPLAY, EXPTP_PANDA_DISPLAY);
NotifyCategoryDecl(gsg, EXPCL_PANDA_DISPLAY, EXPTP_PANDA_DISPLAY);

extern EXPCL_PANDA_DISPLAY ConfigVariableBool view_frustum_cull;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool pstats_unused_states;

extern EXPCL_PANDA_DISPLAY ConfigVariableString threading_model;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool allow_nonpipeline_threads;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool auto_flip;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool sync_flip;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool yield_timeslice;
extern EXPCL_PANDA_DISPLAY ConfigVariableDouble subprocess_window_max_wait;

extern EXPCL_PANDA_DISPLAY ConfigVariableString screenshot_filename;
extern EXPCL_PANDA_DISPLAY ConfigVariableString screenshot_extension;

extern EXPCL_PANDA_DISPLAY ConfigVariableBool prefer_texture_buffer;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool prefer_parasite_buffer;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool force_parasite_buffer;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool prefer_single_buffer;

extern EXPCL_PANDA_DISPLAY ConfigVariableInt max_texture_stages;
extern EXPCL_PANDA_DISPLAY ConfigVariableInt max_color_targets;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool support_render_texture;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool support_rescale_normal;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool support_stencil;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool copy_texture_inverted;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool window_inverted;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool red_blue_stereo;
extern EXPCL_PANDA_DISPLAY ConfigVariableString red_blue_stereo_colors;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool side_by_side_stereo;
extern EXPCL_PANDA_DISPLAY ConfigVariableDouble sbs_left_dimensions;
extern EXPCL_PANDA_DISPLAY ConfigVariableDouble sbs_right_dimensions;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool swap_eyes;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool default_stereo_camera;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool color_scale_via_lighting;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool alpha_scale_via_texture;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool allow_incomplete_render;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool old_alpha_blend;

extern EXPCL_PANDA_DISPLAY ConfigVariableInt win_size;
extern EXPCL_PANDA_DISPLAY ConfigVariableInt win_origin;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool fullscreen;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool undecorated;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool win_fixed_size;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool cursor_hidden;
extern EXPCL_PANDA_DISPLAY ConfigVariableFilename icon_filename;
extern EXPCL_PANDA_DISPLAY ConfigVariableFilename cursor_filename;
extern EXPCL_PANDA_DISPLAY ConfigVariableEnum<WindowProperties::ZOrder> z_order;
extern EXPCL_PANDA_DISPLAY ConfigVariableString window_title;
extern EXPCL_PANDA_DISPLAY ConfigVariableInt parent_window_handle;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool win_unexposed_draw;
extern EXPCL_PANDA_DISPLAY ConfigVariableFilename subprocess_window;

extern EXPCL_PANDA_DISPLAY ConfigVariableString framebuffer_mode;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool framebuffer_hardware;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool framebuffer_software;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool framebuffer_multisample;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool framebuffer_depth;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool framebuffer_alpha;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool framebuffer_stencil;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool framebuffer_accum;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool framebuffer_stereo;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool framebuffer_srgb;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool framebuffer_float;
extern EXPCL_PANDA_DISPLAY ConfigVariableInt depth_bits;
extern EXPCL_PANDA_DISPLAY ConfigVariableInt color_bits;
extern EXPCL_PANDA_DISPLAY ConfigVariableInt alpha_bits;
extern EXPCL_PANDA_DISPLAY ConfigVariableInt stencil_bits;
extern EXPCL_PANDA_DISPLAY ConfigVariableInt accum_bits;
extern EXPCL_PANDA_DISPLAY ConfigVariableInt multisamples;
extern EXPCL_PANDA_DISPLAY ConfigVariableInt back_buffers;
extern EXPCL_PANDA_DISPLAY ConfigVariableInt shadow_depth_bits;

extern EXPCL_PANDA_DISPLAY ConfigVariableDouble pixel_zoom;

extern EXPCL_PANDA_DISPLAY ConfigVariableColor background_color;
extern EXPCL_PANDA_DISPLAY ConfigVariableBool sync_video;

extern EXPCL_PANDA_DISPLAY void init_libdisplay();

#endif /* CONFIG_DISPLAY_H */
