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
#include "DetourNavMeshQuery.h"
#include "lvecBase3.h"
#include <iostream>

NavMeshQuery::NavMeshQuery():
  _nav_query(0) {
  _nav_query = dtAllocNavMeshQuery();
}

NavMeshQuery::NavMeshQuery(PT(NavMesh) nav_mesh):
  _nav_query(0) {
  
  _nav_query = dtAllocNavMeshQuery();
  dtStatus status = _nav_query->init(nav_mesh->get_nav_mesh(), 2048);

  if (dtStatusFailed(status)) {
    std::cout<<"\nCannot set nav query!\n";
  }

}

NavMeshQuery::~NavMeshQuery() {
  dtFreeNavMeshQuery(_nav_query);
}

bool NavMeshQuery::set_nav_query(PT(NavMesh) nav_mesh) {

  dtStatus status = _nav_query->init(nav_mesh->get_nav_mesh(), 2048);

  if (dtStatusFailed(status)) {
    std::cout<<"\nCannot set nav query!\n";
    return false;
  }

  return true;
}

bool NavMeshQuery::nearest_point(LPoint3 &p) {
  if (!_nav_query) {
    std::cout << "\nNavMeshQuery not created!\n";
    return false;
  }

  const float center[3] = { p[0], p[2], -p[1] };  // convert from z-up system to y-up system
  float *nearest_p = new float[3];
  const float extents[3] = { 10 , 10 , 10 };

  dtQueryFilter filter;
  
  dtPolyRef nearest_poly_ref_id = 0;

  dtStatus status = _nav_query->findNearestPoly(center, extents, &filter, &nearest_poly_ref_id, nearest_p);

  if (dtStatusFailed(status)) {
    std::cout << "\nCannot find nearest point on polymesh.\n";
    return false;
  }
  p = LPoint3(nearest_p[0], -nearest_p[2], nearest_p[1]); // convert back from y-up system to z-up system

  return true;
}

PTA_LVecBase3 NavMeshQuery::find_path(LPoint3 &start, LPoint3 &end) {

  PTA_LVecBase3 path_array;

  dtPolyRef start_ref = 0;
  dtPolyRef end_ref = 0;
  const float start_pos[3] = { start[0], start[2], -start[1] }; // convert from z-up system to y-up system
  float *nearest_start = new float[3];
  const float end_pos[3] = { end[0], end[2], -end[1] }; // convert from z-up system to y-up system
  float *nearest_end = new float[3];
  dtQueryFilter filter;
  dtPolyRef path[MAX_POLYS];
  int path_count;
  float *path_cost;
  const float extents[3] = { 10, 10, 10 };

  dtStatus status = _nav_query->findNearestPoly(start_pos, extents, &filter, &start_ref, nearest_start);
  if (dtStatusFailed(status)) {
    std::cout << "\nCannot find nearest point on polymesh for start point.\n";
    return path_array;
  }
  std::cout<<"\nStart_ref: "<<start_ref<<"\n";
  std::cout<<"\nStarting point:\t"<<nearest_start[0]<<"\t"<<-nearest_start[2]<<"\t"<<nearest_start[1]<<"\n";

  status = _nav_query->findNearestPoly(end_pos, extents, &filter, &end_ref, nearest_end);
  if (dtStatusFailed(status)) {
    std::cout << "\nCannot find nearest point on polymesh for end point.\n";
    return path_array;
  }
  std::cout<<"\nEnd_ref: "<<end_ref<<"\n";
  std::cout<<"\nEnding point:\t"<<nearest_end[0]<<"\t"<<-nearest_end[2]<<"\t"<<nearest_end[1]<<"\n";
  
  status = _nav_query->findPath(start_ref, end_ref, nearest_start, nearest_end, &filter, path, &path_count, MAX_POLYS);
  if (dtStatusFailed(status)) {
    std::cout << "\nCannot find the path.\n";
    return path_array;
  }
  std::cout<<"\nNumber of polygons included in path: "<<path_count<<"\n";
  
  float pos[3] = {start_pos[0], start_pos[1], start_pos[2]};

  for(int i=0;i<path_count;i++) {
    
    float *closest = new float[3];
    
    status = _nav_query->closestPointOnPolyBoundary(path[i], pos, closest);
    if (dtStatusFailed(status)) {
      std::cout << "\nCannot find the nearest point on polygon.\n";
      return path_array;
    }

    pos[0] = closest[0];
    pos[1] = closest[1];
    pos[2] = closest[2];
    LVecBase3 point = LVecBase3(closest[0],-closest[2],closest[1]); // convert back from y-up system to z-up system
    path_array.push_back(point);

  }

  return path_array;
}

PTA_LVecBase3 NavMeshQuery::find_straight_path(LPoint3 &start, LPoint3 &end, int opt) {

  PTA_LVecBase3 straight_path_array;

  dtPolyRef start_ref = 0;
  dtPolyRef end_ref = 0;
  const float start_pos[3] = { start[0], start[2], -start[1] }; // convert from z-up system to y-up system
  float *nearest_start = new float[3];
  const float end_pos[3] = { end[0], end[2], -end[1] }; // convert from z-up system to y-up system
  float *nearest_end = new float[3];
  dtQueryFilter filter;
  dtPolyRef path[MAX_POLYS];
  int path_count;
  float *path_cost;
  const float extents[3] = { 10, 10, 10 };

  dtStatus status = _nav_query->findNearestPoly(start_pos, extents, &filter, &start_ref, nearest_start);
  if (dtStatusFailed(status)) {
    std::cout << "\nCannot find nearest point on polymesh for start point.\n";
    return straight_path_array;
  }
  std::cout<<"\nStart_ref: "<<start_ref<<"\n";
  std::cout<<"\nStarting point:\t"<<nearest_start[0]<<"\t"<<-nearest_start[2]<<"\t"<<nearest_start[1]<<"\n";

  status = _nav_query->findNearestPoly(end_pos, extents, &filter, &end_ref, nearest_end);
  if (dtStatusFailed(status)) {
    std::cout << "\nCannot find nearest point on polymesh for end point.\n";
    return straight_path_array;
  }
  std::cout<<"\nEnd_ref: "<<end_ref<<"\n";
  std::cout<<"\nEnding point:\t"<<nearest_end[0]<<"\t"<<-nearest_end[2]<<"\t"<<nearest_end[1]<<"\n";
  
  status = _nav_query->findPath(start_ref, end_ref, nearest_start, nearest_end, &filter, path, &path_count, MAX_POLYS);
  if (dtStatusFailed(status)) {
    std::cout << "\nCannot find the path.\n";
    return straight_path_array;
  }
  std::cout<<"\nNumber of polygons included in path: "<<path_count<<"\n";

  float straight_path[MAX_POLYS*3];
  unsigned char straight_path_flags[MAX_POLYS];
  dtPolyRef straight_path_refs[MAX_POLYS];
  int straight_path_count;

  status = _nav_query->findStraightPath(nearest_start, nearest_end, path, path_count, straight_path, straight_path_flags, straight_path_refs, &straight_path_count, MAX_POLYS, opt);
  if (dtStatusFailed(status)) {
    std::cout << "\nCannot find the straight path.\n";
    return straight_path_array;
  }
  std::cout<<"\nNumber of points included in straight path: "<<straight_path_count<<"\n";

  for(int i=0;i<straight_path_count*3;i+=3) {
    
    LVecBase3 point = LVecBase3( straight_path[i], -straight_path[i+2], straight_path[i+1]);  // convert back from y-up system to z-up system
    straight_path_array.push_back(point);

  }

  return straight_path_array;
}