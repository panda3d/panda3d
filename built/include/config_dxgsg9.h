/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_dxgsg9.h
 * @author drose
 * @date 1999-10-06
 */

#ifndef CONFIG_DXGSG9_H
#define CONFIG_DXGSG9_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dxgsg9base.h"
#include "configVariableBool.h"
#include "configVariableInt.h"
#include "configVariableDouble.h"

NotifyCategoryDecl(dxgsg9, EXPCL_PANDADX, EXPTP_PANDADX);
NotifyCategoryDecl(wdxdisplay9, EXPCL_PANDADX, EXPTP_PANDADX);

extern ConfigVariableBool dx_no_vertex_fog;
extern ConfigVariableBool dx_show_cursor_watermark;
extern ConfigVariableBool dx_full_screen_antialiasing;
extern ConfigVariableBool dx_use_rangebased_fog;
extern ConfigVariableBool link_tristrips;
extern ConfigVariableBool dx_use_triangle_mipgen_filter;
extern ConfigVariableBool dx_broken_max_index;
extern ConfigVariableBool dx_broken_depth_bias;
extern ConfigVariableDouble dx_depth_bias_scale;
extern ConfigVariableBool dx_count_all_cards_memory;

// debug flags we might want to use in full optimized build
extern ConfigVariableBool dx_ignore_mipmaps;
extern ConfigVariableBool dx_mipmap_everything;
extern ConfigVariableBool dx_show_transforms;
extern ConfigVariableBool dx_no_dithering;
extern ConfigVariableBool dx_force_anisotropic_filtering;
extern ConfigVariableBool dx_force_16bpp_zbuffer;
extern ConfigVariableBool dx_do_vidmemsize_check;
extern ConfigVariableBool dx_preserve_fpu_state;
extern ConfigVariableInt dx_preferred_device_id;

extern ConfigVariableBool dx_intel_compressed_texture_bug;

#ifndef NDEBUG
extern ConfigVariableInt dx_force_backface_culling;
#endif

#ifdef _DEBUG
extern ConfigVariableDouble dx_global_miplevel_bias;
extern ConfigVariableBool dx_debug_view_mipmaps;
#endif

// LRU configuration variables
extern ConfigVariableBool dx_management;
extern ConfigVariableBool dx_texture_management;
extern ConfigVariableBool dx_lru_management;
extern ConfigVariableInt dx_lru_maximum_pages;
extern ConfigVariableInt dx_lru_free_memory_requirement;
extern ConfigVariableInt dx_lru_minimum_memory_requirement;
extern ConfigVariableInt dx_lru_maximum_memory_requirement;
extern ConfigVariableInt dx_lru_maximum_page_updates_per_frame;

// LRU debug variables
extern ConfigVariableBool dx_lru_debug;
extern ConfigVariableInt dx_lru_debug_frames_til_output;
extern ConfigVariableBool dx_lru_debug_textures;
extern ConfigVariableBool dx_lru_debug_vertex_buffers;

extern ConfigVariableBool dx_use_dynamic_textures;

// DX device options
extern ConfigVariableBool dx_use_multithread;
extern ConfigVariableBool dx_use_puredevice;
extern ConfigVariableBool dx_disable_driver_management;
extern ConfigVariableBool dx_disable_driver_management_ex;

// nVidia's performace heads up display
extern ConfigVariableBool dx_use_nvperfhud;

extern EXPCL_PANDADX void init_libdxgsg9();

#endif
