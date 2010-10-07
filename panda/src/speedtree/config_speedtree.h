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

NotifyCategoryDecl(speedtree, EXPCL_PANDASKEL, EXPTP_PANDASKEL);

extern ConfigVariableString speedtree_license;
extern ConfigVariableFilename speedtree_shaders_dir;
extern ConfigVariableBool speedtree_allow_horizontal_billboards;
extern ConfigVariableInt speedtree_max_num_visible_cells;
extern ConfigVariableInt speedtree_max_billboard_images_by_base;
extern ConfigVariableDouble speedtree_cull_cell_size;

extern EXPCL_PANDASKEL void init_libspeedtree();

#endif


