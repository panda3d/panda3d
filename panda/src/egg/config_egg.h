// Filename: config_egg.h
// Created by:  drose (19Mar00)
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

#ifndef CONFIG_EGG_H
#define CONFIG_EGG_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableSearchPath.h"
#include "configVariableBool.h"
#include "configVariableEnum.h"
#include "configVariableDouble.h"
#include "configVariableInt.h"

NotifyCategoryDecl(egg, EXPCL_PANDAEGG, EXPTP_PANDAEGG);

extern ConfigVariableSearchPath egg_path;
extern ConfigVariableBool egg_support_old_anims;

extern EXPCL_PANDAEGG ConfigVariableBool egg_mesh;
extern EXPCL_PANDAEGG ConfigVariableBool egg_retesselate_coplanar;
extern EXPCL_PANDAEGG ConfigVariableBool egg_unroll_fans;
extern EXPCL_PANDAEGG ConfigVariableBool egg_show_tstrips;
extern EXPCL_PANDAEGG ConfigVariableBool egg_show_qsheets;
extern EXPCL_PANDAEGG ConfigVariableBool egg_show_quads;
#define egg_false_color (egg_show_tstrips | egg_show_qsheets | egg_show_quads)
extern EXPCL_PANDAEGG ConfigVariableBool egg_subdivide_polys;
extern EXPCL_PANDAEGG ConfigVariableBool egg_consider_fans;
extern EXPCL_PANDAEGG ConfigVariableDouble egg_max_tfan_angle;
extern EXPCL_PANDAEGG ConfigVariableInt egg_min_tfan_tris;
extern EXPCL_PANDAEGG ConfigVariableDouble egg_coplanar_threshold;

extern EXPCL_PANDAEGG void init_libegg();

#endif
