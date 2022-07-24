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
#include "config_navigation.h"
#include "navMesh.h"

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
  _parent(parent) {
  index_temp = 0;
  _ctx = new rcContext;
}

NavMeshBuilder::NavMeshBuilder(PT(NavMesh) navMesh) :
  _nav_mesh_obj(navMesh),
  _params(navMesh->get_params()) {
  index_temp = 0;
  _ctx = new rcContext;
}

NavMeshBuilder::~NavMeshBuilder() {
  cleanup();
}

/**
 * This function adds a custom polygon with three vertices to the input geometry.
 */
void NavMeshBuilder::add_polygon(LPoint3 a, LPoint3 b, LPoint3 c) {
  mat_to_y.xform_point_general_in_place(a);
  update_bounds(a);

  mat_to_y.xform_point_general_in_place(b);
  update_bounds(b);

  mat_to_y.xform_point_general_in_place(c);
  update_bounds(c);

  _untracked_tris.insert((NavTriVertGroup){a, b, c});
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
 * Adds a custom Geom to the NavMesh.
 */
bool NavMeshBuilder::from_geom(PT(Geom) geom) {
  CPT(Geom) const_geom = geom;
  process_geom(_untracked_tris, const_geom, TransformState::make_identity());
  _loaded = true;
  return true;
}

/**
 * Adds all visible geometry under the given node to the NavMesh.
 */
bool NavMeshBuilder::from_node_path(NodePath node) {
  CPT(TransformState) transform = node.get_transform(_parent);

  process_node_path(_untracked_tris, node, transform);

  _loaded = true;
  return true;
}

/**
 * Adds all collision geometry under the given node to the NavMesh.
 */
bool NavMeshBuilder::from_coll_node_path(NodePath node, BitMask32 mask) {
  CPT(TransformState) transform = node.get_transform(_parent);

  process_coll_node_path(_untracked_tris, node, transform, mask);

  _loaded = true;
  return true;
}

/**
 * Function to find vertices and faces in a geom primitive.
 */
void NavMeshBuilder::process_primitive(std::set<NavTriVertGroup> &tris, const GeomPrimitive *orig_prim, const GeomVertexData *vdata, LMatrix4 &transform) {

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
      update_bounds(v1);

      int b = prim->get_vertex(s + 1);
      vertex.set_row(b);
      v2 = vertex.get_data3();
      transform.xform_point_general_in_place(v2);
      update_bounds(v2);

      int c = prim->get_vertex(s + 2);
      vertex.set_row(c);
      v3 = vertex.get_data3();
      transform.xform_point_general_in_place(v3);
      update_bounds(v3);

      tris.insert((NavTriVertGroup){v1, v2, v3});
    } else if (e - s > 3) {
      for (int i = s + 2; i < e; ++i) {
        int a = prim->get_vertex(s);
        vertex.set_row(a);
        v1 = vertex.get_data3();
        transform.xform_point_general_in_place(v1);
        update_bounds(v1);

        int b = prim->get_vertex(i - 1);
        vertex.set_row(b);
        v2 = vertex.get_data3();
        transform.xform_point_general_in_place(v2);
        update_bounds(v2);

        int c = prim->get_vertex(i);
        vertex.set_row(c);
        v3 = vertex.get_data3();
        transform.xform_point_general_in_place(v3);
        update_bounds(v3);

        tris.insert((NavTriVertGroup){v1, v2, v3});
      }
    }
    else continue;
  }
}

/**
 * Function to process geom to find primitives.
 */
void NavMeshBuilder::process_geom(std::set<NavTriVertGroup> &tris, CPT(Geom) &geom, const CPT(TransformState) &transform) {
  // Chain in the matrix to convert to y-up here.
  LMatrix4 transform_mat = transform->get_mat() * mat_to_y;

  CPT(GeomVertexData) vdata = geom->get_vertex_data();

  //process_vertex_data(vdata, transform);

  for (size_t i = 0; i < geom->get_num_primitives(); ++i) {
    CPT(GeomPrimitive) prim = geom->get_primitive(i);
    process_primitive(tris, prim, vdata, transform_mat);
  }
}

/**
 * Function to process geom node to find geoms.
 */
void NavMeshBuilder::process_geom_node(std::set<NavTriVertGroup> &tris, PT(GeomNode) &geomnode, CPT(TransformState) &transform) {
  for (int j = 0; j < geomnode->get_num_geoms(); ++j) {
    CPT(Geom) geom = geomnode->get_geom(j);
    process_geom(tris, geom, transform);
  }
}

void NavMeshBuilder::process_node_path(std::set<NavTriVertGroup> &tris, const NodePath &node, CPT(TransformState) &transform) {
  // Do not process stashed nodes.
  if (node.is_stashed()) {
    return;
  }

  if (node.node()->is_of_type(GeomNode::get_class_type())) {
    PT(GeomNode) g = DCAST(GeomNode, node.node());
    process_geom_node(tris, g, transform);
  }

  NodePathCollection children = node.get_children();
  for (int i=0; i<children.get_num_paths(); i++) {
    NodePath cnp = children.get_path(i);
    CPT(TransformState) child_transform = cnp.get_transform();
    CPT(TransformState) net_transform = transform->compose(child_transform);
    process_node_path(tris, cnp, net_transform);
  }
}

void NavMeshBuilder::process_coll_node_path(std::set<NavTriVertGroup> &tris, const NodePath &node, CPT(TransformState) &transform, BitMask32 mask) {
  // Do not process stashed nodes.
  if (node.is_stashed()) {
    return;
  }

  if (node.node()->is_of_type(CollisionNode::get_class_type())) {
    PT(CollisionNode) g = DCAST(CollisionNode, node.node());
    if ((g->get_into_collide_mask() & mask) != 0) {
      for (int i = 0; i < g->get_num_solids(); i++) {
        PT(GeomNode) gn = g->get_solid(i)->get_viz();
        process_geom_node(tris, gn, transform);
      }
    }
  }

  NodePathCollection children = node.get_children();
  for (int i=0; i<children.get_num_paths(); i++) {
    NodePath cnp = children.get_path(i);
    CPT(TransformState) child_transform = cnp.get_transform();
    CPT(TransformState) net_transform = transform->compose(child_transform);
    process_coll_node_path(tris, cnp, net_transform, mask);
  }
}

/**
 *
 */
void NavMeshBuilder::cleanup()
{
  _triareas.clear();
  rcFreeHeightField(_solid);
  _solid = nullptr;
  rcFreeCompactHeightfield(_chf);
  _chf = nullptr;
  rcFreeContourSet(_cset);
  _cset = nullptr;
  rcFreePolyMesh(_pmesh);
  _pmesh = nullptr;
  rcFreePolyMeshDetail(_dmesh);
  _dmesh = nullptr;
}

static const int NAVMESHSET_MAGIC = 'M' << 24 | 'S' << 16 | 'E' << 8 | 'T'; //'MSET';
static const int NAVMESHSET_VERSION = 1;


/**
 * Function to build the navigation mesh from the vertex array, triangles array
 * and on the basis of the settings such as actor radius, actor height, max climb etc.
 */
PT(NavMesh) NavMeshBuilder::build() {
  navigation_cat.info() << "BMIN: " << _mesh_bMin[0] << " " << _mesh_bMin[1] << std::endl;
  navigation_cat.info() << "BMAX: " << _mesh_bMax[0] << " " << _mesh_bMax[1] << std::endl;

  int gw = 0, gh = 0;
  rcCalcGridSize(_mesh_bMin, _mesh_bMax, _params.get_cell_size(), &gw, &gh);
  const int ts = (int)_params.get_tile_size();
  const int tw = (gw + ts-1) / ts;
  const int th = (gh + ts-1) / ts;
  const float tcs = _params.get_tile_size()*_params.get_cell_size();

  float tileBmin[3] = { 0, 0, 0 };
  float tileBmax[3] = { 0, 0, 0 };

  dtNavMesh *navMesh = dtAllocNavMesh();
  if (!navMesh)
  {
    navigation_cat.error() << "buildTiledNavigation: Could not allocate navmesh." << std::endl;
    return _nav_mesh_obj;
  }

  dtNavMeshParams params = {};
  rcVcopy(params.orig, _mesh_bMin);
  params.tileWidth = _params.get_tile_size()*_params.get_cell_size();
  params.tileHeight = _params.get_tile_size()*_params.get_cell_size();
  params.maxTiles = _params.get_max_tiles();
  params.maxPolys = _params.get_max_polys_per_tile();

  dtStatus status;

  status = navMesh->init(&params);
  if (dtStatusFailed(status))
  {
    navigation_cat.error() << "buildTiledNavigation: Could not init navmesh." << std::endl;
    return _nav_mesh_obj;
  }

  pvector<float> verts;
  pvector<int> tris;
  get_vert_tris(_untracked_tris, verts, tris);

  for (int y = 0; y < th; ++y) {
    for (int x = 0; x < tw; ++x) {
      tileBmin[0] = _mesh_bMin[0] + x*tcs;
      tileBmin[1] = _mesh_bMin[1];
      tileBmin[2] = _mesh_bMin[2] + y*tcs;

      tileBmax[0] = _mesh_bMin[0] + (x+1)*tcs;
      tileBmax[1] = _mesh_bMax[1];
      tileBmax[2] = _mesh_bMin[2] + (y+1)*tcs;

      int dataSize = 0;
      unsigned char* data = buildTileMesh(x, y, tileBmin, tileBmax, dataSize, verts, tris);
      if (data)
      {
        // Remove any previous data (navmesh owns and deletes the data).
        navMesh->removeTile(navMesh->getTileRefAt(x, y,0), 0, 0);
        // Let the navmesh own the data.
        dtStatus status = navMesh->addTile(data,dataSize,DT_TILE_FREE_DATA,0,0);
        if (dtStatusFailed(status)) {
          dtFree(data);
          navigation_cat.error() << "buildTiledNavigation: Could not init navmesh." << std::endl;
        }
      }
    }
  }

  _nav_mesh_obj = new NavMesh(navMesh, _params, _untracked_tris);
  return _nav_mesh_obj;
}

void NavMeshBuilder::
update_nav_mesh() {
  std::set<NavTriVertGroup> tri_vert_set;

  // Start with the untracked tris.
  auto untracked = _nav_mesh_obj->get_untracked_tris();
  std::copy(untracked.begin(), untracked.end(), std::inserter(tri_vert_set, tri_vert_set.begin()));

  // Set the bounds based on the untracked tris. Tracked nodes will be added as they are found.
  _bounds_set = false;
  for (const auto &tri_vert : tri_vert_set) {
    update_bounds(tri_vert.a);
    update_bounds(tri_vert.b);
    update_bounds(tri_vert.c);
  }

  // Add in the tracked nodes.
  for (auto &node : _nav_mesh_obj->get_tracked_nodes()) {
    CPT(TransformState) state = node.get_net_transform();
    process_node_path(tri_vert_set, node, state);
  }
  for (auto &node : _nav_mesh_obj->get_tracked_coll_nodes()) {
    CPT(TransformState) state = node.node.get_net_transform();
    process_coll_node_path(tri_vert_set, node.node, state, node.mask);
  }

  std::set<NavTriVertGroup> diff;

  std::set_symmetric_difference(tri_vert_set.begin(), tri_vert_set.end(), _last_tris.begin(), _last_tris.end(), std::inserter(diff, diff.begin()));

  std::set<std::pair<int, int>> affected_tiles;

  LPoint3 orig_bound_min = _params.get_orig_bound_min();

  for (const NavTriVertGroup &changed_tri : diff) {
    affected_tiles.emplace(std::floor((changed_tri.a[0] - orig_bound_min[0]) / _params.get_tile_cell_size()),
                           std::floor((changed_tri.a[2] - orig_bound_min[2]) / _params.get_tile_cell_size()));
    affected_tiles.emplace(std::floor((changed_tri.b[0] - orig_bound_min[0]) / _params.get_tile_cell_size()),
                           std::floor((changed_tri.b[2] - orig_bound_min[2]) / _params.get_tile_cell_size()));
    affected_tiles.emplace(std::floor((changed_tri.c[0] - orig_bound_min[0]) / _params.get_tile_cell_size()),
                           std::floor((changed_tri.c[2] - orig_bound_min[2]) / _params.get_tile_cell_size()));
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
    tileBmin[0] = orig_bound_min[0] + static_cast<float>(tile_coods.first) * _params.get_tile_cell_size();
    tileBmin[1] = _mesh_bMin[1];
    tileBmin[2] = orig_bound_min[2] + static_cast<float>(tile_coods.second) * _params.get_tile_cell_size();

    tileBmax[0] = orig_bound_min[0] + static_cast<float>(tile_coods.first+1) * _params.get_tile_cell_size();
    tileBmax[1] = _mesh_bMax[1];
    tileBmax[2] = orig_bound_min[2] + static_cast<float>(tile_coods.second+1) * _params.get_tile_cell_size();

    // Remove any previous data (navmesh owns and deletes the data).
    _nav_mesh_obj->get_nav_mesh()->removeTile(_nav_mesh_obj->get_nav_mesh()->getTileRefAt(tile_coods.first, tile_coods.second,0), 0, 0);

    int dataSize = 0;
    unsigned char* data = buildTileMesh(tile_coods.first, tile_coods.second, tileBmin, tileBmax, dataSize, verts, tris);
    if (data)
    {
      // Let the navmesh own the data.
      dtStatus status = _nav_mesh_obj->get_nav_mesh()->addTile(data,dataSize,DT_TILE_FREE_DATA, 0, 0);
      if (dtStatusFailed(status)) {
        dtFree(data);
        navigation_cat.error() << "buildTiledNavigation: Could not init navmesh." << std::endl;
      }
    }
  }

  _last_tris = tri_vert_set;
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
  
  for (int i = 0;i  < _pmesh->nverts * 3; i += 3) {

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
  navigation_cat.info() << "Number of Polygons: " << _pmesh->npolys << std::endl;
  return node;
}

unsigned char* NavMeshBuilder::buildTileMesh(const int tx, const int ty,
                                             const float* bmin, const float* bmax, int& dataSize,
                                             pvector<float> &verts, pvector<int> &tris)
{
  memset(&_cfg, 0, sizeof(_cfg));
  _cfg.cs = _params.get_cell_size();
  _cfg.ch = _params.get_cell_height();
  _cfg.walkableSlopeAngle = _params.get_actor_max_slope();
  _cfg.walkableHeight = (int)ceilf(_params.get_actor_height() / _cfg.ch);
  _cfg.walkableClimb = (int)floorf(_params.get_actor_max_climb() / _cfg.ch);
  _cfg.walkableRadius = (int)ceilf(_params.get_actor_radius() / _cfg.cs);
  _cfg.maxEdgeLen = (int)(_params.get_edge_max_len() / _params.get_cell_size());
  _cfg.maxSimplificationError = _params.get_edge_max_error();
  _cfg.minRegionArea = (int)rcSqr(_params.get_region_min_size());    // Note: area = size*size
  _cfg.mergeRegionArea = (int)rcSqr(_params.get_region_merge_size());  // Note: area = size*size
  _cfg.maxVertsPerPoly = (int)_params.get_verts_per_poly();
  _cfg.tileSize = (int)_params.get_tile_size();
  _cfg.borderSize = _cfg.walkableRadius + 3; // Reserve enough padding.
  _cfg.width = _cfg.tileSize + _cfg.borderSize*2;
  _cfg.height = _cfg.tileSize + _cfg.borderSize*2;
  _cfg.detailSampleDist = _params.get_detail_sample_dist() < 0.9f ? 0 : _params.get_cell_size() * _params.get_detail_sample_dist();
  _cfg.detailSampleMaxError = _params.get_cell_height() * _params.get_detail_sample_max_error();

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
  _solid = rcAllocHeightfield();
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
  _triareas.clear();
  _triareas.resize(static_cast<int>(tris.size() / 3), 0);

  // Find triangles which are walkable based on their slope and rasterize them.
  // If your input data is multiple meshes, you can transform them here, calculate
  // the are type for each of the meshes and rasterize them.
  rcMarkWalkableTriangles(_ctx, _cfg.walkableSlopeAngle, verts.data(), static_cast<int>(verts.size() / 3), tris.data(), static_cast<int>(tris.size() / 3), _triareas.data());

  if (!rcRasterizeTriangles(_ctx, verts.data(), static_cast<int>(verts.size() / 3), tris.data(), _triareas.data(), static_cast<int>(tris.size() / 3), *_solid, _cfg.walkableClimb)) {
    _ctx->log(RC_LOG_ERROR, "build(): Could not rasterize triangles.");
    navigation_cat.error() << "build(): Could not rasterize triangles." << std::endl;
    return nullptr;
  }


  //
  // Step 3. Filter walkables surfaces.
  //

  // Once all geoemtry is rasterized, we do initial pass of filtering to
  // remove unwanted overhangs caused by the conservative rasterization
  // as well as filter spans where the character cannot possibly stand.
  if (_params.get_filter_low_hanging_obstacles())
    rcFilterLowHangingWalkableObstacles(_ctx, _cfg.walkableClimb, *_solid);
  if (_params.get_filter_ledge_spans())
    rcFilterLedgeSpans(_ctx, _cfg.walkableHeight, _cfg.walkableClimb, *_solid);
  if (_params.get_filter_walkable_low_height_spans())
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

  switch (_params.get_partition_type()) {
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
  _cset = rcAllocContourSet();
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
  _pmesh = rcAllocPolyMesh();
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

  _dmesh = rcAllocPolyMeshDetail();
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
    params.walkableHeight = _params.get_actor_height();
    params.walkableRadius = _params.get_actor_radius();
    params.walkableClimb = _params.get_actor_max_climb();
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
