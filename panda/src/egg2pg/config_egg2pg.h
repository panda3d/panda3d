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
extern EXPCL_PANDAEGG ConfigVariableBool egg_unify;
extern EXPCL_PANDAEGG ConfigVariableDouble egg_flatten_radius;
extern EXPCL_PANDAEGG ConfigVariableBool egg_combine_geoms;
extern EXPCL_PANDAEGG ConfigVariableBool egg_show_collision_solids;
extern EXPCL_PANDAEGG ConfigVariableBool egg_load_old_curves;
extern EXPCL_PANDAEGG ConfigVariableBool egg_load_classic_nurbs_curves;
extern EXPCL_PANDAEGG ConfigVariableBool egg_accept_errors;
extern EXPCL_PANDAEGG ConfigVariableBool egg_accept_errors;
extern EXPCL_PANDAEGG ConfigVariableBool egg_suppress_hidden;
extern EXPCL_PANDAEGG ConfigVariableEnum<EggRenderMode::AlphaMode> egg_alpha_mode;

extern EXPCL_PANDAEGG void init_libegg2pg();

#endif
