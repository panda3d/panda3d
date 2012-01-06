// Filename: config_egg2pg.h
// Created by:  drose (26Feb02)
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

#ifndef CONFIG_EGG2PG_H
#define CONFIG_EGG2PG_H

#include "pandabase.h"

#include "coordinateSystem.h"
#include "eggRenderMode.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableDouble.h"
#include "configVariableEnum.h"
#include "configVariableInt.h"
#include "dconfig.h"

ConfigureDecl(config_egg2pg, EXPCL_PANDAEGG, EXPTP_PANDAEGG);
NotifyCategoryDecl(egg2pg, EXPCL_PANDAEGG, EXPTP_PANDAEGG);

extern EXPCL_PANDAEGG ConfigVariableDouble egg_normal_scale;
extern EXPCL_PANDAEGG ConfigVariableBool egg_show_normals;
extern EXPCL_PANDAEGG ConfigVariableEnum<CoordinateSystem> egg_coordinate_system;
extern EXPCL_PANDAEGG ConfigVariableBool egg_ignore_mipmaps;
extern EXPCL_PANDAEGG ConfigVariableBool egg_ignore_filters;
extern EXPCL_PANDAEGG ConfigVariableBool egg_ignore_clamp;
extern EXPCL_PANDAEGG ConfigVariableBool egg_ignore_decals;
extern EXPCL_PANDAEGG ConfigVariableBool egg_flatten;
extern EXPCL_PANDAEGG ConfigVariableDouble egg_flatten_radius;
extern EXPCL_PANDAEGG ConfigVariableBool egg_unify;
extern EXPCL_PANDAEGG ConfigVariableBool egg_combine_geoms;
extern EXPCL_PANDAEGG ConfigVariableBool egg_rigid_geometry;
extern EXPCL_PANDAEGG ConfigVariableBool egg_flat_shading;
extern EXPCL_PANDAEGG ConfigVariableBool egg_flat_colors;
extern EXPCL_PANDAEGG ConfigVariableBool egg_load_old_curves;
extern EXPCL_PANDAEGG ConfigVariableBool egg_load_classic_nurbs_curves;
extern EXPCL_PANDAEGG ConfigVariableBool egg_accept_errors;
extern EXPCL_PANDAEGG ConfigVariableBool egg_suppress_hidden;
extern EXPCL_PANDAEGG ConfigVariableEnum<EggRenderMode::AlphaMode> egg_alpha_mode;
extern EXPCL_PANDAEGG ConfigVariableInt egg_max_vertices;
extern EXPCL_PANDAEGG ConfigVariableInt egg_max_indices;
extern EXPCL_PANDAEGG ConfigVariableBool egg_emulate_bface;
extern EXPCL_PANDAEGG ConfigVariableBool egg_preload_simple_textures;
extern EXPCL_PANDAEGG ConfigVariableDouble egg_vertex_membership_quantize;
extern EXPCL_PANDAEGG ConfigVariableInt egg_vertex_max_num_joints;
extern EXPCL_PANDAEGG ConfigVariableBool egg_implicit_alpha_binary;

extern EXPCL_PANDAEGG void init_libegg2pg();

#endif
