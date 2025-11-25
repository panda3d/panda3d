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

TypeHandle NavMeshSettings::_type_handle;

/**
 * Constructor
 */
NavMeshSettings::
NavMeshSettings() {
  // Default values based on Recast defaults
  _cell_size = 0.3f;
  _cell_height = 0.2f;
  _agent_height = 2.0f;
  _agent_radius = 0.6f;
  _agent_max_climb = 0.9f;
  _agent_max_slope = 45.0f;
  _region_min_size = 8.0f;
  _region_merge_size = 20.0f;
  _edge_max_len = 12.0f;
  _edge_max_error = 1.3f;
  _verts_per_poly = 6;
  _detail_sample_dist = 6.0f;
  _detail_sample_max_error = 1.0f;
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

