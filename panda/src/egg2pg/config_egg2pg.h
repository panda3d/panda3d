// Filename: config_egg2pg.h
// Created by:  drose (26Feb02)
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

#ifndef CONFIG_EGG2PG_H
#define CONFIG_EGG2PG_H

#include "pandabase.h"

#include "coordinateSystem.h"
#include "eggRenderMode.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

ConfigureDecl(config_egg2pg, EXPCL_PANDAEGG, EXPTP_PANDAEGG);
NotifyCategoryDecl(egg2pg, EXPCL_PANDAEGG, EXPTP_PANDAEGG);

extern EXPCL_PANDAEGG bool egg_mesh;
extern EXPCL_PANDAEGG bool egg_retesselate_coplanar;
extern EXPCL_PANDAEGG bool egg_unroll_fans;
extern EXPCL_PANDAEGG bool egg_show_tstrips;
extern EXPCL_PANDAEGG bool egg_show_qsheets;
extern EXPCL_PANDAEGG bool egg_show_quads;
extern EXPCL_PANDAEGG bool egg_false_color;
extern EXPCL_PANDAEGG bool egg_show_normals;
extern EXPCL_PANDAEGG double egg_normal_scale;
extern EXPCL_PANDAEGG bool egg_subdivide_polys;
extern EXPCL_PANDAEGG bool egg_consider_fans;
extern EXPCL_PANDAEGG double egg_max_tfan_angle;
extern EXPCL_PANDAEGG int egg_min_tfan_tris;
extern EXPCL_PANDAEGG double egg_coplanar_threshold;
extern EXPCL_PANDAEGG CoordinateSystem egg_coordinate_system;
extern EXPCL_PANDAEGG bool egg_ignore_mipmaps;
extern EXPCL_PANDAEGG bool egg_ignore_filters;
extern EXPCL_PANDAEGG bool egg_ignore_clamp;
extern EXPCL_PANDAEGG bool egg_always_decal_textures;
extern EXPCL_PANDAEGG bool egg_ignore_decals;
extern EXPCL_PANDAEGG bool egg_flatten;
extern EXPCL_PANDAEGG bool egg_flatten_siblings;
extern EXPCL_PANDAEGG bool egg_show_collision_solids;
extern EXPCL_PANDAEGG bool egg_load_old_curves;
extern EXPCL_PANDAEGG bool egg_load_classic_nurbs_curves;
extern EXPCL_PANDAEGG bool egg_accept_errors;
extern EXPCL_PANDAEGG bool egg_accept_errors;
extern EXPCL_PANDAEGG bool egg_suppress_hidden;
extern EXPCL_PANDAEGG EggRenderMode::AlphaMode egg_alpha_mode;

extern EXPCL_PANDAEGG void init_libegg2pg();

#endif
