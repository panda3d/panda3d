/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshSettings.cxx
 * @author Ashwani / 
 * @date 2024
 */

#include "navMeshSettings.h"
#include "config_navmesh.h"

TypeHandle NavMeshSettings::_type_handle;

/**
 * Constructor
 */
NavMeshSettings::
NavMeshSettings() {
  // Initialize from ConfigVariables
  _cell_size = navmesh_cell_size;
  _cell_height = navmesh_cell_height;
  _agent_height = navmesh_agent_height;
  _agent_radius = navmesh_agent_radius;
  _agent_max_climb = navmesh_agent_max_climb;
  _agent_max_slope = navmesh_agent_max_slope;
  _region_min_size = navmesh_region_min_size;
  _region_merge_size = navmesh_region_merge_size;
  _edge_max_len = navmesh_edge_max_len;
  _edge_max_error = navmesh_edge_max_error;
  _verts_per_poly = navmesh_verts_per_poly;
  _detail_sample_dist = navmesh_detail_sample_dist;
  _detail_sample_max_error = navmesh_detail_sample_max_error;
}

/**
 * Copy Constructor
 */
NavMeshSettings::
NavMeshSettings(const NavMeshSettings &copy) :
  _cell_size(copy._cell_size),
  _cell_height(copy._cell_height),
  _agent_height(copy._agent_height),
  _agent_radius(copy._agent_radius),
  _agent_max_climb(copy._agent_max_climb),
  _agent_max_slope(copy._agent_max_slope),
  _region_min_size(copy._region_min_size),
  _region_merge_size(copy._region_merge_size),
  _edge_max_len(copy._edge_max_len),
  _edge_max_error(copy._edge_max_error),
  _verts_per_poly(copy._verts_per_poly),
  _detail_sample_dist(copy._detail_sample_dist),
  _detail_sample_max_error(copy._detail_sample_max_error)
{
}

/**
 * Destructor
 */
NavMeshSettings::
~NavMeshSettings() {
}

/**
 * Validates the settings.
 */
bool NavMeshSettings::
validate() const {
  if (_cell_size <= 0.0f) {
    navmesh_cat.error() << "NavMeshSettings: cell_size must be > 0\n";
    return false;
  }
  if (_cell_height <= 0.0f) {
    navmesh_cat.error() << "NavMeshSettings: cell_height must be > 0\n";
    return false;
  }
  if (_agent_height < 0.0f) {
    navmesh_cat.error() << "NavMeshSettings: agent_height must be >= 0\n";
    return false;
  }
  if (_agent_radius < 0.0f) {
    navmesh_cat.error() << "NavMeshSettings: agent_radius must be >= 0\n";
    return false;
  }
  if (_agent_max_climb < 0.0f) {
    navmesh_cat.error() << "NavMeshSettings: agent_max_climb must be >= 0\n";
    return false;
  }
  if (_agent_max_slope < 0.0f || _agent_max_slope > 90.0f) {
    navmesh_cat.error() << "NavMeshSettings: agent_max_slope must be between 0 and 90\n";
    return false;
  }
  if (_verts_per_poly < 3) {
    navmesh_cat.error() << "NavMeshSettings: verts_per_poly must be >= 3\n";
    return false;
  }
  return true;
}

/**
 * Output method for debugging
 */
void NavMeshSettings::
output(std::ostream &out) const {
  out << "NavMeshSettings("
      << "cs=" << _cell_size << ", "
      << "ch=" << _cell_height << ", "
      << "h=" << _agent_height << ", "
      << "r=" << _agent_radius << ", "
      << "climb=" << _agent_max_climb << ", "
      << "slope=" << _agent_max_slope << ")";
}

