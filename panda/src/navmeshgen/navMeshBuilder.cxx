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
  process_geom(const_geom, TransformState::make_identity(), false);
  _loaded = true;
  return true;
}

/**
 * Adds all visible geometry under the given node to the NavMesh.
 */
bool NavMeshBuilder::from_node_path(NodePath node, bool tracked_node) {
  CPT(TransformState) transform = node.get_transform(_parent);

  process_node_path(node, transform, tracked_node);

  if (tracked_node) {
    _tracked_nodes.emplace_back(node);
  }

  _loaded = true;
  return true;
}

/**
 * Adds all collision geometry under the given node to the NavMesh.
 */
bool NavMeshBuilder::from_coll_node_path(NodePath node, BitMask32 mask, bool tracked_node) {
  CPT(TransformState) transform = node.get_transform(_parent);

  process_coll_node_path(node, transform, mask, tracked_node);

  if (tracked_node) {
    TrackedCollInfo info = { node, mask };
    _tracked_coll_nodes.emplace_back(info);
  }

  _loaded = true;
  return true;
}

/**
 * Function to find vertices and faces in a geom primitive.
 */
void NavMeshBuilder::process_primitive(const GeomPrimitive *orig_prim, const GeomVertexData *vdata, LMatrix4 &transform, bool tracked_node) {

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

      _tri_verticies.emplace_back((TriVertGroup){v1, v2, v3});
      if (!tracked_node) {
        _untracked_tris.emplace_back((TriVertGroup){v1, v2, v3});
      }
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

        _tri_verticies.emplace_back((TriVertGroup){v1, v2, v3});
        if (!tracked_node) {
          _untracked_tris.emplace_back((TriVertGroup){v1, v2, v3});
        }
      }
    }
    else continue;
  }
}

/**
 * Function to process geom to find primitives.
 */
void NavMeshBuilder::process_geom(CPT(Geom) &geom, const CPT(TransformState) &transform, bool tracked_node) {
  // Chain in the matrix to convert to y-up here.
  LMatrix4 transform_mat = transform->get_mat() * mat_to_y;

  CPT(GeomVertexData) vdata = geom->get_vertex_data();

  //process_vertex_data(vdata, transform);

  for (size_t i = 0; i < geom->get_num_primitives(); ++i) {
    CPT(GeomPrimitive) prim = geom->get_primitive(i);
    process_primitive(prim, vdata, transform_mat, tracked_node);
  }
}

/**
 * Function to process geom node to find geoms.
 */
void NavMeshBuilder::process_geom_node(PT(GeomNode) &geomnode, CPT(TransformState) &transform, bool tracked_node) {
  for (int j = 0; j < geomnode->get_num_geoms(); ++j) {
    CPT(Geom) geom = geomnode->get_geom(j);
    process_geom(geom, transform, tracked_node);
  }
}

void NavMeshBuilder::process_node_path(NodePath &node, CPT(TransformState) &transform, bool tracked_node) {
  // Do not process stashed nodes.
  if (node.is_stashed()) {
    return;
  }

  if (node.node()->is_of_type(GeomNode::get_class_type())) {
    PT(GeomNode) g = DCAST(GeomNode, node.node());
    process_geom_node(g, transform, tracked_node);
  }

  NodePathCollection children = node.get_children();
  for (int i=0; i<children.get_num_paths(); i++) {
    NodePath cnp = children.get_path(i);
    CPT(TransformState) child_transform = cnp.get_transform();
    CPT(TransformState) net_transform = transform->compose(child_transform);
    process_node_path(cnp, net_transform, tracked_node);
  }
}

void NavMeshBuilder::process_coll_node_path(NodePath &node, CPT(TransformState) &transform, BitMask32 mask, bool tracked_node) {
  // Do not process stashed nodes.
  if (node.is_stashed()) {
    return;
  }

  if (node.node()->is_of_type(CollisionNode::get_class_type())) {
    PT(CollisionNode) g = DCAST(CollisionNode, node.node());
    if ((g->get_into_collide_mask() & mask) != 0) {
      for (int i = 0; i < g->get_num_solids(); i++) {
        PT(GeomNode) gn = g->get_solid(i)->get_viz();
        process_geom_node(gn, transform, tracked_node);
      }
    }
  }

  NodePathCollection children = node.get_children();
  for (int i=0; i<children.get_num_paths(); i++) {
    NodePath cnp = children.get_path(i);
    CPT(TransformState) child_transform = cnp.get_transform();
    CPT(TransformState) net_transform = transform->compose(child_transform);
    process_coll_node_path(cnp, net_transform, mask, tracked_node);
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
  _partition_type = NavMeshParams::SAMPLE_PARTITION_LAYERS;
  _tile_size = 32;

  // Max tiles and max polys affect how the tile IDs are calculated.
  // There are 22 bits available for identifying a tile and a polygon.
  // We will default to the max recommended number of tile bits.
  int tileBits = 14;
  int polyBits = 22 - tileBits;
  _max_tiles = 1 << tileBits;
  _max_polys_per_tile = 1 << polyBits;
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

  navmeshgen_cat.info() << "BMIN: " << _mesh_bMin[0] << " " << _mesh_bMin[1] << std::endl;
  navmeshgen_cat.info() << "BMAX: " << _mesh_bMax[0] << " " << _mesh_bMax[1] << std::endl;
  navmeshgen_cat.info() << "vert count: " << get_vert_count() << std::endl;

  int gw = 0, gh = 0;
  rcCalcGridSize(_mesh_bMin, _mesh_bMax, _cell_size, &gw, &gh);
  const int ts = (int)_tile_size;
  const int tw = (gw + ts-1) / ts;
  const int th = (gh + ts-1) / ts;
  const float tcs = _tile_size*_cell_size;

  float tileBmin[3] = { 0, 0, 0 };
  float tileBmax[3] = { 0, 0, 0 };

  dtNavMesh *navMesh = dtAllocNavMesh();
  if (!navMesh)
  {
    navmeshgen_cat.error() << "buildTiledNavigation: Could not allocate navmesh." << std::endl;
    return _nav_mesh_obj;
  }

  dtNavMeshParams params = {};
  rcVcopy(params.orig, _mesh_bMin);
  params.tileWidth = _tile_size*_cell_size;
  params.tileHeight = _tile_size*_cell_size;
  params.maxTiles = _max_tiles;
  params.maxPolys = _max_polys_per_tile;

  dtStatus status;

  status = navMesh->init(&params);
  if (dtStatusFailed(status))
  {
    navmeshgen_cat.error() << "buildTiledNavigation: Could not init navmesh." << std::endl;
    return _nav_mesh_obj;
  }

  std::vector<float> verts;
  std::vector<int> tris;
  get_vert_tris(verts, tris);

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
          navmeshgen_cat.error() << "buildTiledNavigation: Could not init navmesh." << std::endl;
        }
      }
    }
  }

  NavMeshParams nav_params{};
  nav_params.cell_size = _cell_size;
  nav_params.cell_height = _cell_height;
  nav_params.tile_size = _tile_size;
  nav_params.max_tiles = _max_tiles;
  nav_params.max_polys_per_tile = _max_polys_per_tile;
  nav_params.agent_height = _agent_height;
  nav_params.agent_radius = _agent_radius;
  nav_params.agent_max_climb = _agent_max_climb;
  nav_params.agent_max_slope = _agent_max_slope;
  nav_params.region_min_size = _region_min_size;
  nav_params.region_merge_size = _region_merge_size;
  nav_params.edge_max_len = _edge_max_len;
  nav_params.edge_max_error = _edge_max_error;
  nav_params.verts_per_poly = _verts_per_poly;
  nav_params.detail_sample_dist = _detail_sample_dist;
  nav_params.detail_sample_max_error = _detail_sample_max_error;
  nav_params.partition_type = _partition_type;
  rcVcopy(nav_params.orig_bound_min, _mesh_bMin);
  nav_params.tile_cell_size = tcs;
  nav_params.filter_low_hanging_obstacles = _filter_low_hanging_obstacles;
  nav_params.filter_ledge_spans = _filter_ledge_spans;
  nav_params.filter_walkable_low_height_spans = _filter_walkable_low_height_spans;

  _nav_mesh_obj = new NavMesh(navMesh, nav_params, _untracked_tris, _tracked_nodes, _tracked_coll_nodes, _tri_verticies);
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

void NavMeshBuilder::get_vert_tris(std::vector<float> &verts, std::vector<int> &tris) {
  std::unordered_map<LVector3, int> vert_map;
  int num_verts = 0;
  for (const auto &tri_group : _tri_verticies) {
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

unsigned char* NavMeshBuilder::buildTileMesh(const int tx, const int ty,
                                             const float* bmin, const float* bmax, int& dataSize,
                                             std::vector<float> &verts, std::vector<int> &tris)
{
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
  _cfg.tileSize = (int)_tile_size;
  _cfg.borderSize = _cfg.walkableRadius + 3; // Reserve enough padding.
  _cfg.width = _cfg.tileSize + _cfg.borderSize*2;
  _cfg.height = _cfg.tileSize + _cfg.borderSize*2;
  _cfg.detailSampleDist = _detail_sample_dist < 0.9f ? 0 : _cell_size * _detail_sample_dist;
  _cfg.detailSampleMaxError = _cell_height * _detail_sample_max_error;

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
    navmeshgen_cat.error() << "buildNavigation: Out of memory 'solid'." << std::endl;
    return nullptr;
  }
  if (!rcCreateHeightfield(_ctx, *_solid, _cfg.width, _cfg.height, _cfg.bmin, _cfg.bmax, _cfg.cs, _cfg.ch))
  {
    navmeshgen_cat.error() << "buildNavigation: Could not create solid heightfield." << std::endl;
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
    navmeshgen_cat.error() << "build(): Could not rasterize triangles." << std::endl;
    return nullptr;
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
    return nullptr;
  }
  if (!rcBuildCompactHeightfield(_ctx, _cfg.walkableHeight, _cfg.walkableClimb, *_solid, *_chf)) {
    _ctx->log(RC_LOG_ERROR, "build(): Could not build compact data.");
    navmeshgen_cat.error() << "build(): Could not build compact data." << std::endl;
    return nullptr;
  }

  rcFreeHeightField(_solid);
  _solid = nullptr;

  // Erode the walkable area by agent radius.
  if (!rcErodeWalkableArea(_ctx, _cfg.walkableRadius, *_chf)) {
    _ctx->log(RC_LOG_ERROR, "build(): Could not erode.");
    navmeshgen_cat.error() << "build(): Could not erode." << std::endl;
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

  switch (_partition_type) {
    case NavMeshParams::SAMPLE_PARTITION_WATERSHED:
      // Prepare for region partitioning, by calculating distance field along the walkable surface.
      if (!rcBuildDistanceField(_ctx, *_chf)) {
        _ctx->log(RC_LOG_ERROR, "build(): Could not build distance field.");
        navmeshgen_cat.error() << "build(): Could not build distance field." << std::endl;
        return nullptr;
      }

      // Partition the walkable surface into simple regions without holes.
      if (!rcBuildRegions(_ctx, *_chf, _cfg.borderSize, _cfg.minRegionArea, _cfg.mergeRegionArea)) {
        _ctx->log(RC_LOG_ERROR, "build(): Could not build watershed regions.");
        navmeshgen_cat.error() << "build(): Could not build watershed regions." << std::endl;
        return nullptr;
      }
      break;
    case NavMeshParams::SAMPLE_PARTITION_MONOTONE:
      // Partition the walkable surface into simple regions without holes.
      // Monotone partitioning does not need distancefield.
      if (!rcBuildRegionsMonotone(_ctx, *_chf, _cfg.borderSize, _cfg.minRegionArea, _cfg.mergeRegionArea)) {
        _ctx->log(RC_LOG_ERROR, "build(): Could not build monotone regions.");
        navmeshgen_cat.error() << "build(): Could not build monotone regions." << std::endl;
        return nullptr;
      }
      break;
    case NavMeshParams::SAMPLE_PARTITION_LAYERS:
      // Partition the walkable surface into simple regions without holes.
      if (!rcBuildLayerRegions(_ctx, *_chf, _cfg.borderSize, _cfg.minRegionArea)) {
        _ctx->log(RC_LOG_ERROR, "build(): Could not build layer regions.");
        navmeshgen_cat.error() << "build(): Could not build layer regions." << std::endl;
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
    navmeshgen_cat.error() << "build(): Out of memory 'cset'." << std::endl;
    return nullptr;
  }
  if (!rcBuildContours(_ctx, *_chf, _cfg.maxSimplificationError, _cfg.maxEdgeLen, *_cset)) {
    _ctx->log(RC_LOG_ERROR, "build(): Could not create contours.");
    navmeshgen_cat.error() << "build(): Could not create contours." << std::endl;
    return nullptr;
  }

  //
  // Step 6. Build polygons mesh from contours.
  //

  // Build polygon navmesh from the contours.
  _pmesh = rcAllocPolyMesh();
  if (!_pmesh) {
    _ctx->log(RC_LOG_ERROR, "build(): Out of memory 'pmesh'.");
    navmeshgen_cat.error() << "build(): Out of memory 'pmesh'." << std::endl;
    return nullptr;
  }
  if (!rcBuildPolyMesh(_ctx, *_cset, _cfg.maxVertsPerPoly, *_pmesh)) {
    _ctx->log(RC_LOG_ERROR, "build(): Could not triangulate contours.");
    navmeshgen_cat.error() << "build(): Could not triangulate contours." << std::endl;
    return nullptr;
  }

  //
  // Step 7. Create detail mesh which allows to access approximate height on each polygon.
  //

  _dmesh = rcAllocPolyMeshDetail();
  if (!_dmesh) {
    _ctx->log(RC_LOG_ERROR, "build(): Out of memory 'pmdtl'.");
    navmeshgen_cat.error() << "build(): Out of memory 'pmdt1'." << std::endl;
    return nullptr;
  }

  if (!rcBuildPolyMeshDetail(_ctx, *_pmesh, *_chf, _cfg.detailSampleDist, _cfg.detailSampleMaxError, *_dmesh)) {
    _ctx->log(RC_LOG_ERROR, "build(): Could not build detail mesh.");
    navmeshgen_cat.error() << "build(): Could not build detail mesh." << std::endl;
    return nullptr;
  }
  navmeshgen_cat.info() << "Number of vertices: " << _pmesh->nverts << std::endl;
  navmeshgen_cat.info() << "Number of polygons: " << _pmesh->npolys << std::endl;
  navmeshgen_cat.info() << "Number of allocated polygons: " << _pmesh->maxpolys << std::endl;

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
      navmeshgen_cat.error() << "Too many vertices per tile " << _pmesh->nverts << " (max: " << 0xffff << ")." << std::endl;
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
    params.walkableHeight = _agent_height;
    params.walkableRadius = _agent_radius;
    params.walkableClimb = _agent_max_climb;
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
      navmeshgen_cat.error() << "Could not build Detour navmesh." << std::endl;
      return nullptr;
    }
  }

  dataSize = navDataSize;
  return navData;
}
