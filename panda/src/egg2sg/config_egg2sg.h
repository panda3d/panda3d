// Filename: config_egg2sg.h
// Created by:  drose (01Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_EGG2SG_H
#define CONFIG_EGG2SG_H

#include <pandabase.h>

#include <coordinateSystem.h>
#include <typedef.h>
#include <notifyCategoryProxy.h>
#include <dconfig.h>

ConfigureDecl(config_egg2sg, EXPCL_PANDAEGG, EXPTP_PANDAEGG);
NotifyCategoryDecl(egg2sg, EXPCL_PANDAEGG, EXPTP_PANDAEGG);

extern const bool egg_mesh;
extern const bool egg_retesselate_coplanar;
extern const bool egg_unroll_fans;
extern const bool egg_show_tstrips;
extern const bool egg_show_qsheets;
extern const bool egg_show_quads;
extern const bool egg_false_color;
extern const bool egg_show_normals;
extern const double egg_normal_scale;
extern const bool egg_subdivide_polys;
extern const bool egg_consider_fans;
extern const double egg_max_tfan_angle;
extern const int egg_min_tfan_tris;
extern const double egg_coplanar_threshold;
extern CoordinateSystem egg_coordinate_system;
extern const bool egg_ignore_mipmaps;
extern const bool egg_ignore_filters;
extern const bool egg_ignore_clamp;
extern const bool egg_always_decal_textures;
extern const bool egg_ignore_decals;
extern const bool egg_flatten;
extern const bool egg_flatten_siblings;
extern const bool egg_show_collision_solids;



#endif
