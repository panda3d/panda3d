/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_navmesh.cxx
 * @author Ashwani / 
 * @date 2024
 */

#include "config_navmesh.h"
#include "navMesh.h"
#include "navMeshBuilder.h"
#include "navMeshSettings.h"
#include "navPath.h"

#include "dconfig.h"

ConfigureDef(config_navmesh);
NotifyCategoryDef(navmesh, "");

ConfigureFn(config_navmesh) {
  init_libnavmesh();
}

ConfigVariableDouble navmesh_cell_size
("navmesh-cell-size", 0.3,
 PRC_DESC("The width/depth of each navigation mesh cell in world units. Default is 0.3."));

ConfigVariableDouble navmesh_cell_height
("navmesh-cell-height", 0.2,
 PRC_DESC("The vertical resolution of the voxelization. Default is 0.2."));

ConfigVariableDouble navmesh_agent_height
("navmesh-agent-height", 2.0,
 PRC_DESC("The height of the agent in world units. Default is 2.0."));

ConfigVariableDouble navmesh_agent_radius
("navmesh-agent-radius", 0.6,
 PRC_DESC("The radius of the agent in world units. Default is 0.6."));

ConfigVariableDouble navmesh_agent_max_climb
("navmesh-agent-max-climb", 0.9,
 PRC_DESC("The maximum vertical step the agent can climb. Default is 0.9."));

ConfigVariableDouble navmesh_agent_max_slope
("navmesh-agent-max-slope", 45.0,
 PRC_DESC("The maximum slope angle (in degrees) the agent can walk on. Default is 45.0."));

ConfigVariableDouble navmesh_region_min_size
("navmesh-region-min-size", 8.0,
 PRC_DESC("The minimum area of a region (in cells) to remain isolated. Default is 8.0."));

ConfigVariableDouble navmesh_region_merge_size
("navmesh-region-merge-size", 20.0,
 PRC_DESC("The maximum area of a region (in cells) to be merged with larger regions. Default is 20.0."));

ConfigVariableDouble navmesh_edge_max_len
("navmesh-edge-max-len", 12.0,
 PRC_DESC("The maximum length of a contour edge in world units. Default is 12.0."));

ConfigVariableDouble navmesh_edge_max_error
("navmesh-edge-max-error", 1.3,
 PRC_DESC("The maximum error allowed when simplifying contours. Default is 1.3."));

ConfigVariableInt navmesh_verts_per_poly
("navmesh-verts-per-poly", 6,
 PRC_DESC("The maximum number of vertices per polygon. Default is 6."));

ConfigVariableDouble navmesh_detail_sample_dist
("navmesh-detail-sample-dist", 6.0,
 PRC_DESC("The sampling distance for detail mesh generation. Default is 6.0."));

ConfigVariableDouble navmesh_detail_sample_max_error
("navmesh-detail-sample-max-error", 1.0,
 PRC_DESC("The maximum error allowed for detail mesh generation. Default is 1.0."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libnavmesh() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  NavMeshPoly::init_type();
  NavMeshSettings::init_type();
  NavPath::init_type();
}
