/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMesh.cxx
 * @author ashwini
 * @date 2020-060-21
 */


#include "navMesh.h"
#include "geom.h"
#include "geomTrifans.h"
#include "config_navigation.h"
#include "lvecBase3.h"
#include "geomVertexWriter.h"
#include <iostream>
#include "recastnavigation/Recast.h"
#include "lineSegs.h"
#include "collisionNode.h"
#include "geomVertexReader.h"

TypeHandle NavMesh::_type_handle;


/**
 * NavMesh constructor to initialize member variables.
 */
NavMesh::NavMesh():
  _nav_mesh(nullptr) {}

NavMesh::~NavMesh() {
  dtFreeNavMesh(_nav_mesh);
  _nav_mesh = nullptr;
}

/**
 * NavMesh constructor to store dtNavMesh object as _nav_mesh.
 */
NavMesh::NavMesh(dtNavMesh *nav_mesh,
                 NavMeshParams params,
                 TriVertGroups &untracked_tris,
                 NodePaths &tracked_nodes,
                 TrackedCollInfos &tracked_coll_nodes,
                 pvector<TriVertGroup> &last_tris) :
                 _nav_mesh(nav_mesh),
                 _untracked_tris(untracked_tris),
                 _tracked_coll_nodes(tracked_coll_nodes),
                 _tracked_nodes(tracked_nodes),
                 _params(params) {
  std::copy(last_tris.begin(), last_tris.end(), std::inserter(_last_tris, _last_tris.begin()));
}

/**
 * NavMesh constructor to store NavMesh Parameters.
 */
NavMesh::NavMesh(NavMeshParams mesh_params):
  _nav_mesh(nullptr) {
  //memset(&(_params), 0, sizeof(_params));
  _params = {};
/*
  _params.verts = mesh_params.verts;
  _params.vertCount = mesh_params.vert_count;
  _params.polys = mesh_params.polys;
  _params.polyAreas = mesh_params.poly_areas;
  _params.polyFlags = mesh_params.poly_flags;
  _params.polyCount = mesh_params.poly_count;
  _params.nvp = mesh_params.nvp;
  _params.detailMeshes = mesh_params.detail_meshes;
  _params.detailVerts = mesh_params.detail_verts;
  _params.detailVertsCount = mesh_params.detail_vert_count;
  _params.detailTris = mesh_params.detail_tris;
  _params.detailTriCount = mesh_params.detail_tri_count;


  _params.walkableHeight = mesh_params.walkable_height;
  _params.walkableRadius = mesh_params.walkable_radius;
  _params.walkableClimb = mesh_params.walkable_climb;

  _params.bmin[0] = mesh_params.b_min[0];
  _params.bmin[1] = mesh_params.b_min[1];
  _params.bmin[2] = mesh_params.b_min[2];
  _params.bmax[0] = mesh_params.b_max[0];
  _params.bmax[1] = mesh_params.b_max[1];
  _params.bmax[2] = mesh_params.b_max[2];
  
  _params.cs = mesh_params.cs;
  _params.ch = mesh_params.ch;
  _params.buildBvTree = mesh_params.build_bv_tree;
  
  border_index = mesh_params.border_index;
  */
  init_nav_mesh();
}

/**
 * Function to build navigation mesh from the parameters.
 */
bool NavMesh::init_nav_mesh() {
  unsigned char *nav_data = nullptr;
  int nav_data_size = 0;

  return true;
}

/**
 * This function generates a GeomNode that visually represents the NavMesh.
 */
PT(GeomNode) NavMesh::draw_nav_mesh_geom() {
  bool have_poly_outline_cache = _cache_poly_outlines != nullptr;

  PT(GeomVertexData) vdata;
  vdata = new GeomVertexData("vertexInfo", GeomVertexFormat::get_v3c4(), Geom::UH_static);

  pvector<std::tuple<LPoint3, LColor>> vector_data;
  pvector<pvector<int>> prims;

  PT(GeomNode) node;
  if (have_poly_outline_cache) {
    node = DCAST(GeomNode, _cache_poly_outlines->make_copy());
  } else {
    node = new GeomNode("navmesh_gnode");
  }

  if (_cache_poly_verts.empty()) {
    for (const NavMeshPoly &poly : get_polys()) {
      _cache_poly_verts[poly.get_poly_ref()] = poly.get_verts();
    }
  }

  // Loop through every vertex in every poly, collecting the vertices,
  // which are intentionally duplicated for adjacent polys in order to allow
  // individual polys to have different colors.
  for (const auto &poly : _cache_poly_verts) {
    pvector<int> primitive;

    LineSegs poly_segs;
    poly_segs.set_color(0, 0, 0);
    poly_segs.set_thickness(3);

    for (const LPoint3 &point : poly.second) {
      vector_data.emplace_back(std::make_tuple(point, get_poly_debug_color(poly.first)));
      primitive.emplace_back(vector_data.size() - 1);  // Save off the vertex index for the primitive.
      if (!have_poly_outline_cache) {
        poly_segs.draw_to(point);
      }
    }

    if (!have_poly_outline_cache) {
      poly_segs.create(node);
    }
    prims.emplace_back(primitive);
  }

  if (!have_poly_outline_cache) {
    _cache_poly_outlines = DCAST(GeomNode, node->make_copy());
  }

  vdata->set_num_rows(vector_data.size());

  GeomVertexWriter vertex(vdata, "vertex");
  GeomVertexWriter colour(vdata, "color");

  for (auto &vert_pair : vector_data) {
    vertex.add_data3(std::get<0>(vert_pair));
    colour.add_data4(std::get<1>(vert_pair));
  }

  PT(GeomTrifans) prim;
  prim = new GeomTrifans(Geom::UH_static);

  for (const auto &primitive : prims) {
    for (int vert_idx : primitive) {
      prim->add_vertex(vert_idx);
    }
    prim->close_primitive();
  }

  PT(Geom) polymeshgeom = new Geom(vdata);
  polymeshgeom->add_primitive(prim);

  node->add_geom(polymeshgeom);

  return node;
}

NavMeshPolys NavMesh::
get_polys() {
  NavMeshPolys results;

  const dtNavMesh *navMesh = _nav_mesh;

  for (int i = 0; i < _nav_mesh->getMaxTiles(); i++) {
    const dtMeshTile* tile = navMesh->getTile(i);
    for (int j = 0; j < _nav_mesh->getParams()->maxPolys; j++) {
      dtPolyRef ref = _nav_mesh->encodePolyId(tile->salt, i, j);
      if (_nav_mesh->isValidPolyRef(ref)) {
        results.emplace_back(NavMeshPoly(this, ref));
      }
    }
  }

  return results;
}

/**
 * Finds the NavMeshPoly that is closest to the given point.
 */
NavMeshPoly NavMesh::
get_poly_at(LPoint3 point) {
  dtNavMeshQuery query;

  query.init(_nav_mesh, 8);

  LPoint3 center_pt = mat_to_y.xform_point(point);
  const float center[3] = { center_pt[0], center_pt[1], center_pt[2] };  // convert to y-up system
  float nearest_p[3] = { 0, 0, 0 };
  const float extents[3] = { 10 , 10 , 10 };

  dtQueryFilter filter;

  dtPolyRef nearest_poly_ref_id = 0;

  dtStatus status = query.findNearestPoly(center, extents, &filter, &nearest_poly_ref_id, nearest_p);

  if (dtStatusFailed(status)) {
    navigation_cat.error() << "Cannot find nearest point on polymesh." << std::endl;
    return {this, 0};
  }

  return {this, nearest_poly_ref_id};
}

/**
 * Gets a list of NavMeshPolys that are within the box formed by the passed in half-extents.
 */
NavMeshPolys NavMesh::
get_polys_around(LPoint3 point, LVector3 extents) {
  NavMeshPolys results;

  dtNavMeshQuery query;

  query.init(_nav_mesh, 8);

  LPoint3 center_pt = mat_to_y.xform_point(point);
  const float center[3] = { center_pt[0], center_pt[1], center_pt[2] };  // convert to y-up system

  LVector3 transformed_extents = mat_to_y.xform_point(extents);
  const float extent_array[3] = { fabs(transformed_extents[0]), fabs(transformed_extents[1]), fabs(transformed_extents[2]) };

  dtQueryFilter filter;

  static const int MAX_POLYS = 64;

  dtPolyRef polys[MAX_POLYS] = {};
  int poly_count = 0;

  dtStatus status = query.queryPolygons(center, extent_array, &filter, polys, &poly_count, MAX_POLYS);

  if (dtStatusFailed(status)) {
    navigation_cat.error() << "Cannot find nearest point on polymesh." << std::endl;
    return results;
  }

  for (int i = 0; i < poly_count; i++) {
    results.emplace_back(NavMeshPoly(this, polys[i]));
  }

  return results;
}


void get_vert_tris(std::set<TriVertGroup> &tri_verts, pvector<float> &verts, pvector<int> &tris) {
  std::unordered_map<LVector3, int> vert_map;
  int num_verts = 0;
  for (const auto &tri_group : tri_verts) {
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

/**
 * This is a duplicate of the function in NavMeshBuilder.
 * TODO: Restructure!
 */
unsigned char* NavMesh::buildTileMesh(const int tx, const int ty,
                                      const float* bmin, const float* bmax, int& dataSize,
                                      pvector<float> &verts, pvector<int> &tris) const
{
  rcContext *_ctx = new rcContext;
  rcConfig _cfg = {};
  memset(&_cfg, 0, sizeof(_cfg));
  _cfg.cs = _params.cell_size;
  _cfg.ch = _params.cell_height;
  _cfg.walkableSlopeAngle = _params.agent_max_slope;
  _cfg.walkableHeight = (int)ceilf(_params.agent_height / _cfg.ch);
  _cfg.walkableClimb = (int)floorf(_params.agent_max_climb / _cfg.ch);
  _cfg.walkableRadius = (int)ceilf(_params.agent_radius / _cfg.cs);
  _cfg.maxEdgeLen = (int)(_params.edge_max_len / _params.cell_size);
  _cfg.maxSimplificationError = _params.edge_max_error;
  _cfg.minRegionArea = (int)rcSqr(_params.region_min_size);    // Note: area = size*size
  _cfg.mergeRegionArea = (int)rcSqr(_params.region_merge_size);  // Note: area = size*size
  _cfg.maxVertsPerPoly = (int)_params.verts_per_poly;
  _cfg.tileSize = (int)_params.tile_size;
  _cfg.borderSize = _cfg.walkableRadius + 3; // Reserve enough padding.
  _cfg.width = _cfg.tileSize + _cfg.borderSize*2;
  _cfg.height = _cfg.tileSize + _cfg.borderSize*2;
  _cfg.detailSampleDist = _params.detail_sample_dist < 0.9f ? 0 : _params.cell_size * _params.detail_sample_dist;
  _cfg.detailSampleMaxError = _params.cell_height * _params.detail_sample_max_error;

  // Expand the heighfield bounding box by border size to find the extents of geometry we need to build this tile.
  //
  // This is done in order to make sure that the navmesh tiles connect correctly at the borders,
  // and the obstacles close to the border work correctly with the dilation process.
  // No polygons (or contours) will be created on the border area.
  //
  // IMPORTANT!
  //
  //   :''''''''':
  //   : +-----+ :
  //   : |     | :
  //   : |     |<--- tile to build
  //   : |     | :
  //   : +-----+ :<-- geometry needed
  //   :.........:
  //
  // You should use this bounding box to query your input geometry.
  //
  // For example if you build a navmesh for terrain, and want the navmesh tiles to match the terrain tile size
  // you will need to pass in data from neighbour terrain tiles too! In a simple case, just pass in all the 8 neighbours,
  // or use the bounding box below to only pass in a sliver of each of the 8 neighbours.
  rcVcopy(_cfg.bmin, bmin);
  rcVcopy(_cfg.bmax, bmax);
  _cfg.bmin[0] -= _cfg.borderSize*_cfg.cs;
  _cfg.bmin[2] -= _cfg.borderSize*_cfg.cs;
  _cfg.bmax[0] += _cfg.borderSize*_cfg.cs;
  _cfg.bmax[2] += _cfg.borderSize*_cfg.cs;


  // Allocate voxel heightfield where we rasterize our input data to.
  auto _solid = rcAllocHeightfield();
  if (!_solid)
  {
    navigation_cat.error() << "buildNavigation: Out of memory 'solid'." << std::endl;
    return nullptr;
  }
  if (!rcCreateHeightfield(_ctx, *_solid, _cfg.width, _cfg.height, _cfg.bmin, _cfg.bmax, _cfg.cs, _cfg.ch))
  {
    navigation_cat.error() << "buildNavigation: Could not create solid heightfield." << std::endl;
    return nullptr;
  }

  float tbmin[2], tbmax[2];
  tbmin[0] = _cfg.bmin[0];
  tbmin[1] = _cfg.bmin[2];
  tbmax[0] = _cfg.bmax[0];
  tbmax[1] = _cfg.bmax[2];

  // Allocate array that can hold triangle area types.
  // If you have multiple meshes you need to process, allocate
  // an array which can hold the max number of triangles you need to process.
  pvector<unsigned char> _triareas;
  _triareas.resize(static_cast<int>(tris.size() / 3), 0);

  // Find triangles which are walkable based on their slope and rasterize them.
  // If your input data is multiple meshes, you can transform them here, calculate
  // the are type for each of the meshes and rasterize them.
  rcMarkWalkableTriangles(_ctx, _cfg.walkableSlopeAngle, verts.data(), static_cast<int>(verts.size() / 3), tris.data(), static_cast<int>(tris.size() / 3), _triareas.data());

  if (!rcRasterizeTriangles(_ctx, verts.data(), static_cast<int>(verts.size() / 3), tris.data(), _triareas.data(), static_cast<int>(tris.size() / 3), *_solid, _cfg.walkableClimb)) {
    navigation_cat.error() << "build(): Could not rasterize triangles." << std::endl;
    return nullptr;
  }


  //
  // Step 3. Filter walkables surfaces.
  //

  // Once all geoemtry is rasterized, we do initial pass of filtering to
  // remove unwanted overhangs caused by the conservative rasterization
  // as well as filter spans where the character cannot possibly stand.
  if (_params.filter_low_hanging_obstacles)
    rcFilterLowHangingWalkableObstacles(_ctx, _cfg.walkableClimb, *_solid);
  if (_params.filter_ledge_spans)
    rcFilterLedgeSpans(_ctx, _cfg.walkableHeight, _cfg.walkableClimb, *_solid);
  if (_params.filter_walkable_low_height_spans)
    rcFilterWalkableLowHeightSpans(_ctx, _cfg.walkableHeight, *_solid);


  //
  // Step 4. Partition walkable surface to simple regions.
  //

  // Compact the heightfield so that it is faster to handle from now on.
  // This will result more cache coherent data as well as the neighbours
  // between walkable cells will be calculated.
  auto _chf = rcAllocCompactHeightfield();
  if (!_chf) {
    _ctx->log(RC_LOG_ERROR, "build(): Out of memory 'chf'.");
    navigation_cat.error() << "build(): Out of memory 'chf'." << std::endl;
    return nullptr;
  }
  if (!rcBuildCompactHeightfield(_ctx, _cfg.walkableHeight, _cfg.walkableClimb, *_solid, *_chf)) {
    _ctx->log(RC_LOG_ERROR, "build(): Could not build compact data.");
    navigation_cat.error() << "build(): Could not build compact data." << std::endl;
    return nullptr;
  }

  rcFreeHeightField(_solid);
  _solid = nullptr;

  // Erode the walkable area by agent radius.
  if (!rcErodeWalkableArea(_ctx, _cfg.walkableRadius, *_chf)) {
    _ctx->log(RC_LOG_ERROR, "build(): Could not erode.");
    navigation_cat.error() << "build(): Could not erode." << std::endl;
    return nullptr;
  }


  // Partition the heightfield so that we can use simple algorithm later to triangulate the walkable areas.
  // There are 3 martitioning methods, each with some pros and cons:
  // 1) Watershed partitioning
  //   - the classic Recast partitioning
  //   - creates the nicest tessellation
  //   - usually slowest
  //   - partitions the heightfield into nice regions without holes or overlaps
  //   - the are some corner cases where this method creates produces holes and overlaps
  //      - holes may appear when a small obstacles is close to large open area (triangulation can handle this)
  //      - overlaps may occur if you have narrow spiral corridors (i.e stairs), this make triangulation to fail
  //   * generally the best choice if you precompute the nacmesh, use this if you have large open areas
  // 2) Monotone partioning
  //   - fastest
  //   - partitions the heightfield into regions without holes and overlaps (guaranteed)
  //   - creates long thin polygons, which sometimes causes paths with detours
  //   * use this if you want fast navmesh generation
  // 3) Layer partitoining
  //   - quite fast
  //   - partitions the heighfield into non-overlapping regions
  //   - relies on the triangulation code to cope with holes (thus slower than monotone partitioning)
  //   - produces better triangles than monotone partitioning
  //   - does not have the corner cases of watershed partitioning
  //   - can be slow and create a bit ugly tessellation (still better than monotone)
  //     if you have large open areas with small obstacles (not a problem if you use tiles)
  //   * good choice to use for tiled navmesh with medium and small sized tiles

  switch (_params.partition_type) {
    case NavMeshParams::SAMPLE_PARTITION_WATERSHED:
      // Prepare for region partitioning, by calculating distance field along the walkable surface.
      if (!rcBuildDistanceField(_ctx, *_chf)) {
        _ctx->log(RC_LOG_ERROR, "build(): Could not build distance field.");
        navigation_cat.error() << "build(): Could not build distance field." << std::endl;
        return nullptr;
      }

      // Partition the walkable surface into simple regions without holes.
      if (!rcBuildRegions(_ctx, *_chf, _cfg.borderSize, _cfg.minRegionArea, _cfg.mergeRegionArea)) {
        _ctx->log(RC_LOG_ERROR, "build(): Could not build watershed regions.");
        navigation_cat.error() << "build(): Could not build watershed regions." << std::endl;
        return nullptr;
      }
      break;
    case NavMeshParams::SAMPLE_PARTITION_MONOTONE:
      // Partition the walkable surface into simple regions without holes.
      // Monotone partitioning does not need distancefield.
      if (!rcBuildRegionsMonotone(_ctx, *_chf, _cfg.borderSize, _cfg.minRegionArea, _cfg.mergeRegionArea)) {
        _ctx->log(RC_LOG_ERROR, "build(): Could not build monotone regions.");
        navigation_cat.error() << "build(): Could not build monotone regions." << std::endl;
        return nullptr;
      }
      break;
    case NavMeshParams::SAMPLE_PARTITION_LAYERS:
      // Partition the walkable surface into simple regions without holes.
      if (!rcBuildLayerRegions(_ctx, *_chf, _cfg.borderSize, _cfg.minRegionArea)) {
        _ctx->log(RC_LOG_ERROR, "build(): Could not build layer regions.");
        navigation_cat.error() << "build(): Could not build layer regions." << std::endl;
        return nullptr;
      }
      break;
  }

  //
  // Step 5. Trace and simplify region contours.
  //

  // Create contours.
  auto _cset = rcAllocContourSet();
  if (!_cset) {
    _ctx->log(RC_LOG_ERROR, "build(): Out of memory 'cset'.");
    navigation_cat.error() << "build(): Out of memory 'cset'." << std::endl;
    return nullptr;
  }
  if (!rcBuildContours(_ctx, *_chf, _cfg.maxSimplificationError, _cfg.maxEdgeLen, *_cset)) {
    _ctx->log(RC_LOG_ERROR, "build(): Could not create contours.");
    navigation_cat.error() << "build(): Could not create contours." << std::endl;
    return nullptr;
  }

  //
  // Step 6. Build polygons mesh from contours.
  //

  // Build polygon navmesh from the contours.
  auto _pmesh = rcAllocPolyMesh();
  if (!_pmesh) {
    _ctx->log(RC_LOG_ERROR, "build(): Out of memory 'pmesh'.");
    navigation_cat.error() << "build(): Out of memory 'pmesh'." << std::endl;
    return nullptr;
  }
  if (!rcBuildPolyMesh(_ctx, *_cset, _cfg.maxVertsPerPoly, *_pmesh)) {
    _ctx->log(RC_LOG_ERROR, "build(): Could not triangulate contours.");
    navigation_cat.error() << "build(): Could not triangulate contours." << std::endl;
    return nullptr;
  }

  //
  // Step 7. Create detail mesh which allows to access approximate height on each polygon.
  //

  auto _dmesh = rcAllocPolyMeshDetail();
  if (!_dmesh) {
    _ctx->log(RC_LOG_ERROR, "build(): Out of memory 'pmdtl'.");
    navigation_cat.error() << "build(): Out of memory 'pmdt1'." << std::endl;
    return nullptr;
  }

  if (!rcBuildPolyMeshDetail(_ctx, *_pmesh, *_chf, _cfg.detailSampleDist, _cfg.detailSampleMaxError, *_dmesh)) {
    _ctx->log(RC_LOG_ERROR, "build(): Could not build detail mesh.");
    navigation_cat.error() << "build(): Could not build detail mesh." << std::endl;
    return nullptr;
  }
  navigation_cat.info() << "Number of vertices: " << _pmesh->nverts << std::endl;
  navigation_cat.info() << "Number of polygons: " << _pmesh->npolys << std::endl;
  navigation_cat.info() << "Number of allocated polygons: " << _pmesh->maxpolys << std::endl;

  rcFreeCompactHeightfield(_chf);
  _chf = nullptr;
  rcFreeContourSet(_cset);
  _cset = nullptr;

  if (_pmesh->npolys == 0) {
    // There are no matching polys. Skip this tile.
    return nullptr;
  }

  unsigned char* navData = nullptr;
  int navDataSize = 0;
  if (_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
  {
    if (_pmesh->nverts >= 0xffff)
    {
      // The vertex indices are ushorts, and cannot point to more than 0xffff vertices.
      navigation_cat.error() << "Too many vertices per tile " << _pmesh->nverts << " (max: " << 0xffff << ")." << std::endl;
      return nullptr;
    }

    for (int i = 0; i < _pmesh->npolys; ++i) {
      // Initialize all polygons to 1, so they are enabled by default.
      _pmesh->flags[i] = 1;
    }

    dtNavMeshCreateParams params;
    memset(&params, 0, sizeof(params));
    params.verts = _pmesh->verts;
    params.vertCount = _pmesh->nverts;
    params.polys = _pmesh->polys;
    params.polyAreas = _pmesh->areas;
    params.polyFlags = _pmesh->flags;
    params.polyCount = _pmesh->npolys;
    params.nvp = _pmesh->nvp;
    params.detailMeshes = _dmesh->meshes;
    params.detailVerts = _dmesh->verts;
    params.detailVertsCount = _dmesh->nverts;
    params.detailTris = _dmesh->tris;
    params.detailTriCount = _dmesh->ntris;
    params.walkableHeight = _params.agent_height;
    params.walkableRadius = _params.agent_radius;
    params.walkableClimb = _params.agent_max_climb;
    params.tileX = tx;
    params.tileY = ty;
    params.tileLayer = 0;
    rcVcopy(params.bmin, _pmesh->bmin);
    rcVcopy(params.bmax, _pmesh->bmax);
    params.cs = _cfg.cs;
    params.ch = _cfg.ch;
    params.buildBvTree = false;

    if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
    {
      navigation_cat.error() << "Could not build Detour navmesh." << std::endl;
      return nullptr;
    }
  }

  dataSize = navDataSize;
  return navData;
}

/**
 * Function to find vertices and faces in a geom primitive.
 */
void process_primitive(std::set<TriVertGroup> &vert_tris, const GeomPrimitive *orig_prim, const GeomVertexData *vdata, LMatrix4 &transform) {

  GeomVertexReader vertex(vdata, "vertex");

  CPT(GeomPrimitive) prim = orig_prim->decompose();

  for (int k = 0; k < prim->get_num_primitives(); ++k) {

    int s = prim->get_primitive_start(k);
    int e = prim->get_primitive_end(k);

    LVector3 v1, v2, v3;
    if (e - s == 3) {
      int a = prim->get_vertex(s);
      vertex.set_row(a);
      v1 = vertex.get_data3();
      transform.xform_point_general_in_place(v1);

      int b = prim->get_vertex(s + 1);
      vertex.set_row(b);
      v2 = vertex.get_data3();
      transform.xform_point_general_in_place(v2);

      int c = prim->get_vertex(s + 2);
      vertex.set_row(c);
      v3 = vertex.get_data3();
      transform.xform_point_general_in_place(v3);

      vert_tris.emplace((TriVertGroup){v1, v2, v3});
    } else if (e - s > 3) {
      for (int i = s + 2; i < e; ++i) {
        int a = prim->get_vertex(s);
        vertex.set_row(a);
        v1 = vertex.get_data3();
        transform.xform_point_general_in_place(v1);

        int b = prim->get_vertex(i - 1);
        vertex.set_row(b);
        v2 = vertex.get_data3();
        transform.xform_point_general_in_place(v2);

        int c = prim->get_vertex(i);
        vertex.set_row(c);
        v3 = vertex.get_data3();
        transform.xform_point_general_in_place(v3);

        vert_tris.emplace((TriVertGroup){v1, v2, v3});
      }
    }
    else continue;
  }
}

/**
 * Function to process geom to find primitives.
 */
void process_geom(std::set<TriVertGroup> &vert_tris, CPT(Geom) &geom, const CPT(TransformState) &transform) {
  LMatrix4 mat_to_y = LMatrix4::convert_mat(CS_default, CS_yup_right);
  // Chain in the matrix to convert to y-up here.
  LMatrix4 transform_mat = transform->get_mat() * mat_to_y;

  CPT(GeomVertexData) vdata = geom->get_vertex_data();

  //process_vertex_data(vdata, transform);

  for (size_t i = 0; i < geom->get_num_primitives(); ++i) {
    CPT(GeomPrimitive) prim = geom->get_primitive(i);
    process_primitive(vert_tris, prim, vdata, transform_mat);
  }
}

/**
 * Function to process geom node to find geoms.
 */
void process_geom_node(std::set<TriVertGroup> &vert_tris, PT(GeomNode) &geomnode, CPT(TransformState) &transform) {
  for (int j = 0; j < geomnode->get_num_geoms(); ++j) {
    CPT(Geom) geom = geomnode->get_geom(j);
    process_geom(vert_tris, geom, transform);
  }
}

void process_node_path(std::set<TriVertGroup> &vert_tris, NodePath &node, CPT(TransformState) &transform) {
  // Do not process stashed nodes.
  if (node.is_stashed()) {
    return;
  }

  if (node.node()->is_of_type(GeomNode::get_class_type())) {
    PT(GeomNode) g = DCAST(GeomNode, node.node());
    process_geom_node(vert_tris, g, transform);
  }

  NodePathCollection children = node.get_children();
  for (int i=0; i<children.get_num_paths(); i++) {
    NodePath cnp = children.get_path(i);
    CPT(TransformState) child_transform = cnp.get_transform();
    CPT(TransformState) net_transform = transform->compose(child_transform);
    process_node_path(vert_tris, cnp, net_transform);
  }
}

void process_coll_node_path(std::set<TriVertGroup> &vert_tris, NodePath &node, CPT(TransformState) &transform, BitMask32 mask) {
  // Do not process stashed nodes.
  if (node.is_stashed()) {
    return;
  }

  if (node.node()->is_of_type(CollisionNode::get_class_type())) {
    PT(CollisionNode) g = DCAST(CollisionNode, node.node());
    if ((g->get_into_collide_mask() & mask) != 0) {
      for (int i = 0; i < g->get_num_solids(); i++) {
        PT(GeomNode) gn = g->get_solid(i)->get_viz();
        process_geom_node(vert_tris, gn, transform);
      }
    }
  }

  NodePathCollection children = node.get_children();
  for (int i=0; i<children.get_num_paths(); i++) {
    NodePath cnp = children.get_path(i);
    CPT(TransformState) child_transform = cnp.get_transform();
    CPT(TransformState) net_transform = transform->compose(child_transform);
    process_coll_node_path(vert_tris, cnp, net_transform, mask);
  }
}

void update_bounds(float *_mesh_bMin, float *_mesh_bMax, bool *_bounds_set, LVector3 vert) {
  if (!_bounds_set) {
    _mesh_bMin[0] = vert[0];
    _mesh_bMin[1] = vert[1];
    _mesh_bMin[2] = vert[2];
    _mesh_bMax[0] = vert[0];
    _mesh_bMax[1] = vert[1];
    _mesh_bMax[2] = vert[2];
    *_bounds_set = true;
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

void NavMesh::
update() {
  std::set<TriVertGroup> tri_vert_set;
  std::copy(_untracked_tris.begin(), _untracked_tris.end(), std::inserter(tri_vert_set, tri_vert_set.begin()));

  for (auto &node : _tracked_nodes) {
    CPT(TransformState) state = node.get_net_transform();
    process_node_path(tri_vert_set, node, state);
  }
  for (auto &node : _tracked_coll_nodes) {
    CPT(TransformState) state = node.node.get_net_transform();
    process_coll_node_path(tri_vert_set, node.node, state, node.mask);
  }

  float _mesh_bMin[3] = { 0, 0, 0 };
  float _mesh_bMax[3] = { 0, 0, 0 };
  bool bounds_set = false;
  for (const auto &tri_vert : tri_vert_set) {
    update_bounds(_mesh_bMin, _mesh_bMax, &bounds_set, tri_vert.a);
    update_bounds(_mesh_bMin, _mesh_bMax, &bounds_set, tri_vert.b);
    update_bounds(_mesh_bMin, _mesh_bMax, &bounds_set, tri_vert.c);
  }

  std::set<TriVertGroup> diff;

  std::set_symmetric_difference(tri_vert_set.begin(), tri_vert_set.end(), _last_tris.begin(), _last_tris.end(), std::inserter(diff, diff.begin()));

  std::set<std::pair<int, int>> affected_tiles;

  for (const TriVertGroup &changed_tri : diff) {
    affected_tiles.emplace(std::floor((changed_tri.a[0] - _params.orig_bound_min[0]) / _params.tile_cell_size),
                           std::floor((changed_tri.a[2] - _params.orig_bound_min[2]) / _params.tile_cell_size));
    affected_tiles.emplace(std::floor((changed_tri.b[0] - _params.orig_bound_min[0]) / _params.tile_cell_size),
                           std::floor((changed_tri.b[2] - _params.orig_bound_min[2]) / _params.tile_cell_size));
    affected_tiles.emplace(std::floor((changed_tri.c[0] - _params.orig_bound_min[0]) / _params.tile_cell_size),
                           std::floor((changed_tri.c[2] - _params.orig_bound_min[2]) / _params.tile_cell_size));
  }

  std::set<std::pair<int, int>> tiles_to_regen;

  // We also need to regenerate the adjacent tiles because they include data from this tile.
  for (auto &tile_coods : affected_tiles) {
    tiles_to_regen.emplace(tile_coods.first - 1, tile_coods.second - 1);
    tiles_to_regen.emplace(tile_coods.first - 1, tile_coods.second);
    tiles_to_regen.emplace(tile_coods.first, tile_coods.second - 1);
    tiles_to_regen.emplace(tile_coods.first, tile_coods.second);
    tiles_to_regen.emplace(tile_coods.first, tile_coods.second + 1);
    tiles_to_regen.emplace(tile_coods.first + 1, tile_coods.second);
    tiles_to_regen.emplace(tile_coods.first + 1, tile_coods.second + 1);
  }

  pvector<float> verts;
  pvector<int> tris;
  get_vert_tris(tri_vert_set, verts, tris);

  float tileBmin[3] = { 0, 0, 0 };
  float tileBmax[3] = { 0, 0, 0 };
  for (auto &tile_coods : tiles_to_regen) {
    tileBmin[0] = _params.orig_bound_min[0] + static_cast<float>(tile_coods.first) * _params.tile_cell_size;
    tileBmin[1] = _mesh_bMin[1];
    tileBmin[2] = _params.orig_bound_min[2] + static_cast<float>(tile_coods.second) * _params.tile_cell_size;

    tileBmax[0] = _params.orig_bound_min[0] + static_cast<float>(tile_coods.first+1) * _params.tile_cell_size;
    tileBmax[1] = _mesh_bMax[1];
    tileBmax[2] = _params.orig_bound_min[2] + static_cast<float>(tile_coods.second+1) * _params.tile_cell_size;

    // Remove any previous data (navmesh owns and deletes the data).
    _nav_mesh->removeTile(_nav_mesh->getTileRefAt(tile_coods.first, tile_coods.second,0), 0, 0);

    int dataSize = 0;
    unsigned char* data = buildTileMesh(tile_coods.first, tile_coods.second, tileBmin, tileBmax, dataSize, verts, tris);
    if (data)
    {
      // Let the navmesh own the data.
      dtStatus status = _nav_mesh->addTile(data,dataSize,DT_TILE_FREE_DATA,0,0);
      if (dtStatusFailed(status)) {
        dtFree(data);
        navigation_cat.error() << "buildTiledNavigation: Could not init navmesh." << std::endl;
      }
    }
  }

  // Clear debug mesh cache.
  _cache_poly_outlines = nullptr;
  _cache_poly_verts.clear();

  _last_tris = tri_vert_set;
}


/**
 * Tells the BamReader how to create objects of type NavMesh.
 */
void NavMesh::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void NavMesh::
write_datagram(BamWriter *manager, Datagram &dg) {
  //NavMesh::write_datagram(manager, dg);


  //border_index
  dg.add_int32(border_index);

}

/**
 * This function is called by the BamReader's factory when a new object of
 * type NavMesh is encountered in the Bam file.  It should create the
 * NavMesh and extract its information from the file.
 */
TypedWritable *NavMesh::
make_from_bam(const FactoryParams &params) {
  NavMesh *param = new NavMesh;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  param->fillin(scan, manager);

  return param;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new NavMesh.
 */
void NavMesh::
fillin(DatagramIterator &scan, BamReader *manager) {
  
  int vert_count = scan.get_int32();
  int poly_count = scan.get_int32();
  int nvp = scan.get_int32();
  int detail_vert_count = scan.get_int32();
  int detail_tri_count = scan.get_int32();
  float walkable_height = scan.get_float32();
  float walkable_radius = scan.get_float32();
  float walkable_climb = scan.get_float32();
  float cs = scan.get_float32();
  float ch = scan.get_float32();
  bool build_bv_tree = scan.get_bool();

  float b_min[3];
  for (float &f : b_min) {
    f = scan.get_float32();
  }
  float b_max[3];
  for (float &f : b_max) {
    f = scan.get_float32();
  }

  //POLYGON MESH ATTRIBUTES

  unsigned short *verts = new unsigned short[3 * vert_count];
  for (int i=0 ; i < 3 * vert_count ; i++) {
    verts[i] = scan.get_uint16();
  }

  unsigned short *polys = new unsigned short[poly_count * 2 * nvp];
  for (int i=0 ; i < poly_count * 2 * nvp ; i++) {
    polys[i] = scan.get_uint16();
  }

  unsigned short *poly_flags = new unsigned short[poly_count];
  for (int i=0 ; i < poly_count; i++) {
    poly_flags[i] = scan.get_uint16();
  }

  unsigned char *poly_areas = new unsigned char[poly_count];
  for (int i=0 ; i < poly_count; i++) {
    poly_areas[i] = scan.get_uint8();
  }

  //POLYGON MESH DETAIL ATTRIBUTES

  unsigned int *detail_meshes = new unsigned int[poly_count * 4];
  for (int i=0 ; i < poly_count * 4 ;i++) {
    detail_meshes[i] = scan.get_uint32();
  }

  float *detail_verts = new float[detail_vert_count * 3];
  for (int i=0 ; i < detail_vert_count * 3 ;i++) {
    detail_verts[i] = scan.get_float32();
  }

  unsigned char *detail_tris = new unsigned char[detail_tri_count * 4];
  for (int i=0 ; i < detail_tri_count * 4 ;i++) {
    detail_tris[i] = scan.get_uint8();
  }

  //border_index
  border_index = scan.get_int32();

  memset(&(_params), 0, sizeof(_params));

  init_nav_mesh();
}
