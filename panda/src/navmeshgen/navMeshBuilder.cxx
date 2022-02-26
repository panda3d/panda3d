/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshBuilder.cxx
 * @author ashwini
 * @date 2020-06-21
 */

#include "navMeshBuilder.h"
#include "recastnavigation/Recast.h"
#include "recastnavigation/DetourNavMesh.h"
#include "recastnavigation/DetourNavMeshBuilder.h"
#include "pta_LVecBase3.h"
#include "lvecBase3.h"
#include "config_navmeshgen.h"

#define _USE_MATH_DEFINES
#include <string>
#include "geom.h"
#include "geomVertexFormat.h"
#include "geomVertexWriter.h"
#include "geomVertexReader.h"
#include "geomTrifans.h"
#include "collisionNode.h"

#include "string_utils.h"


/**
 * NavMeshBuilder contructor which initiates the member variables
 */
NavMeshBuilder::NavMeshBuilder(NodePath parent) :
  _parent(parent),
  _filter_low_hanging_obstacles(true),
  _filter_ledge_spans(true),
  _filter_walkable_low_height_spans(true),
  _scale(1.0f) {
  index_temp = 0;
  _ctx = new rcContext;
  reset_common_settings();
}

NavMeshBuilder::~NavMeshBuilder() {
  cleanup();
}


/**
 * Function to add vertex to the vertex array.
 */
void NavMeshBuilder::add_vertex(float x, float y, float z) {
  _verts.emplace_back(x);
  _verts.emplace_back(y);
  _verts.emplace_back(z);
}

/**
 * Function to add triangles to the triangles array.
 */
void NavMeshBuilder::add_triangle(int a, int b, int c) {
  _tris.emplace_back(a);
  _tris.emplace_back(b);
  _tris.emplace_back(c);
}

/**
 * This function adds a custom polygon with three vertices to the input geometry.
 */
void NavMeshBuilder::add_polygon(LPoint3 a, LPoint3 b, LPoint3 c) {

  int v1, v2, v3;

  LVector3 v = {a[0], a[1], a[2]};
  if (_vertex_map.find(v) == _vertex_map.end()) {

    LVecBase3 vec = mat_to_y.xform_point(v);
    add_vertex(vec[0], vec[1], vec[2]);
  
    //add_vertex(v[0], v[2], -v[1], _vert_capacity); //if input model is originally z-up
    //add_vertex(v[0], v[1], v[2], _vert_capacity); //if input model is originally y-up
    _vertex_map[v] = index_temp++;
    _vertex_vector.push_back(v);
  }

  v1 = _vertex_map[v];

  v = {b[0], b[1], b[2]};
  if (_vertex_map.find(v) == _vertex_map.end()) {
    LVecBase3 vec = mat_to_y.xform_point(v);
    add_vertex(vec[0], vec[1], vec[2]);

    //add_vertex(v[0], v[2], -v[1], _vert_capacity); //if input model is originally z-up
    //add_vertex(v[0], v[1], v[2], _vert_capacity); //if input model is originally y-up
    _vertex_map[v] = index_temp++;
    _vertex_vector.push_back(v);
  }

  v2 = _vertex_map[v];

  v = {c[0], c[1], c[2]};
  if (_vertex_map.find(v) == _vertex_map.end()) {
    LVecBase3 vec = mat_to_y.xform_point(v);
    add_vertex(vec[0], vec[1], vec[2]);

    //add_vertex(v[0], v[2], -v[1], _vert_capacity); //if input model is originally z-up
    //add_vertex(v[0], v[1], v[2], _vert_capacity); //if input model is originally y-up
    _vertex_map[v] = index_temp++;
    _vertex_vector.push_back(v);
  }

  v3 = _vertex_map[v];

  add_triangle(v1, v2, v3);

}

/**
 * This function adds a custom polygon with equal to or more than three vertices to the input geometry.
 */
void NavMeshBuilder::add_polygon(PTA_LVecBase3f &vec) {
  for (int i = 2; i < vec.size(); ++i) {
    add_polygon(LPoint3(vec[i-2]), LPoint3(vec[i-1]), LPoint3(vec[i]));
  }
}

/**
 * Function to build vertex array and triangles array from a geom.
 */
bool NavMeshBuilder::from_geom(PT(Geom) geom) {
  CPT(Geom) const_geom = geom;
  process_geom(const_geom, TransformState::make_identity());
  _loaded = true;
  return true;
}

/**
 * Adds all visible geometry under the given node to the NavMesh.
 */
bool NavMeshBuilder::from_node_path(NodePath node) {
  CPT(TransformState) transform = node.get_transform(_parent);

  process_node_path(node, transform);

  for (auto &tri_verts : _tri_verticies) {
    add_triangle(_vertex_map[tri_verts.a], _vertex_map[tri_verts.b], _vertex_map[tri_verts.c]);
  }

  _loaded = true;
  return true;
}

/**
 * Adds all collision geometry under the given node to the NavMesh.
 */
bool NavMeshBuilder::from_coll_node_path(NodePath node) {
  CPT(TransformState) transform = node.get_transform(_parent);

  process_coll_node_path(node, transform);

  for (auto &tri_verts : _tri_verticies) {
    add_triangle(_vertex_map[tri_verts.a], _vertex_map[tri_verts.b], _vertex_map[tri_verts.c]);
  }

  _loaded = true;
  return true;
}

/**
 * Function to find vertices and faces in a geom primitive.
 */
void NavMeshBuilder::process_primitive(const GeomPrimitive *orig_prim, const GeomVertexData *vdata, CPT(TransformState) &transform) {

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
      transform->get_mat().xform_point_general_in_place(v1);

      int b = prim->get_vertex(s + 1);
      vertex.set_row(b);
      v2 = vertex.get_data3();
      transform->get_mat().xform_point_general_in_place(v2);

      int c = prim->get_vertex(s + 2);
      vertex.set_row(c);
      v3 = vertex.get_data3();
      transform->get_mat().xform_point_general_in_place(v3);

      _tri_verticies.emplace_back((TriVertGroup){v1, v2, v3});
    } else if (e - s > 3) {
      for (int i = s + 2; i < e; ++i) {
        int a = prim->get_vertex(s);
        vertex.set_row(a);
        v1 = vertex.get_data3();
        transform->get_mat().xform_point_general_in_place(v1);

        int b = prim->get_vertex(i - 1);
        vertex.set_row(b);
        v2 = vertex.get_data3();
        transform->get_mat().xform_point_general_in_place(v2);

        int c = prim->get_vertex(i);
        vertex.set_row(c);
        v3 = vertex.get_data3();
        transform->get_mat().xform_point_general_in_place(v3);

        _tri_verticies.emplace_back((TriVertGroup){v1, v2, v3});
      }
    }
    else continue;

  }
  
}

/**
 * Function to find vertices from GeomVertexData.
 */
void NavMeshBuilder::process_vertex_data(const GeomVertexData *vdata, CPT(TransformState) &transform) {
  GeomVertexReader vertex(vdata, "vertex");

  while (!vertex.is_at_end()) {

    LVector3 v = vertex.get_data3();
    transform->get_mat().xform_point_general_in_place(v);
    if (_vertex_map.find(v) == _vertex_map.end()) {
      LVecBase3 vec = mat_to_y.xform_point(v);
      add_vertex(vec[0], vec[1], vec[2]);

      _vertex_map[v] = index_temp++;
      _vertex_vector.push_back(v);
    }
  }
}

/**
 * Function to process geom to find primitives.
 */
void NavMeshBuilder::process_geom(CPT(Geom) &geom, CPT(TransformState) transform) {

  CPT(GeomVertexData) vdata = geom->get_vertex_data();

  process_vertex_data(vdata, transform);

  for (size_t i = 0; i < geom->get_num_primitives(); ++i) {
    CPT(GeomPrimitive) prim = geom->get_primitive(i);
    process_primitive(prim, vdata, transform);
  }
}

/**
 * Function to process geom node to find geoms.
 */
void NavMeshBuilder::process_geom_node(PT(GeomNode) &geomnode, CPT(TransformState) &transform) {
  for (int j = 0; j < geomnode->get_num_geoms(); ++j) {
    CPT(Geom) geom = geomnode->get_geom(j);
    process_geom(geom, transform);
  }
}

void NavMeshBuilder::process_node_path(NodePath &node, CPT(TransformState) &transform) {
  if (node.node()->is_of_type(GeomNode::get_class_type())) {
    PT(GeomNode) g = DCAST(GeomNode, node.node());
    process_geom_node(g, transform);
  }

  NodePathCollection children = node.get_children();
  for (int i=0; i<children.get_num_paths(); i++) {
    NodePath cnp = children.get_path(i);
    CPT(TransformState) child_transform = cnp.get_transform();
    CPT(TransformState) net_transform = transform->compose(child_transform);
    process_node_path(cnp, net_transform);
  }
}

void NavMeshBuilder::process_coll_node_path(NodePath &node, CPT(TransformState) &transform) {
  if (node.node()->is_of_type(CollisionNode::get_class_type())) {
    PT(CollisionNode) g = DCAST(CollisionNode, node.node());
    for (int i = 0; i < g->get_num_solids(); i++) {
      PT(GeomNode) gn = g->get_solid(i)->get_viz();
      process_geom_node(gn, transform);
    }
  }

  NodePathCollection children = node.get_children();
  for (int i=0; i<children.get_num_paths(); i++) {
    NodePath cnp = children.get_path(i);
    CPT(TransformState) child_transform = cnp.get_transform();
    CPT(TransformState) net_transform = transform->compose(child_transform);
    process_coll_node_path(cnp, net_transform);
  }
}

/**
 *
 */
void NavMeshBuilder::cleanup()
{
  _triareas = nullptr;
  _solid = nullptr;
  _chf = nullptr;
  _cset = nullptr;
  _pmesh = nullptr;
  _dmesh = nullptr;
}

/**
 * Function to reset common settings to default values.
 */
void NavMeshBuilder::reset_common_settings()
{
  _cell_size = 0.3f;
  _cell_height = 0.2f;
  _agent_height = 2.0f;
  _agent_radius = 0.6f;
  _agent_max_climb = 0.9f;
  _agent_max_slope = 45.0f;
  _region_min_size = 8;
  _region_merge_size = 20;
  _edge_max_len = 12.0f;
  _edge_max_error = 1.3f;
  _verts_per_poly = 6.0f;
  _detail_sample_dist = 6.0f;
  _detail_sample_max_error = 1.0f;
  _partition_type = SAMPLE_PARTITION_WATERSHED;
}

static const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'MSET';
static const int NAVMESHSET_VERSION = 1;

/**
 * Function to build the navigation mesh from the vertex array, triangles array
 * and on the basis of the settings such as actor radius, actor height, max climb etc.
 */
PT(NavMesh) NavMeshBuilder::build() {
  if (!loaded_geom()) {

    _ctx->log(RC_LOG_ERROR, "build(): Input mesh is not specified.");
    navmeshgen_cat.error() << "build(): Input mesh is not specified." << std::endl;

    return _nav_mesh_obj;
  }

  cleanup();

  rcCalcBounds(_verts.data(), get_vert_count(), _mesh_bMin, _mesh_bMax);

  const float *bmin = _mesh_bMin;
  const float *bmax = _mesh_bMax;
  navmeshgen_cat.info() << "BMIN: " << bmin[0] << " " << bmin[1] << std::endl;
  navmeshgen_cat.info() << "BMAX: " << bmax[0] << " " << bmax[1] << std::endl;
  navmeshgen_cat.info() << "vert count: " << get_vert_count() << std::endl;

  //
  // Step 1. Initialize build config.
  //
  // Init build configuration from GUI
  memset(&_cfg, 0, sizeof(_cfg));
  _cfg.cs = _cell_size;
  _cfg.ch = _cell_height;
  _cfg.walkableSlopeAngle = _agent_max_slope;
  _cfg.walkableHeight = (int)ceilf(_agent_height / _cfg.ch);
  _cfg.walkableClimb = (int)floorf(_agent_max_climb / _cfg.ch);
  _cfg.walkableRadius = (int)ceilf(_agent_radius / _cfg.cs);
  _cfg.maxEdgeLen = (int)(_edge_max_len / _cell_size);
  _cfg.maxSimplificationError = _edge_max_error;
  _cfg.minRegionArea = (int)rcSqr(_region_min_size);    // Note: area = size*size
  _cfg.mergeRegionArea = (int)rcSqr(_region_merge_size);  // Note: area = size*size
  _cfg.maxVertsPerPoly = (int)_verts_per_poly;
  _cfg.detailSampleDist = _detail_sample_dist < 0.9f ? 0 : _cell_size * _detail_sample_dist;
  _cfg.detailSampleMaxError = _cell_height * _detail_sample_max_error;

  // Set the area where the navigation will be build.
  // Here the bounds of the input mesh are used, but the
  // area could be specified by an user defined box, etc.
  rcVcopy(_cfg.bmin, bmin);
  rcVcopy(_cfg.bmax, bmax);
  rcCalcGridSize(_cfg.bmin, _cfg.bmax, _cfg.cs, &_cfg.width, &_cfg.height);

  // Reset build times gathering.
  _ctx->resetTimers();

  // Start the build process. 
  _ctx->startTimer(RC_TIMER_TOTAL);

  _ctx->log(RC_LOG_PROGRESS, "Building navigation mesh:");
  navmeshgen_cat.info() << "Building navigation mesh:" << std::endl;
  _ctx->log(RC_LOG_PROGRESS, " - %d x %d cells", _cfg.width, _cfg.height);
  navmeshgen_cat.info() << " - " << _cfg.width << " x " << _cfg.height << " cells" << std::endl;
  _ctx->log(RC_LOG_PROGRESS, " - %.1fK verts, %.1fK tris", get_vert_count() / 1000.0, get_tri_count() / 1000.0);
  navmeshgen_cat.info() << " - " << get_vert_count() / 1000.0 << "K vetrs, " << get_tri_count() / 1000.0 << "K tris" << std::endl;

  //
  // Step 2. Rasterize input polygon soup.
  //

  // Allocate voxel heightfield where we rasterize our input data to.
  _solid = rcAllocHeightfield();
  if (!_solid) {
    _ctx->log(RC_LOG_ERROR, "build(): Out of memory 'solid'.");
    navmeshgen_cat.error() << "build(): Out of memory 'solid'." << std::endl;
    return _nav_mesh_obj;
  }
  if (!rcCreateHeightfield(_ctx, *_solid, _cfg.width, _cfg.height, _cfg.bmin, _cfg.bmax, _cfg.cs, _cfg.ch)) {
    _ctx->log(RC_LOG_ERROR, "build(): Could not create solid heightfield.");
    navmeshgen_cat.error() << "build(): Could not create solid heightfield." << std::endl;
    return _nav_mesh_obj;
  }

  // Allocate array that can hold triangle area types.
  // If you have multiple meshes you need to process, allocate
  // an array which can hold the max number of triangles you need to process.
  _triareas = new unsigned char[get_tri_count()];
  if (!_triareas) {
    _ctx->log(RC_LOG_ERROR, "build(): Out of memory '_triareas' (%d).", get_tri_count());
    navmeshgen_cat.error() << "build(): Out of memory '_triareas' (" << get_tri_count() << ")." << std::endl;
    return _nav_mesh_obj;
  }

  // Find triangles which are walkable based on their slope and rasterize them.
  // If your input data is multiple meshes, you can transform them here, calculate
  // the are type for each of the meshes and rasterize them.
  memset(_triareas, 0, get_tri_count() * sizeof(unsigned char));
  
  rcMarkWalkableTriangles(_ctx, _cfg.walkableSlopeAngle, _verts.data(), get_vert_count(), _tris.data(), get_tri_count(), _triareas);

  if (!rcRasterizeTriangles(_ctx, _verts.data(), get_vert_count(), _tris.data(), _triareas, get_tri_count(), *_solid, _cfg.walkableClimb)) {
    _ctx->log(RC_LOG_ERROR, "build(): Could not rasterize triangles.");
    navmeshgen_cat.error() << "build(): Could not rasterize triangles." << std::endl;
    return _nav_mesh_obj;
  }


  //
  // Step 3. Filter walkables surfaces.
  //

  // Once all geoemtry is rasterized, we do initial pass of filtering to
  // remove unwanted overhangs caused by the conservative rasterization
  // as well as filter spans where the character cannot possibly stand.
  if (_filter_low_hanging_obstacles)
    rcFilterLowHangingWalkableObstacles(_ctx, _cfg.walkableClimb, *_solid);
  if (_filter_ledge_spans)
    rcFilterLedgeSpans(_ctx, _cfg.walkableHeight, _cfg.walkableClimb, *_solid);
  if (_filter_walkable_low_height_spans)
    rcFilterWalkableLowHeightSpans(_ctx, _cfg.walkableHeight, *_solid);


  //
  // Step 4. Partition walkable surface to simple regions.
  //

  // Compact the heightfield so that it is faster to handle from now on.
  // This will result more cache coherent data as well as the neighbours
  // between walkable cells will be calculated.
  _chf = rcAllocCompactHeightfield();
  if (!_chf) {
    _ctx->log(RC_LOG_ERROR, "build(): Out of memory 'chf'.");
    navmeshgen_cat.error() << "build(): Out of memory 'chf'." << std::endl;
    return _nav_mesh_obj;
  }
  if (!rcBuildCompactHeightfield(_ctx, _cfg.walkableHeight, _cfg.walkableClimb, *_solid, *_chf)) {
    _ctx->log(RC_LOG_ERROR, "build(): Could not build compact data.");
    navmeshgen_cat.error() << "build(): Could not build compact data." << std::endl;
    return _nav_mesh_obj;
  }


  // Erode the walkable area by agent radius.
  if (!rcErodeWalkableArea(_ctx, _cfg.walkableRadius, *_chf)) {
    _ctx->log(RC_LOG_ERROR, "build(): Could not erode.");
    navmeshgen_cat.error() << "build(): Could not erode." << std::endl;
    return _nav_mesh_obj;
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

  switch (_partition_type) {
    case SAMPLE_PARTITION_WATERSHED:
      // Prepare for region partitioning, by calculating distance field along the walkable surface.
      if (!rcBuildDistanceField(_ctx, *_chf)) {
        _ctx->log(RC_LOG_ERROR, "build(): Could not build distance field.");
        navmeshgen_cat.error() << "build(): Could not build distance field." << std::endl;
        return _nav_mesh_obj;
      }

      // Partition the walkable surface into simple regions without holes.
      if (!rcBuildRegions(_ctx, *_chf, 0, _cfg.minRegionArea, _cfg.mergeRegionArea)) {
        _ctx->log(RC_LOG_ERROR, "build(): Could not build watershed regions.");
        navmeshgen_cat.error() << "build(): Could not build watershed regions." << std::endl;
        return _nav_mesh_obj;
      }
      break;
    case SAMPLE_PARTITION_MONOTONE:
      // Partition the walkable surface into simple regions without holes.
      // Monotone partitioning does not need distancefield.
      if (!rcBuildRegionsMonotone(_ctx, *_chf, 0, _cfg.minRegionArea, _cfg.mergeRegionArea)) {
        _ctx->log(RC_LOG_ERROR, "build(): Could not build monotone regions.");
        navmeshgen_cat.error() << "build(): Could not build monotone regions." << std::endl;
        return _nav_mesh_obj;
      }
      break;
    case SAMPLE_PARTITION_LAYERS:
      // Partition the walkable surface into simple regions without holes.
      if (!rcBuildLayerRegions(_ctx, *_chf, 0, _cfg.minRegionArea)) {
        _ctx->log(RC_LOG_ERROR, "build(): Could not build layer regions.");
        navmeshgen_cat.error() << "build(): Could not build layer regions." << std::endl;
        return _nav_mesh_obj;
      }
      break;
  }

  //
  // Step 5. Trace and simplify region contours.
  //

  // Create contours.
  _cset = rcAllocContourSet();
  if (!_cset) {
    _ctx->log(RC_LOG_ERROR, "build(): Out of memory 'cset'.");
    navmeshgen_cat.error() << "build(): Out of memory 'cset'." << std::endl;
    return _nav_mesh_obj;
  }
  if (!rcBuildContours(_ctx, *_chf, _cfg.maxSimplificationError, _cfg.maxEdgeLen, *_cset)) {
    _ctx->log(RC_LOG_ERROR, "build(): Could not create contours.");
    navmeshgen_cat.error() << "build(): Could not create contours." << std::endl;
    return _nav_mesh_obj;
  }

  //
  // Step 6. Build polygons mesh from contours.
  //

  // Build polygon navmesh from the contours.
  _pmesh = rcAllocPolyMesh();
  if (!_pmesh) {
    _ctx->log(RC_LOG_ERROR, "build(): Out of memory 'pmesh'.");
    navmeshgen_cat.error() << "build(): Out of memory 'pmesh'." << std::endl;
    return _nav_mesh_obj;
  }
  if (!rcBuildPolyMesh(_ctx, *_cset, _cfg.maxVertsPerPoly, *_pmesh)) {
    _ctx->log(RC_LOG_ERROR, "build(): Could not triangulate contours.");
    navmeshgen_cat.error() << "build(): Could not triangulate contours." << std::endl;
    return _nav_mesh_obj;
  }

  //
  // Step 7. Create detail mesh which allows to access approximate height on each polygon.
  //

  _dmesh = rcAllocPolyMeshDetail();
  if (!_dmesh) {
    _ctx->log(RC_LOG_ERROR, "build(): Out of memory 'pmdtl'.");
    navmeshgen_cat.error() << "build(): Out of memory 'pmdt1'." << std::endl;
    return _nav_mesh_obj;
  }

  if (!rcBuildPolyMeshDetail(_ctx, *_pmesh, *_chf, _cfg.detailSampleDist, _cfg.detailSampleMaxError, *_dmesh)) {
    _ctx->log(RC_LOG_ERROR, "build(): Could not build detail mesh.");
    navmeshgen_cat.error() << "build(): Could not build detail mesh." << std::endl;
    return _nav_mesh_obj;
  }
  navmeshgen_cat.info() << "Number of vertices: " << _pmesh->nverts << std::endl;
  navmeshgen_cat.info() << "Number of polygons: " << _pmesh->npolys << std::endl;
  navmeshgen_cat.info() << "Number of allocated polygons: " << _pmesh->maxpolys << std::endl;

  // At this point the navigation mesh data is ready, you can access it from _pmesh.
  // See duDebugDrawPolyMesh or dtCreateNavMeshData as examples how to access the data.


  //
  // Step 8. Create Detour data from Recast poly mesh.
  //
  // The GUI may allow more max points per polygon than Detour can handle.
  // Only build the detour navmesh if we do not exceed the limit.
  if (_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON) {
    

    // Update poly flags from areas.
    for (int i = 0; i < _pmesh->npolys; ++i) {
      if (_pmesh->areas[i] == RC_WALKABLE_AREA)
        _pmesh->areas[i] = SAMPLE_POLYAREA_GROUND;

      if (_pmesh->areas[i] == SAMPLE_POLYAREA_GROUND || _pmesh->areas[i] == SAMPLE_POLYAREA_GRASS || _pmesh->areas[i] == SAMPLE_POLYAREA_ROAD) {
        _pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK;
      }
      else if (_pmesh->areas[i] == SAMPLE_POLYAREA_WATER) {
        _pmesh->flags[i] = SAMPLE_POLYFLAGS_SWIM;
      }
      else if (_pmesh->areas[i] == SAMPLE_POLYAREA_DOOR) {
        _pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK | SAMPLE_POLYFLAGS_DOOR;
      }
    }
    
    NavMeshParams mesh_params;

    mesh_params.verts = _pmesh->verts;
    mesh_params.vert_count = _pmesh->nverts;
    mesh_params.polys = _pmesh->polys;
    mesh_params.poly_areas = _pmesh->areas;
    mesh_params.poly_flags = _pmesh->flags;
    mesh_params.poly_count = _pmesh->npolys;
    mesh_params.nvp = _pmesh->nvp;
    mesh_params.detail_meshes = _dmesh->meshes;
    mesh_params.detail_verts = _dmesh->verts;
    mesh_params.detail_vert_count = _dmesh->nverts;
    mesh_params.detail_tris = _dmesh->tris;
    mesh_params.detail_tri_count = _dmesh->ntris;

    mesh_params.walkable_height = _agent_height;
    mesh_params.walkable_radius = _agent_radius;
    mesh_params.walkable_climb = _agent_max_climb;
    rcVcopy(mesh_params.b_min, _pmesh->bmin);
    rcVcopy(mesh_params.b_max, _pmesh->bmax);
    mesh_params.cs = _cfg.cs;
    mesh_params.ch = _cfg.ch;
    mesh_params.build_bv_tree = true;

    mesh_params.border_index = RC_MESH_NULL_IDX;
    
    _nav_mesh_obj = new NavMesh(mesh_params);

  }

  _ctx->stopTimer(RC_TIMER_TOTAL);

  // Show performance stats.
  _ctx->log(RC_LOG_PROGRESS, ">> Polymesh: %d vertices  %d polygons", _pmesh->nverts, _pmesh->npolys);

  _total_build_time_ms = _ctx->getAccumulatedTime(RC_TIMER_TOTAL) / 1000.0f;

  return _nav_mesh_obj;
}

/**
 * Function to return geomnode for the navigation mesh.
 */
PT(GeomNode) NavMeshBuilder::draw_poly_mesh_geom() {

  PT(GeomVertexData) vdata;
  vdata = new GeomVertexData("vertexInfo", GeomVertexFormat::get_v3c4(), Geom::UH_static);
  vdata->set_num_rows(_pmesh->nverts);

  GeomVertexWriter vertex(vdata, "vertex");
  GeomVertexWriter colour(vdata, "color");

  const int nvp = _pmesh->nvp;
  const float cs = _pmesh->cs;
  const float ch = _pmesh->ch;
  const float* orig = _pmesh->bmin;
  
  for (int i = 0;i < _pmesh->nverts * 3;i += 3) {

    const unsigned short* v = &_pmesh->verts[i];

    //convert to world space
    const float x = orig[0] + v[0] * cs;
    const float y = orig[1] + v[1] * ch;
    const float z = orig[2] + v[2] * cs;
    
    LVecBase3 vec = mat_from_y.xform_point({x, y, z});
    vertex.add_data3(vec);

    //vertex.add_data3(x, -z, y); //if origingally model is z-up
    //vertex.add_data3(x, y, z); //if originally model is y-up
    colour.add_data4((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1);
    
  }

  PT(GeomNode) node;
  node = new GeomNode("gnode");

  PT(GeomTrifans) prim;
  prim = new GeomTrifans(Geom::UH_static);

  for (int i = 0; i < _pmesh->npolys; ++i) {
    const unsigned short* p = &_pmesh->polys[i*nvp * 2];

    // Iterate the vertices.
    //unsigned short vi[3];  // The vertex indices.
    for (int j = 0; j < nvp; ++j) {
      if (p[j] == RC_MESH_NULL_IDX) {
        break;// End of vertices.
      }
      prim->add_vertex(p[j]);// The edge beginning with this vertex is a solid border.
    }
    prim->close_primitive();

  }
  PT(Geom) polymeshgeom;
  polymeshgeom = new Geom(vdata);
  polymeshgeom->add_primitive(prim);

  node->add_geom(polymeshgeom);
  navmeshgen_cat.info() << "Number of Polygons: " << _pmesh->npolys << std::endl;
  return node;
}
