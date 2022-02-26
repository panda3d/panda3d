/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshQuery.cxx
 * @author ashwini
 * @date 2020-060-21
 */

#include "navMeshQuery.h"
#include "navMeshNode.h"
#include "recastnavigation/DetourNavMeshQuery.h"
#include "lvecBase3.h"
#include "config_navigation.h"
#include <iostream>

/**
 * This constructor forms and initialises query object. No need to call set_nav_query()
 * after calling this constructor.
 */
NavMeshQuery::NavMeshQuery(PT(NavMesh) nav_mesh, int max_nodes) {
  _nav_query = dtAllocNavMeshQuery();
  _nav_query->init(nav_mesh->get_nav_mesh(), max_nodes);
}

/**
 * This constructor forms and initialises query object. No need to call set_nav_query()
 * after calling this constructor.
 */
NavMeshQuery::NavMeshQuery(NodePath nav_mesh_node_path, int max_nodes) {
  _nav_query = dtAllocNavMeshQuery();
  NavMeshNode *nav_mesh = DCAST(NavMeshNode, nav_mesh_node_path.node());
  _nav_query->init(nav_mesh->get_nav_mesh()->get_nav_mesh(), max_nodes);
}


NavMeshQuery::~NavMeshQuery() {
  dtFreeNavMeshQuery(_nav_query);
}

/**
 * Given an input point, this function finds the nearest point
 * to it lying over the navigation mesh surface.
 */
bool NavMeshQuery::nearest_point(LPoint3 &p) {
  if (!_nav_query) {
    navigation_cat.error() << "NavMeshQuery not created!" << std::endl;
    return false;
  }

  LPoint3 center_pt = mat_to_y.xform_point(p);
  const float center[3] = { center_pt[0], center_pt[1], center_pt[2] };  // convert to y-up system
  float nearest_p[3] = { 0, 0, 0 };
  const float extents[3] = { 10 , 10 , 10 };

  dtQueryFilter filter;

  dtPolyRef nearest_poly_ref_id = 0;

  dtStatus status = _nav_query->findNearestPoly(center, extents, &filter, &nearest_poly_ref_id, nearest_p);

  if (dtStatusFailed(status)) {
    navigation_cat.error() << "Cannot find nearest point on polymesh." << std::endl;
    return false;
  }
  p = mat_from_y.xform_point({ nearest_p[0], nearest_p[1], nearest_p[2] }); // convert back from y-up system

  return true;
}

/**
 * This function takes two input points as start and end points and finds a
 * path from the start to end point using the NavMesh.
 * It returns a NavMeshPath object that represents the path that was found.
 */
NavMeshPath NavMeshQuery::find_path(LPoint3 &start, LPoint3 &end) {
  pvector<LPoint3> path_array;

  dtPolyRef start_ref = 0;
  dtPolyRef end_ref = 0;
  LPoint3 start_pos_pt = mat_to_y.xform_point(start);
  const float start_pos[3] = { start_pos_pt[0], start_pos_pt[1], start_pos_pt[2] }; // convert to y-up system
  float nearest_start[3] = { 0, 0, 0 };
  LPoint3 end_pos_pt = mat_to_y.xform_point(end);
  const float end_pos[3] = { end_pos_pt[0], end_pos_pt[1], end_pos_pt[2] }; // convert to y-up system
  float nearest_end[3] = { 0, 0, 0 };
  dtQueryFilter filter;
  dtPolyRef path[MAX_POLYS];
  int path_count;
  const float extents[3] = { 10, 10, 10 };

  dtStatus status = _nav_query->findNearestPoly(start_pos, extents, &filter, &start_ref, nearest_start);
  if (dtStatusFailed(status)) {
    navigation_cat.error() << "Cannot find nearest point on polymesh for start point." << std::endl;
    return {};
  }

  status = _nav_query->findNearestPoly(end_pos, extents, &filter, &end_ref, nearest_end);
  if (dtStatusFailed(status)) {
    navigation_cat.error() << "Cannot find nearest point on polymesh for end point." << std::endl;
    return {};
  }

  status = _nav_query->findPath(start_ref, end_ref, nearest_start, nearest_end, &filter, path, &path_count, MAX_POLYS);
  if (dtStatusFailed(status)) {
    navigation_cat.error() << "Cannot find the path." << std::endl;
    return {};
  }

  float pos[3] = {start_pos[0], start_pos[1], start_pos[2]};

  for (int i=0; i < path_count; i++) {
    float closest[3] = { 0, 0, 0 };

    status = _nav_query->closestPointOnPolyBoundary(path[i], pos, closest);
    if (dtStatusFailed(status)) {
      navigation_cat.error() << "Cannot find the nearest point on polygon." << std::endl;
      return {};
    }

    pos[0] = closest[0];
    pos[1] = closest[1];
    pos[2] = closest[2];
    LPoint3 point = mat_from_y.xform_point({ closest[0], closest[1], closest[2] }); // convert back from y-up system
    path_array.push_back(point);
  }

  path_array.push_back(end);

  return NavMeshPath(path_array);
}

/**
 * This function takes two input points as start and end points and finds a straight
 * path from the start to end point using the NavMesh. This method performs what is
 * often called 'string pulling', which might not properly account for terrain height.
 * It returns a NavMeshPath object that represents the path that was found.
 */
NavMeshPath NavMeshQuery::find_straight_path(LPoint3 &start, LPoint3 &end, int opt) {
  pvector<LPoint3> straight_path_array;

  dtPolyRef start_ref = 0;
  dtPolyRef end_ref = 0;
  LPoint3 start_pos_pt = mat_to_y.xform_point(start);
  const float start_pos[3] = { start_pos_pt[0], start_pos_pt[1], start_pos_pt[2] }; // convert to y-up system
  float nearest_start[3] = { 0, 0, 0 };
  LPoint3 end_pos_pt = mat_to_y.xform_point(end);
  const float end_pos[3] = { end_pos_pt[0], end_pos_pt[1], end_pos_pt[2] }; // convert to y-up system
  float nearest_end[3] = { 0, 0, 0 };
  dtQueryFilter filter;
  dtPolyRef path[MAX_POLYS];
  int path_count;
  const float extents[3] = { 10, 10, 10 };

  dtStatus status = _nav_query->findNearestPoly(start_pos, extents, &filter, &start_ref, nearest_start);
  if (dtStatusFailed(status)) {
    navigation_cat.error() << "Cannot find nearest point on polymesh for start point." << std::endl;
    return {};
  }

  status = _nav_query->findNearestPoly(end_pos, extents, &filter, &end_ref, nearest_end);
  if (dtStatusFailed(status)) {
    navigation_cat.error() << "Cannot find nearest point on polymesh for end point." << std::endl;
    return {};
  }

  status = _nav_query->findPath(start_ref, end_ref, nearest_start, nearest_end, &filter, path, &path_count, MAX_POLYS);
  if (dtStatusFailed(status)) {
    navigation_cat.error() << "Cannot find the path." << std::endl;
    return {};
  }

  float straight_path[MAX_POLYS*3];
  unsigned char straight_path_flags[MAX_POLYS];
  dtPolyRef straight_path_refs[MAX_POLYS];
  int straight_path_count;

  status = _nav_query->findStraightPath(nearest_start, nearest_end, path, path_count, straight_path, straight_path_flags, straight_path_refs, &straight_path_count, MAX_POLYS, opt);
  if (dtStatusFailed(status)) {
    navigation_cat.error() << "Cannot find the straight path." << std::endl;
    return {};
  }

  for (int i=0;i<straight_path_count*3;i+=3) {
    LPoint3 point = mat_from_y.xform_point({ straight_path[i], straight_path[i+1], straight_path[i+2] });  // convert back from y-up system
    straight_path_array.push_back(point);
  }

  straight_path_array.push_back(end);

  return NavMeshPath(straight_path_array);
}
