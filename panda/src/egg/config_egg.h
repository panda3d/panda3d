// Filename: config_egg.h
// Created by:  drose (19Mar00)
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
extern EXPCL_PANDAEGG ConfigVariableInt egg_test_vref_integrity;
extern EXPCL_PANDAEGG ConfigVariableInt egg_recursion_limit;

extern EXPCL_PANDAEGG void init_libegg();

#endif
