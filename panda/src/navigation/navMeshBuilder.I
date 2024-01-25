/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshBuilder.I
 * @author Maxwell175
 * @date 2022-02-21
 */

/**
 * Sets the NavMeshParams that control building the NavMesh.
 */
#include "navMeshBuilder.h"

INLINE void NavMeshBuilder::
set_params(NavMeshParams &p) {
  _params = NavMeshParams(p);
}

/**
 * Returns the NavMeshParams that control building the NavMesh.
 */
INLINE NavMeshParams& NavMeshBuilder::
get_params() {
  return _params;
}

INLINE void NavMeshBuilder::
update_bounds(LVector3 vert) {
  if (!_bounds_set) {
    _mesh_bMin[0] = vert[0];
    _mesh_bMin[1] = vert[1];
    _mesh_bMin[2] = vert[2];
    _mesh_bMax[0] = vert[0];
    _mesh_bMax[1] = vert[1];
    _mesh_bMax[2] = vert[2];
    _bounds_set = true;
    return;
  }

  if (vert[0] < _mesh_bMin[0]) {
    _mesh_bMin[0] = vert[0];
  }
  if (vert[1] < _mesh_bMin[1]) {
    _mesh_bMin[1] = vert[1];
  }
  if (vert[2] < _mesh_bMin[2]) {
    _mesh_bMin[2] = vert[2];
  }

  if (vert[0] > _mesh_bMax[0]) {
    _mesh_bMax[0] = vert[0];
  }
  if (vert[1] > _mesh_bMax[1]) {
    _mesh_bMax[1] = vert[1];
  }
  if (vert[2] > _mesh_bMax[2]) {
    _mesh_bMax[2] = vert[2];
  }
}

INLINE void NavMeshBuilder::
get_vert_tris(const std::set<NavTriVertGroup> &tri_vert_groups, pvector<float> &verts, pvector<int> &tris) {
  std::unordered_map<LVector3, int> vert_map;
  int num_verts = 0;
  for (const auto &tri_group : tri_vert_groups) {
    auto a_itr = vert_map.find(tri_group.a);
    int a_idx = -1;
    if (a_itr == vert_map.end()) {
      a_idx = num_verts++;
      verts.emplace_back(tri_group.a[0]);
      verts.emplace_back(tri_group.a[1]);
      verts.emplace_back(tri_group.a[2]);
    } else {
      a_idx = (*a_itr).second;
    }

    auto b_itr = vert_map.find(tri_group.b);
    int b_idx = -1;
    if (b_itr == vert_map.end()) {
      b_idx = num_verts++;
      verts.emplace_back(tri_group.b[0]);
      verts.emplace_back(tri_group.b[1]);
      verts.emplace_back(tri_group.b[2]);
    } else {
      b_idx = (*b_itr).second;
    }

    auto c_itr = vert_map.find(tri_group.c);
    int c_idx = -1;
    if (c_itr == vert_map.end()) {
      c_idx = num_verts++;
      verts.emplace_back(tri_group.c[0]);
      verts.emplace_back(tri_group.c[1]);
      verts.emplace_back(tri_group.c[2]);
    } else {
      c_idx = (*c_itr).second;
    }

    tris.emplace_back(a_idx);
    tris.emplace_back(b_idx);
    tris.emplace_back(c_idx);
  }
}
