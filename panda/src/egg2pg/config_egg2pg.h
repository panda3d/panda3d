/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_egg2pg.h
 * @author drose
 * @date 2002-02-26
 */

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

ConfigureDecl(config_egg2pg, EXPCL_PANDA_EGG2PG, EXPTP_PANDA_EGG2PG);
NotifyCategoryDecl(egg2pg, EXPCL_PANDA_EGG2PG, EXPTP_PANDA_EGG2PG);

extern EXPCL_PANDA_EGG2PG ConfigVariableDouble egg_normal_scale;
extern EXPCL_PANDA_EGG2PG ConfigVariableBool egg_show_normals;
extern EXPCL_PANDA_EGG2PG ConfigVariableEnum<CoordinateSystem> egg_coordinate_system;
extern EXPCL_PANDA_EGG2PG ConfigVariableBool egg_ignore_mipmaps;
extern EXPCL_PANDA_EGG2PG ConfigVariableBool egg_ignore_filters;
extern EXPCL_PANDA_EGG2PG ConfigVariableBool egg_ignore_clamp;
extern EXPCL_PANDA_EGG2PG ConfigVariableBool egg_ignore_decals;
extern EXPCL_PANDA_EGG2PG ConfigVariableBool egg_flatten;
extern EXPCL_PANDA_EGG2PG ConfigVariableDouble egg_flatten_radius;
extern EXPCL_PANDA_EGG2PG ConfigVariableBool egg_unify;
extern EXPCL_PANDA_EGG2PG ConfigVariableBool egg_combine_geoms;
extern EXPCL_PANDA_EGG2PG ConfigVariableBool egg_rigid_geometry;
extern EXPCL_PANDA_EGG2PG ConfigVariableBool egg_flat_shading;
extern EXPCL_PANDA_EGG2PG ConfigVariableBool egg_flat_colors;
extern EXPCL_PANDA_EGG2PG ConfigVariableBool egg_load_old_curves;
extern EXPCL_PANDA_EGG2PG ConfigVariableBool egg_load_classic_nurbs_curves;
extern EXPCL_PANDA_EGG2PG ConfigVariableBool egg_accept_errors;
extern EXPCL_PANDA_EGG2PG ConfigVariableBool egg_suppress_hidden;
extern EXPCL_PANDA_EGG2PG ConfigVariableEnum<EggRenderMode::AlphaMode> egg_alpha_mode;
extern EXPCL_PANDA_EGG2PG ConfigVariableInt egg_max_vertices;
extern EXPCL_PANDA_EGG2PG ConfigVariableInt egg_max_indices;
extern EXPCL_PANDA_EGG2PG ConfigVariableBool egg_emulate_bface;
extern EXPCL_PANDA_EGG2PG ConfigVariableBool egg_preload_simple_textures;
extern EXPCL_PANDA_EGG2PG ConfigVariableDouble egg_vertex_membership_quantize;
extern EXPCL_PANDA_EGG2PG ConfigVariableInt egg_vertex_max_num_joints;
extern EXPCL_PANDA_EGG2PG ConfigVariableBool egg_implicit_alpha_binary;

extern EXPCL_PANDA_EGG2PG void init_libegg2pg();

#endif
