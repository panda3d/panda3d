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
#include "recastnavigation/DetourCommon.h"

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
bool NavMeshQuery::nearest_point(LPoint3 &p, LVector3 extents) {
  if (!_nav_query) {
    navigation_cat.error() << "NavMeshQuery not created!" << std::endl;
    return false;
  }

  LPoint3 center_pt = mat_to_y.xform_point(p);
  const float center[3] = { center_pt[0], center_pt[1], center_pt[2] };  // convert to y-up system
  float nearest_p[3] = { 0, 0, 0 };
  LVector3 transformed_extents = mat_to_y.xform_point(extents);
  const float extent_array[3] = { fabs(transformed_extents[0]), fabs(transformed_extents[1]), fabs(transformed_extents[2]) };

  dtQueryFilter filter;

  dtPolyRef nearest_poly_ref_id = 0;

  dtStatus status = _nav_query->findNearestPoly(center, extent_array, &filter, &nearest_poly_ref_id, nearest_p);

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
NavMeshPath NavMeshQuery::find_path(LPoint3 &start, LPoint3 &end, LVector3 extents) {
  pvector<LPoint3> path_array;

  dtPolyRef start_ref = 0;
  dtPolyRef end_ref = 0;
  LPoint3 start_pos_pt = mat_to_y.xform_point(start);
  const float start_pos[3] = { start_pos_pt[0], start_pos_pt[1], start_pos_pt[2] }; // convert to y-up system
  float nearest_start[3] = { 0, 0, 0 };
  LPoint3 end_pos_pt = mat_to_y.xform_point(end);
  const float end_pos[3] = { end_pos_pt[0], end_pos_pt[1], end_pos_pt[2] }; // convert to y-up system
  float nearest_end[3] = { 0, 0, 0 };
  dtQueryFilter *filter = _filter.get_filter();
  dtPolyRef path[MAX_POLYS];
  int path_count;
  LVector3 transformed_extents = mat_to_y.xform_point(extents);
  const float extent_array[3] = { fabs(transformed_extents[0]), fabs(transformed_extents[1]), fabs(transformed_extents[2]) };

  dtStatus status = _nav_query->findNearestPoly(start_pos, extent_array, filter, &start_ref, nearest_start);
  if (dtStatusFailed(status)) {
    navigation_cat.error() << "Cannot find nearest point on polymesh for start point." << std::endl;
    return {};
  }

  status = _nav_query->findNearestPoly(end_pos, extent_array, filter, &end_ref, nearest_end);
  if (dtStatusFailed(status)) {
    navigation_cat.error() << "Cannot find nearest point on polymesh for end point." << std::endl;
    return {};
  }

  status = _nav_query->findPath(start_ref, end_ref, nearest_start, nearest_end, filter, path, &path_count, MAX_POLYS);
  if (dtStatusFailed(status)) {
    navigation_cat.error() << "Cannot find the path." << std::endl;
    return {};
  }

  delete filter;

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
NavMeshPath NavMeshQuery::find_straight_path(LPoint3 &start, LPoint3 &end, LVector3 extents, int opt) {
  pvector<LPoint3> straight_path_array;

  dtPolyRef start_ref = 0;
  dtPolyRef end_ref = 0;
  LPoint3 start_pos_pt = mat_to_y.xform_point(start);
  const float start_pos[3] = { start_pos_pt[0], start_pos_pt[1], start_pos_pt[2] }; // convert to y-up system
  float nearest_start[3] = { 0, 0, 0 };
  LPoint3 end_pos_pt = mat_to_y.xform_point(end);
  const float end_pos[3] = { end_pos_pt[0], end_pos_pt[1], end_pos_pt[2] }; // convert to y-up system
  float nearest_end[3] = { 0, 0, 0 };
  dtQueryFilter *filter = _filter.get_filter();
  dtPolyRef path[MAX_POLYS];
  int path_count;
  LVector3 transformed_extents = mat_to_y.xform_point(extents);
  const float extent_array[3] = { fabs(transformed_extents[0]), fabs(transformed_extents[1]), fabs(transformed_extents[2]) };

  dtStatus status = _nav_query->findNearestPoly(start_pos, extent_array, filter, &start_ref, nearest_start);
  if (dtStatusFailed(status)) {
    navigation_cat.error() << "Cannot find nearest point on polymesh for start point." << std::endl;
    return {};
  }

  status = _nav_query->findNearestPoly(end_pos, extent_array, _filter.get_filter(), &end_ref, nearest_end);
  if (dtStatusFailed(status)) {
    navigation_cat.error() << "Cannot find nearest point on polymesh for end point." << std::endl;
    return {};
  }

  status = _nav_query->findPath(start_ref, end_ref, nearest_start, nearest_end, _filter.get_filter(), path, &path_count, MAX_POLYS);
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



inline bool inRange(const float* v1, const float* v2, const float r, const float h)
{
  const float dx = v2[0] - v1[0];
  const float dy = v2[1] - v1[1];
  const float dz = v2[2] - v1[2];
  return (dx*dx + dz*dz) < r*r && fabsf(dy) < h;
}

static bool getSteerTarget(dtNavMeshQuery* navQuery, const float* startPos, const float* endPos,
                           const float minTargetDist,
                           const dtPolyRef* path, const int pathSize,
                           float* steerPos, unsigned char& steerPosFlag, dtPolyRef& steerPosRef,
                           float* outPoints = 0, int* outPointCount = 0)
{
  // Find steer target.
  static const int MAX_STEER_POINTS = 3;
  float steerPath[MAX_STEER_POINTS*3];
  unsigned char steerPathFlags[MAX_STEER_POINTS];
  dtPolyRef steerPathPolys[MAX_STEER_POINTS];
  int nsteerPath = 0;
  navQuery->findStraightPath(startPos, endPos, path, pathSize,
                             steerPath, steerPathFlags, steerPathPolys, &nsteerPath, MAX_STEER_POINTS);
  if (!nsteerPath)
    return false;

  if (outPoints && outPointCount)
  {
    *outPointCount = nsteerPath;
    for (int i = 0; i < nsteerPath; ++i)
      dtVcopy(&outPoints[i*3], &steerPath[i*3]);
  }


  // Find vertex far enough to steer to.
  int ns = 0;
  while (ns < nsteerPath)
  {
    // Stop at Off-Mesh link or when point is further than slop away.
    if ((steerPathFlags[ns] & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ||
        !inRange(&steerPath[ns*3], startPos, minTargetDist, 1000.0f))
      break;
    ns++;
  }
  // Failed to find good point to steer to.
  if (ns >= nsteerPath)
    return false;

  dtVcopy(steerPos, &steerPath[ns*3]);
  steerPos[1] = startPos[1];
  steerPosFlag = steerPathFlags[ns];
  steerPosRef = steerPathPolys[ns];

  return true;
}

static int fixupCorridor(dtPolyRef* path, const int npath, const int maxPath,
                         const dtPolyRef* visited, const int nvisited)
{
  int furthestPath = -1;
  int furthestVisited = -1;

  // Find furthest common polygon.
  for (int i = npath-1; i >= 0; --i)
  {
    bool found = false;
    for (int j = nvisited-1; j >= 0; --j)
    {
      if (path[i] == visited[j])
      {
        furthestPath = i;
        furthestVisited = j;
        found = true;
      }
    }
    if (found)
      break;
  }

  // If no intersection found just return current path.
  if (furthestPath == -1 || furthestVisited == -1)
    return npath;

  // Concatenate paths.

  // Adjust beginning of the buffer to include the visited.
  const int req = nvisited - furthestVisited;
  const int orig = std::min(furthestPath+1, npath);
  int size = std::max(0, npath-orig);
  if (req+size > maxPath)
    size = maxPath-req;
  if (size)
    memmove(path+req, path+orig, size*sizeof(dtPolyRef));

  // Store visited
  for (int i = 0; i < req; ++i)
    path[i] = visited[(nvisited-1)-i];

  return req+size;
}

// This function checks if the path has a small U-turn, that is,
// a polygon further in the path is adjacent to the first polygon
// in the path. If that happens, a shortcut is taken.
// This can happen if the target (T) location is at tile boundary,
// and we're (S) approaching it parallel to the tile edge.
// The choice at the vertex can be arbitrary,
//  +---+---+
//  |:::|:::|
//  +-S-+-T-+
//  |:::|   | <-- the step can end up in here, resulting U-turn path.
//  +---+---+
static int fixupShortcuts(dtPolyRef* path, int npath, dtNavMeshQuery* navQuery)
{
  if (npath < 3)
    return npath;

  // Get connected polygons
  static const int maxNeis = 16;
  dtPolyRef neis[maxNeis];
  int nneis = 0;

  const dtMeshTile* tile = 0;
  const dtPoly* poly = 0;
  if (dtStatusFailed(navQuery->getAttachedNavMesh()->getTileAndPolyByRef(path[0], &tile, &poly)))
    return npath;

  for (unsigned int k = poly->firstLink; k != DT_NULL_LINK; k = tile->links[k].next)
  {
    const dtLink* link = &tile->links[k];
    if (link->ref != 0)
    {
      if (nneis < maxNeis)
        neis[nneis++] = link->ref;
    }
  }

  // If any of the neighbour polygons is within the next few polygons
  // in the path, short cut to that polygon directly.
  static const int maxLookAhead = 6;
  int cut = 0;
  for (int i = dtMin(maxLookAhead, npath) - 1; i > 1 && cut == 0; i--) {
    for (int j = 0; j < nneis; j++)
    {
      if (path[i] == neis[j]) {
        cut = i;
        break;
      }
    }
  }
  if (cut > 1)
  {
    int offset = cut-1;
    npath -= offset;
    for (int i = 1; i < npath; i++)
      path[i] = path[i+offset];
  }

  return npath;
}

NavMeshPath NavMeshQuery::find_smooth_path(LPoint3 &start, LPoint3 &end, LVector3 extents) {
  pvector<LPoint3> path_array;

  dtPolyRef start_ref = 0;
  dtPolyRef end_ref = 0;
  LPoint3 start_pos_pt = mat_to_y.xform_point(start);
  const float start_pos[3] = { start_pos_pt[0], start_pos_pt[1], start_pos_pt[2] }; // convert to y-up system
  float nearest_start[3] = { 0, 0, 0 };
  LPoint3 end_pos_pt = mat_to_y.xform_point(end);
  const float end_pos[3] = { end_pos_pt[0], end_pos_pt[1], end_pos_pt[2] }; // convert to y-up system
  float nearest_end[3] = { 0, 0, 0 };
  dtPolyRef path[MAX_POLYS];
  int path_count;
  LVector3 transformed_extents = mat_to_y.xform_point(extents);
  const float extent_array[3] = { fabs(transformed_extents[0]), fabs(transformed_extents[1]), fabs(transformed_extents[2]) };

  dtStatus status = _nav_query->findNearestPoly(start_pos, extent_array, _filter.get_filter(), &start_ref, nearest_start);
  if (dtStatusFailed(status)) {
    navigation_cat.error() << "Cannot find nearest point on polymesh for start point." << std::endl;
    return {};
  }

  status = _nav_query->findNearestPoly(end_pos, extent_array, _filter.get_filter(), &end_ref, nearest_end);
  if (dtStatusFailed(status)) {
    navigation_cat.error() << "Cannot find nearest point on polymesh for end point." << std::endl;
    return {};
  }

  status = _nav_query->findPath(start_ref, end_ref, nearest_start, nearest_end, _filter.get_filter(), path, &path_count, MAX_POLYS);
  if (dtStatusFailed(status)) {
    navigation_cat.error() << "Cannot find the path." << std::endl;
    return {};
  }

  pvector<float> smooth_path;

  if (path_count)
  {
    // Iterate over the path to find smooth path on the detail mesh surface.
    dtPolyRef polys[MAX_POLYS];
    memcpy(polys, path, sizeof(dtPolyRef)*path_count);
    int npolys = path_count;

    float iterPos[3], targetPos[3];
    _nav_query->closestPointOnPoly(start_ref, start_pos, iterPos, 0);
    _nav_query->closestPointOnPoly(polys[npolys-1], end_pos, targetPos, 0);

    static const float STEP_SIZE = 0.5f;
    static const float SLOP = 0.01f;
    static const int MAX_SMOOTH = 2048;

    smooth_path.insert(smooth_path.begin(), std::begin(iterPos), std::end(iterPos));

    // Move towards target a small advancement at a time until target reached or
    // when ran out of memory to store the path.
    while (npolys && smooth_path.size() / 3 < MAX_SMOOTH)
    {
      // Find location to steer towards.
      float steerPos[3];
      unsigned char steerPosFlag;
      dtPolyRef steerPosRef;

      if (!getSteerTarget(_nav_query, iterPos, targetPos, SLOP,
                          polys, npolys, steerPos, steerPosFlag, steerPosRef))
        break;

      bool endOfPath = (steerPosFlag & DT_STRAIGHTPATH_END) != 0;
      bool offMeshConnection = (steerPosFlag & DT_STRAIGHTPATH_OFFMESH_CONNECTION) != 0;

      // Find movement delta.
      float delta[3], len;
      dtVsub(delta, steerPos, iterPos);
      len = dtMathSqrtf(dtVdot(delta, delta));
      // If the steer target is end of path or off-mesh link, do not move past the location.
      if ((endOfPath || offMeshConnection) && len < STEP_SIZE)
        len = 1;
      else
        len = STEP_SIZE / len;
      float moveTgt[3];
      dtVmad(moveTgt, iterPos, delta, len);

      // Move
      float result[3];
      dtPolyRef visited[16];
      int nvisited = 0;
      _nav_query->moveAlongSurface(polys[0], iterPos, moveTgt, _filter.get_filter(),
                                   result, visited, &nvisited, 16);

      npolys = fixupCorridor(polys, npolys, MAX_POLYS, visited, nvisited);
      npolys = fixupShortcuts(polys, npolys, _nav_query);

      float h = 0;
      _nav_query->getPolyHeight(polys[0], result, &h);
      result[1] = h;
      dtVcopy(iterPos, result);

      // Handle end of path and off-mesh links when close enough.
      if (endOfPath && inRange(iterPos, steerPos, SLOP, 1.0f))
      {
        // Reached end of path.
        dtVcopy(iterPos, targetPos);
        if (smooth_path.size() / 3 < MAX_SMOOTH)
        {
          smooth_path.insert(smooth_path.begin(), std::begin(iterPos), std::end(iterPos));
        }
        break;
      }
      else if (offMeshConnection && inRange(iterPos, steerPos, SLOP, 1.0f))
      {
        // Reached off-mesh connection.
        float startPos[3], endPos[3];

        // Advance the path up to and over the off-mesh connection.
        dtPolyRef prevRef = 0, polyRef = polys[0];
        int npos = 0;
        while (npos < npolys && polyRef != steerPosRef)
        {
          prevRef = polyRef;
          polyRef = polys[npos];
          npos++;
        }
        for (int i = npos; i < npolys; ++i)
          polys[i-npos] = polys[i];
        npolys -= npos;

        // Handle the connection.
        dtStatus status = _nav_query->getAttachedNavMesh()->getOffMeshConnectionPolyEndPoints(prevRef, polyRef, startPos, endPos);
        if (dtStatusSucceed(status))
        {
          if (smooth_path.size() / 3 < MAX_SMOOTH)
          {
            smooth_path.insert(smooth_path.begin(), std::begin(startPos), std::end(startPos));

            // Hack to make the dotted path not visible during off-mesh connection.
            if (smooth_path.size() / 3 & 1)
            {
              smooth_path.insert(smooth_path.begin(), std::begin(startPos), std::end(startPos));
            }
          }
          // Move position at the other side of the off-mesh link.
          dtVcopy(iterPos, endPos);
          float eh = 0.0f;
          _nav_query->getPolyHeight(polys[0], iterPos, &eh);
          iterPos[1] = eh;
        }
      }

      // Store results.
      if (smooth_path.size() / 3 < MAX_SMOOTH)
      {
        smooth_path.insert(smooth_path.begin(), std::begin(iterPos), std::end(iterPos));
      }
    }
  }

  for (int i=0; i < smooth_path.size(); i += 3) {
    LPoint3 point = mat_from_y.xform_point({ smooth_path[i], smooth_path[i + 1], smooth_path[i + 2] }); // convert back from y-up system
    path_array.push_back(point);
  }

  return NavMeshPath(path_array);
}
