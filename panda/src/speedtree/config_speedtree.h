// Filename: config_speedtree.h
// Created by:  drose (30Sep10)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_SPEEDTREE_H
#define CONFIG_SPEEDTREE_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableDouble.h"
#include "configVariableString.h"
#include "configVariableInt.h"
#include "configVariableFilename.h"
#include "configVariableColor.h"

NotifyCategoryDecl(speedtree, EXPCL_PANDASPEEDTREE, EXPTP_PANDASPEEDTREE);

extern ConfigVariableString speedtree_license;
extern ConfigVariableFilename speedtree_shaders_dir;
extern ConfigVariableFilename speedtree_textures_dir;
extern ConfigVariableDouble speedtree_max_anisotropy;
extern ConfigVariableBool speedtree_horizontal_billboards;
extern ConfigVariableDouble speedtree_alpha_test_scalar;
extern ConfigVariableBool speedtree_z_pre_pass;
extern ConfigVariableInt speedtree_max_billboard_images_by_base;

extern ConfigVariableDouble speedtree_visibility;
extern ConfigVariableDouble speedtree_global_light_scalar;
extern ConfigVariableBool speedtree_specular_lighting;
extern ConfigVariableBool speedtree_transmission_lighting;
extern ConfigVariableBool speedtree_detail_layer;
extern ConfigVariableBool speedtree_detail_normal_mapping;
extern ConfigVariableBool speedtree_ambient_contrast;
extern ConfigVariableDouble speedtree_transmission_scalar;
extern ConfigVariableDouble speedtree_fog_distance;
extern ConfigVariableColor speedtree_fog_color;
extern ConfigVariableColor speedtree_sky_color;
extern ConfigVariableDouble speedtree_sky_fog_min;
extern ConfigVariableDouble speedtree_sky_fog_max;
extern ConfigVariableColor speedtree_sun_color;
extern ConfigVariableDouble speedtree_sun_size;
extern ConfigVariableDouble speedtree_sun_spread_exponent;
extern ConfigVariableDouble speedtree_sun_fog_bloom;
extern ConfigVariableColor speedtree_specular_color;
extern ConfigVariableColor speedtree_emissive_color;
extern ConfigVariableInt speedtree_shadow_map_resolution;
extern ConfigVariableDouble speedtree_cascading_shadow_splits;
extern ConfigVariableBool speedtree_smooth_shadows;
extern ConfigVariableBool speedtree_show_shadow_splits_on_terrain;
extern ConfigVariableBool speedtree_wind_enabled;
extern ConfigVariableBool speedtree_frond_rippling;
extern ConfigVariableInt speedtree_terrain_num_lods;
extern ConfigVariableInt speedtree_terrain_resolution;
extern ConfigVariableInt speedtree_terrain_cell_size;

extern ConfigVariableDouble speedtree_shadow_fade;
extern ConfigVariableBool speedtree_show_overlays;

extern ConfigVariableInt speedtree_max_num_visible_cells;
extern ConfigVariableDouble speedtree_cull_cell_size;
extern ConfigVariableDouble speedtree_area_scale;
extern ConfigVariableBool speedtree_follow_terrain;
extern ConfigVariableInt speedtree_max_random_try_count;
extern ConfigVariableBool speedtree_5_2_stf;

extern EXPCL_PANDASPEEDTREE void init_libspeedtree();

#endif


