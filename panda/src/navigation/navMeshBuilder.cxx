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
#include "recastnavigation/DetourCommon.h"
#include "recastnavigation/DetourNavMesh.h"
#include "recastnavigation/DetourNavMeshBuilder.h"
#include "recastnavigation/DetourTileCacheBuilder.h"
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
#include "compress_string.h"


typedef unsigned int dtStatus;

/**
 * Uses the (de)compress_string() functions in express to compress tiles,
 * as required by Detour.
 *
 * (Internal use only)
 */
struct ExpressCompressor : public dtTileCacheCompressor
{
public:
  virtual int maxCompressedSize(const int bufferSize)
  {
    return (int)(bufferSize* 1.05f);
  }

  virtual dtStatus compress(const unsigned char* buffer, const int bufferSize,
                            unsigned char* compressed, const int maxCompressedSize, int* compressedSize)
  {
    auto result = compress_string(std::string(reinterpret_cast<const char *>(buffer), bufferSize), 6);

    *compressedSize = std::min((int)result.size(), maxCompressedSize);
    memcpy(compressed, result.data(), *compressedSize);

    return DT_SUCCESS;
  }

  virtual dtStatus decompress(const unsigned char* compressed, const int compressedSize,
                              unsigned char* buffer, const int maxBufferSize, int* bufferSize)
  {
    auto result = decompress_string(std::string(reinterpret_cast<const char *>(compressed), compressedSize));

    *bufferSize = std::min((int)result.size(), maxBufferSize);
    memcpy(buffer, result.data(), *bufferSize);

    return *bufferSize < 0 ? DT_FAILURE : DT_SUCCESS;
  }
};

/**
 * Use a simple naive allocator for Detour tiles.
 *
 * (Internal use only)
 */
struct LinearAllocator : public dtTileCacheAlloc
{
  unsigned char* buffer;
  size_t capacity;
  size_t top;
  size_t high;

  LinearAllocator(const size_t cap) : buffer(0), capacity(0), top(0), high(0)
  {
    resize(cap);
  }

  ~LinearAllocator()
  {
    dtFree(buffer);
  }

  void resize(const size_t cap)
  {
    if (buffer) dtFree(buffer);
    buffer = (unsigned char*)dtAlloc(cap, DT_ALLOC_PERM);
    capacity = cap;
  }

  virtual void reset()
  {
    high = dtMax(high, top);
    top = 0;
  }

  virtual void* alloc(const size_t size)
  {
    if (!buffer)
      return 0;
    if (top+size > capacity)
      return 0;
    unsigned char* mem = &buffer[top];
    top += size;
    return mem;
  }

  virtual void free(void* /*ptr*/)
  {
    // Empty
  }
};

/**
 * This is a basic naive mesh processor that we use for Detour.
 *
 * (Internal use only)
 */
struct MeshProcess : public dtTileCacheMeshProcess
{

  inline MeshProcess()
  { }

  inline void init()
  { }

  virtual void process(struct dtNavMeshCreateParams* params,
                       unsigned char* polyAreas, unsigned short* polyFlags)
  {
    for (int i = 0; i < params->polyCount; ++i) {
      // Initialize all polygons to 1, so they are enabled by default.
      polyFlags[i] = 1;
    }
  }
};


/**
 * NavMeshBuilder constructor which initiates the member variables
 */
NavMeshBuilder::NavMeshBuilder(NodePath parent) :
  _parent(parent) {
}

NavMeshBuilder::NavMeshBuilder(NavMeshParams &params, NodePath parent) :
  _parent(parent),
  _params(params) {
}

NavMeshBuilder::~NavMeshBuilder() {
}

/**
 * This function adds a custom polygon with three vertices to the input geometry.
 */
void NavMeshBuilder::add_polygon(LPoint3 a, LPoint3 b, LPoint3 c) {
  mat_to_y.xform_point_in_place(a);
  update_bounds(a);

  mat_to_y.xform_point_in_place(b);
  update_bounds(b);

  mat_to_y.xform_point_in_place(c);
  update_bounds(c);

  _untracked_tris.insert((NavTriVertGroup){a, b, c});
}

/**
 * This function adds a custom polygon with equal to or more than three vertices to the input geometry.
 */
void NavMeshBuilder::add_polygon(PTA_LVecBase3f &vec) {
  for (size_t i = 2; i < vec.size(); ++i) {
    add_polygon(LPoint3(vec[i-2]), LPoint3(vec[i-1]), LPoint3(vec[i]));
  }
}

/**
 * Adds a custom Geom to the NavMesh.
 */
bool NavMeshBuilder::add_geom(PT(Geom) geom) {
  CPT(Geom) const_geom = geom;
  process_geom(_untracked_tris, const_geom, TransformState::make_identity());
  return true;
}

/**
 * Adds all visible geometry under the given node to the NavMesh.
 */
bool NavMeshBuilder::add_node_path(NodePath node) {
  CPT(TransformState) transform = node.get_transform(_parent);

  process_node_path(_untracked_tris, node, transform);

  return true;
}

/**
 * Adds all collision geometry under the given node to the NavMesh.
 */
bool NavMeshBuilder::add_coll_node_path(NodePath node, BitMask32 mask) {
  CPT(TransformState) transform = node.get_transform(_parent);

  process_coll_node_path(_untracked_tris, node, transform, mask);

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
      transform.xform_point_in_place(v1);
      update_bounds(v1);

      int b = prim->get_vertex(s + 1);
      vertex.set_row(b);
      v2 = vertex.get_data3();
      transform.xform_point_in_place(v2);
      update_bounds(v2);

      int c = prim->get_vertex(s + 2);
      vertex.set_row(c);
      v3 = vertex.get_data3();
      transform.xform_point_in_place(v3);
      update_bounds(v3);

      tris.insert((NavTriVertGroup){v1, v2, v3});
    } else if (e - s > 3) {
      for (int i = s + 2; i < e; ++i) {
        int a = prim->get_vertex(s);
        vertex.set_row(a);
        v1 = vertex.get_data3();
        transform.xform_point_in_place(v1);
        update_bounds(v1);

        int b = prim->get_vertex(i - 1);
        vertex.set_row(b);
        v2 = vertex.get_data3();
        transform.xform_point_in_place(v2);
        update_bounds(v2);

        int c = prim->get_vertex(i);
        vertex.set_row(c);
        v3 = vertex.get_data3();
        transform.xform_point_in_place(v3);
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
      for (size_t i = 0; i < g->get_num_solids(); i++) {
        PT(GeomNode) gn = g->get_solid(i)->get_viz();
        process_geom_node(tris, gn, transform);
      }
    }
  }

  NodePathCollection children = node.get_children();
  for (int i = 0; i < children.get_num_paths(); i++) {
    NodePath cnp = children.get_path(i);
    CPT(TransformState) child_transform = cnp.get_transform();
    CPT(TransformState) net_transform = transform->compose(child_transform);
    process_coll_node_path(tris, cnp, net_transform, mask);
  }
}

struct TileCacheData
{
  unsigned char* data;
  int dataSize;
};

int NavMeshBuilder::rasterizeTileLayers(
    const int tx, const int ty,
    const float* bmin, const float* bmax,
    pvector<float> &verts, pvector<int> &tris,
    TileCacheData* tiles,
    const int maxTiles)
{
  ExpressCompressor comp;

  rcConfig cfg = {};
  memset(&cfg, 0, sizeof(cfg));
  cfg.cs = _params.get_cell_size();
  cfg.ch = _params.get_cell_height();
  cfg.walkableSlopeAngle = _params.get_actor_max_slope();
  cfg.walkableHeight = (int)ceilf(_params.get_actor_height() / cfg.ch);
  cfg.walkableClimb = (int)floorf(_params.get_actor_max_climb() / cfg.ch);
  cfg.walkableRadius = (int)ceilf(_params.get_actor_radius() / cfg.cs);
  cfg.maxEdgeLen = (int)(_params.get_edge_max_len() / _params.get_cell_size());
  cfg.maxSimplificationError = _params.get_edge_max_error();
  cfg.minRegionArea = (int)rcSqr(_params.get_region_min_size());    // Note: area = size*size
  cfg.mergeRegionArea = (int)rcSqr(_params.get_region_merge_size());  // Note: area = size*size
  cfg.maxVertsPerPoly = (int)_params.get_verts_per_poly();
  cfg.tileSize = (int)_params.get_tile_size();
  cfg.borderSize = cfg.walkableRadius + 3; // Reserve enough padding.
  cfg.width = cfg.tileSize + cfg.borderSize*2;
  cfg.height = cfg.tileSize + cfg.borderSize*2;
  cfg.detailSampleDist = _params.get_detail_sample_dist() < 0.9f ? 0 : _params.get_cell_size() * _params.get_detail_sample_dist();
  cfg.detailSampleMaxError = _params.get_cell_height() * _params.get_detail_sample_max_error();

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
  rcVcopy(cfg.bmin, bmin);
  rcVcopy(cfg.bmax, bmax);
  cfg.bmin[0] -= cfg.borderSize*cfg.cs;
  cfg.bmin[2] -= cfg.borderSize*cfg.cs;
  cfg.bmax[0] += cfg.borderSize*cfg.cs;
  cfg.bmax[2] += cfg.borderSize*cfg.cs;

  // Allocate voxel heightfield where we rasterize our input data to.
	rcHeightfield *solid = rcAllocHeightfield();
  if (!solid)
  {
    navigation_cat.error() << "buildNavigation: Out of memory 'solid'." << std::endl;
    return 0;
  }
	rcContext ctx;
	if (!rcCreateHeightfield(&ctx, *solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch))
  {
    navigation_cat.error() << "buildNavigation: Could not create solid heightfield." << std::endl;
    return 0;
  }

  // Allocate array that can hold triangle area types.
  // If you have multiple meshes you need to process, allocate
  // an array which can hold the max number of triangles you need to process.
	std::vector<unsigned char> triareas;
  triareas.resize(static_cast<int>(tris.size() / 3), 0);

  // Find triangles which are walkable based on their slope and rasterize them.
  // If your input data is multiple meshes, you can transform them here, calculate
  // the are type for each of the meshes and rasterize them.
  rcMarkWalkableTriangles(&ctx, cfg.walkableSlopeAngle, verts.data(), static_cast<int>(verts.size() / 3), tris.data(), static_cast<int>(tris.size() / 3), triareas.data());

  if (!rcRasterizeTriangles(&ctx, verts.data(), static_cast<int>(verts.size() / 3), tris.data(), triareas.data(), static_cast<int>(tris.size() / 3), *solid, cfg.walkableClimb)) {
    navigation_cat.error() << "build(): Could not rasterize triangles." << std::endl;
    return 0;
  }


  //
  // Step 3. Filter walkables surfaces.
  //

  // Once all geoemtry is rasterized, we do initial pass of filtering to
  // remove unwanted overhangs caused by the conservative rasterization
  // as well as filter spans where the character cannot possibly stand.
  if (_params.get_filter_low_hanging_obstacles())
    rcFilterLowHangingWalkableObstacles(&ctx, cfg.walkableClimb, *solid);
  if (_params.get_filter_ledge_spans())
    rcFilterLedgeSpans(&ctx, cfg.walkableHeight, cfg.walkableClimb, *solid);
  if (_params.get_filter_walkable_low_height_spans())
    rcFilterWalkableLowHeightSpans(&ctx, cfg.walkableHeight, *solid);


  //
  // Step 4. Partition walkable surface to simple regions.
  //

  // Compact the heightfield so that it is faster to handle from now on.
  // This will result more cache coherent data as well as the neighbours
  // between walkable cells will be calculated.
	rcCompactHeightfield *chf = rcAllocCompactHeightfield();
  if (!chf) {
    navigation_cat.error() << "build(): Out of memory 'chf'." << std::endl;
    return 0;
  }
  if (!rcBuildCompactHeightfield(&ctx, cfg.walkableHeight, cfg.walkableClimb, *solid, *chf)) {
    navigation_cat.error() << "build(): Could not build compact data." << std::endl;
    return 0;
  }

  rcFreeHeightField(solid);
	solid = nullptr;

  // Erode the walkable area by agent radius.
  if (!rcErodeWalkableArea(&ctx, cfg.walkableRadius, *chf)) {
    navigation_cat.error() << "build(): Could not erode." << std::endl;
    return 0;
  }

  auto lset = rcAllocHeightfieldLayerSet();
  if (!lset)
  {
    navigation_cat.error() << "buildNavigation: Out of memory 'lset'." << std::endl;
    return 0;
  }
  if (!rcBuildHeightfieldLayers(&ctx, *chf, cfg.borderSize, cfg.walkableHeight, *lset))
  {
    navigation_cat.error() << "buildNavigation: Could not build heighfield layers." << std::endl;
    return 0;
  }

  int ntiles = 0;
  TileCacheData _tiles[_params.get_max_layers_per_tile()];

  for (int i = 0; i < rcMin(lset->nlayers, _params.get_max_layers_per_tile()); ++i)
  {
    TileCacheData* tile = &_tiles[ntiles++];
    const rcHeightfieldLayer* layer = &lset->layers[i];

    // Store header
    dtTileCacheLayerHeader header = {};
    header.magic = DT_TILECACHE_MAGIC;
    header.version = DT_TILECACHE_VERSION;

    // Tile layer location in the navmesh.
    header.tx = tx;
    header.ty = ty;
    header.tlayer = i;
    dtVcopy(header.bmin, layer->bmin);
    dtVcopy(header.bmax, layer->bmax);

    // Tile info.
    header.width = (unsigned char)layer->width;
    header.height = (unsigned char)layer->height;
    header.minx = (unsigned char)layer->minx;
    header.maxx = (unsigned char)layer->maxx;
    header.miny = (unsigned char)layer->miny;
    header.maxy = (unsigned char)layer->maxy;
    header.hmin = (unsigned short)layer->hmin;
    header.hmax = (unsigned short)layer->hmax;

    dtStatus status = dtBuildTileCacheLayer(&comp, &header, layer->heights, layer->areas, layer->cons,
                                            &tile->data, &tile->dataSize);
    if (dtStatusFailed(status))
    {
      return 0;
    }
  }

  // Transfer ownership of tile data from build context to the caller.
  int n = 0;
  for (int i = 0; i < rcMin(ntiles, maxTiles); ++i)
  {
    tiles[n++] = _tiles[i];
    _tiles[i].data = nullptr;
    _tiles[i].dataSize = 0;
  }

  return n;
}

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

  // Tile cache params.
  dtTileCacheParams tcparams;
  memset(&tcparams, 0, sizeof(tcparams));
  rcVcopy(tcparams.orig, _mesh_bMin);
  tcparams.cs = _params.get_cell_size();
  tcparams.ch = _params.get_cell_height();
  tcparams.width = (int)_params.get_tile_size();
  tcparams.height = (int)_params.get_tile_size();
  tcparams.walkableHeight = _params.get_actor_height();
  tcparams.walkableRadius = _params.get_actor_radius();
  tcparams.walkableClimb = _params.get_actor_max_climb();
  tcparams.maxSimplificationError = _params.get_edge_max_error();
  tcparams.maxTiles = _params.get_max_tiles() * _params.get_max_layers_per_tile();
  tcparams.maxObstacles = 256;

  dtStatus status;

	dtTileCache *tile_cache = dtAllocTileCache();
  if (!tile_cache)
  {
    navigation_cat.error() << "buildTiledNavigation: Could not allocate tile cache." << std::endl;
    return nullptr;
  }

	dtTileCacheAlloc *tile_alloc = new LinearAllocator(32000);
	dtTileCacheCompressor *tile_compressor = new ExpressCompressor();
	dtTileCacheMeshProcess *tile_mesh_proc = new MeshProcess();

  status = tile_cache->init(&tcparams, tile_alloc, tile_compressor, tile_mesh_proc);
  if (dtStatusFailed(status))
  {
    navigation_cat.error() << "buildTiledNavigation: Could not init tile cache." << std::endl;
    return nullptr;
  }

  dtNavMesh *navMesh = dtAllocNavMesh();
  if (!navMesh)
  {
    navigation_cat.error() << "buildTiledNavigation: Could not allocate navmesh." << std::endl;
    return nullptr;
  }

  dtNavMeshParams params = {};
  rcVcopy(params.orig, _mesh_bMin);
  params.tileWidth = _params.get_tile_size()*_params.get_cell_size();
  params.tileHeight = _params.get_tile_size()*_params.get_cell_size();
  params.maxTiles = _params.get_max_tiles();
  params.maxPolys = _params.get_max_polys_per_tile();

  status = navMesh->init(&params);
  if (dtStatusFailed(status))
  {
    navigation_cat.error() << "buildTiledNavigation: Could not init navmesh." << std::endl;
    return nullptr;
  }

  pvector<float> verts;
  pvector<int> tris;
  get_vert_tris(_untracked_tris, verts, tris);

  for (int y = 0; y < th; ++y) {
    for (int x = 0; x < tw; ++x) {
      TileCacheData tiles[_params.get_max_layers_per_tile()];
      memset(tiles, 0, sizeof(tiles));
      tileBmin[0] = _mesh_bMin[0] + x*tcs;
      tileBmin[1] = _mesh_bMin[1];
      tileBmin[2] = _mesh_bMin[2] + y*tcs;

      tileBmax[0] = _mesh_bMin[0] + (x+1)*tcs;
      tileBmax[1] = _mesh_bMax[1];
      tileBmax[2] = _mesh_bMin[2] + (y+1)*tcs;
      int ntiles = rasterizeTileLayers(x, y, tileBmin, tileBmax,
                                       verts, tris, tiles, _params.get_max_layers_per_tile());

      for (int i = 0; i < ntiles; ++i)
      {
        TileCacheData* tile = &tiles[i];
        status = tile_cache->addTile(tile->data, tile->dataSize, DT_COMPRESSEDTILE_FREE_DATA, 0);
        if (dtStatusFailed(status))
        {
          dtFree(tile->data);
          tile->data = 0;
          continue;
        }
      }
    }
  }

  for (int y = 0; y < th; ++y)
    for (int x = 0; x < tw; ++x)
      tile_cache->buildNavMeshTilesAt(x,y, navMesh);

  return new NavMesh(navMesh, _params, _untracked_tris, tile_cache, tile_alloc, tile_compressor, tile_mesh_proc);
}

void NavMeshBuilder::
update_nav_mesh(NavMesh *nav_mesh_obj, dtTileCache *tile_cache) {
  std::set<NavTriVertGroup> tri_vert_set;

  // Start with the untracked tris.
  auto untracked = nav_mesh_obj->get_untracked_tris();
  std::copy(untracked.begin(), untracked.end(), std::inserter(tri_vert_set, tri_vert_set.begin()));

  // Set the bounds based on the untracked tris. Tracked nodes will be added as they are found.
  _bounds_set = false;
  for (const auto &tri_vert : tri_vert_set) {
    update_bounds(tri_vert.a);
    update_bounds(tri_vert.b);
    update_bounds(tri_vert.c);
  }

  // Add in the tracked nodes.
  for (auto &node : nav_mesh_obj->get_tracked_nodes()) {
    CPT(TransformState) state = node.get_net_transform();
    process_node_path(tri_vert_set, node, state);
  }
  for (auto &node : nav_mesh_obj->get_tracked_coll_nodes()) {
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

  std::set<ObstacleData> existing_obstacles;
  std::map<ObstacleData, dtObstacleRef> refs;
  for (int i = 0; i < tile_cache->getObstacleCount(); ++i) {
    const dtTileCacheObstacle *ob = tile_cache->getObstacle(i);
    if (ob->state == DT_OBSTACLE_EMPTY)
      continue;

    ObstacleData data;
    switch (ob->type) {
      case DT_OBSTACLE_CYLINDER:
        data = {ob->type, LPoint3(ob->cylinder.pos[0], ob->cylinder.pos[1], ob->cylinder.pos[2]), LPoint3(),
                                     ob->cylinder.height, ob->cylinder.radius};
        break;
      case DT_OBSTACLE_BOX:
        data = {ob->type, LPoint3(ob->box.bmin[0], ob->box.bmin[1], ob->box.bmin[2]),
                                     LPoint3(ob->box.bmax[0], ob->box.bmax[1], ob->box.bmax[2]),
                                     0, 0};
        break;
      case DT_OBSTACLE_ORIENTED_BOX:
        data = {ob->type, LPoint3(ob->orientedBox.center[0], ob->orientedBox.center[1], ob->orientedBox.center[2]),
                                     LPoint3(ob->orientedBox.halfExtents[0], ob->orientedBox.halfExtents[1], ob->orientedBox.halfExtents[2]),
                                     ob->orientedBox.rotAux[0], ob->orientedBox.rotAux[1]};
        break;
    }
    existing_obstacles.emplace(data);
    refs.emplace(data, tile_cache->getObstacleRef(ob));
  }

  std::set<ObstacleData> new_obstacles;

  for (auto &node : nav_mesh_obj->get_obstacles()) {
    CPT(TransformState) state = node.get_net_transform();
    process_obstacle_node_path(tile_cache, existing_obstacles, new_obstacles, node, state);
  }

  // Remove all obstacles that are no longer there.
  std::set<ObstacleData> removed_obstacles;
  std::set_difference(existing_obstacles.begin(), existing_obstacles.end(), new_obstacles.begin(), new_obstacles.end(), std::inserter(removed_obstacles, removed_obstacles.begin()));
  for (auto &obstacle : removed_obstacles) {
    tile_cache->removeObstacle(refs[obstacle]);
  }

  pvector<float> verts;
  pvector<int> tris;
  get_vert_tris(tri_vert_set, verts, tris);

  float tileBmin[3] = { 0, 0, 0 };
  float tileBmax[3] = { 0, 0, 0 };
  for (auto &tile_coods : tiles_to_regen) {
    TileCacheData tiles[_params.get_max_layers_per_tile()];
    memset(tiles, 0, sizeof(tiles));
    tileBmin[0] = orig_bound_min[0] + static_cast<float>(tile_coods.first) * _params.get_tile_cell_size();
    tileBmin[1] = _mesh_bMin[1];
    tileBmin[2] = orig_bound_min[2] + static_cast<float>(tile_coods.second) * _params.get_tile_cell_size();

    tileBmax[0] = orig_bound_min[0] + static_cast<float>(tile_coods.first+1) * _params.get_tile_cell_size();
    tileBmax[1] = _mesh_bMax[1];
    tileBmax[2] = orig_bound_min[2] + static_cast<float>(tile_coods.second+1) * _params.get_tile_cell_size();

    int ntiles = rasterizeTileLayers(tile_coods.first, tile_coods.second, tileBmin, tileBmax,
                                     verts, tris, tiles, _params.get_max_layers_per_tile());

    dtCompressedTileRef old_layers[_params.get_max_layers_per_tile()];
    int old_nlayers = tile_cache->getTilesAt(tile_coods.first, tile_coods.second,
                                              reinterpret_cast<dtCompressedTileRef *>(&old_layers), _params.get_max_layers_per_tile());
    for (int i = 0; i < old_nlayers; ++i) {
      tile_cache->removeTile(old_layers[i], nullptr, nullptr);
    }

    for (int i = 0; i < ntiles; ++i)
    {
      TileCacheData* tile = &tiles[i];
      auto status = tile_cache->addTile(tile->data, tile->dataSize, DT_COMPRESSEDTILE_FREE_DATA, 0);
      if (dtStatusFailed(status))
      {
        dtFree(tile->data);
        tile->data = 0;
        continue;
      }
    }
  }

  bool obstacles_done = false;
  while (!obstacles_done) {
    tile_cache->update(0, nav_mesh_obj->get_nav_mesh(), &obstacles_done);
  }
  for (auto &tile_coods : tiles_to_regen) {
    tile_cache->buildNavMeshTilesAt(tile_coods.first, tile_coods.second, nav_mesh_obj->get_nav_mesh());
  }

  _last_tris = tri_vert_set;
}

void NavMeshBuilder::
process_obstacle_node_path(dtTileCache *tile_cache, std::set<ObstacleData> &existing_obstacles, std::set<ObstacleData> &new_obstacles, const NodePath &node, CPT(TransformState) &transform) {
  // Do not process stashed nodes.
  if (node.is_stashed()) {
    return;
  }

  if (node.node()->is_of_type(NavObstacleNode::get_class_type())) {
    PT(NavObstacleNode) g = DCAST(NavObstacleNode, node.node());
    auto mat = transform->get_mat() * mat_to_y;
    auto obs_data = g->get_obstacle_data(mat);
    new_obstacles.emplace(obs_data);
    if (existing_obstacles.find(obs_data) == existing_obstacles.end()) {
      g->add_obstacle(tile_cache, mat);
    }
    existing_obstacles.emplace(obs_data);
  }

  NodePathCollection children = node.get_children();
  for (int i=0; i<children.get_num_paths(); i++) {
    NodePath cnp = children.get_path(i);
    CPT(TransformState) child_transform = cnp.get_transform();
    CPT(TransformState) net_transform = transform->compose(child_transform);
    process_obstacle_node_path(tile_cache, existing_obstacles, new_obstacles, cnp, net_transform);
  }
}
