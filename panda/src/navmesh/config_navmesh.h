/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_navmesh.h
 * @author Ashwani / 
 * @date 2024
 */

#ifndef CONFIG_NAVMESH_H
#define CONFIG_NAVMESH_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"
#include "configVariableDouble.h"
#include "configVariableInt.h"

#ifndef EXPCL_PANDA_NAVMESH
  #ifdef BUILDING_PANDA_NAVMESH
    #define EXPCL_PANDA_NAVMESH EXPORT_CLASS
    #define EXPTP_PANDA_NAVMESH EXPORT_TEMPL
  #else
    #define EXPCL_PANDA_NAVMESH IMPORT_CLASS
    #define EXPTP_PANDA_NAVMESH IMPORT_TEMPL
  #endif
#endif

ConfigureDecl(config_navmesh, EXPCL_PANDA_NAVMESH, EXPTP_PANDA_NAVMESH);
NotifyCategoryDecl(navmesh, EXPCL_PANDA_NAVMESH, EXPTP_PANDA_NAVMESH);

extern ConfigVariableDouble navmesh_cell_size;
extern ConfigVariableDouble navmesh_cell_height;
extern ConfigVariableDouble navmesh_agent_height;
extern ConfigVariableDouble navmesh_agent_radius;
extern ConfigVariableDouble navmesh_agent_max_climb;
extern ConfigVariableDouble navmesh_agent_max_slope;
extern ConfigVariableDouble navmesh_region_min_size;
extern ConfigVariableDouble navmesh_region_merge_size;
extern ConfigVariableDouble navmesh_edge_max_len;
extern ConfigVariableDouble navmesh_edge_max_error;
extern ConfigVariableInt    navmesh_verts_per_poly;
extern ConfigVariableDouble navmesh_detail_sample_dist;
extern ConfigVariableDouble navmesh_detail_sample_max_error;

extern EXPCL_PANDA_NAVMESH void init_libnavmesh();

#endif

