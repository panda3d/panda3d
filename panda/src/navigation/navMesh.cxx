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
	if (_nav_mesh != nullptr) {
		dtFreeNavMesh(_nav_mesh);
	}
  _nav_mesh = nullptr;

	if (_tile_cache != nullptr) {
		dtFreeTileCache(_tile_cache);
	}
	_tile_cache = nullptr;

	delete _tile_alloc;
	delete _tile_compressor;
	delete _tile_mesh_proc;
}

/**
 * NavMesh constructor to store dtNavMesh object as _nav_mesh.
 */
NavMesh::NavMesh(dtNavMesh *nav_mesh,
                 NavMeshParams params,
                 std::set<NavTriVertGroup> &untracked_tris,
								 dtTileCache *tile_cache,
								 dtTileCacheAlloc *tile_alloc,
								 dtTileCacheCompressor *tile_compressor,
								 dtTileCacheMeshProcess *tile_mesh_proc) :
                 _nav_mesh(nav_mesh),
                 _params(params),
								 _internal_rebuilder(params),
								 _tile_cache(tile_cache),
								 _tile_alloc(tile_alloc),
								 _tile_compressor(tile_compressor),
                 _tile_mesh_proc(tile_mesh_proc) {
  _internal_rebuilder._last_tris = untracked_tris;
  std::copy(untracked_tris.begin(), untracked_tris.end(), std::inserter(_untracked_tris, _untracked_tris.begin()));
}

/**
 * NavMesh constructor to store NavMesh Parameters.
 */
NavMesh::NavMesh(NavMeshParams params) :
                 _params(params),
                 _internal_rebuilder(params) {
  _nav_mesh = dtAllocNavMesh();
  if (!_nav_mesh)
  {
    navigation_cat.error() << "buildTiledNavigation: Could not allocate navmesh." << std::endl;
    return;
  }

  dtNavMeshParams vmParams = {};
  rcVcopy(vmParams.orig, _params.get_orig_bound_min().get_data());
  vmParams.tileWidth = _params.get_tile_size()*_params.get_cell_size();
  vmParams.tileHeight = _params.get_tile_size()*_params.get_cell_size();
  vmParams.maxTiles = _params.get_max_tiles();
  vmParams.maxPolys = _params.get_max_polys_per_tile();

  dtStatus status;

  status = _nav_mesh->init(&vmParams);
  if (dtStatusFailed(status))
  {
    navigation_cat.error() << "buildTiledNavigation: Could not init navmesh." << std::endl;
  }
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

/**
 * Update the NavMesh to account for changes in the tracked nodes.
 */
void NavMesh::
update() {
  _internal_rebuilder.update_nav_mesh(this, _tile_cache);

  // Clear debug mesh cache.
  _cache_poly_outlines = nullptr;
  _cache_poly_verts.clear();
}

/**
 * Add a node and its descendents to the NavMesh, optionally tracking its changes in the future.
 *
 * Does not affect the NavMesh until update() is called.
 */
void NavMesh::
add_node_path(NodePath node, bool tracked) {
  if (tracked) {
    _tracked_nodes.push_back(node);

    // De-dup.
    std::sort( _tracked_nodes.begin(), _tracked_nodes.end() );
    _tracked_nodes.erase( std::unique( _tracked_nodes.begin(), _tracked_nodes.end() ), _tracked_nodes.end() );
  } else {
    std::set<NavTriVertGroup> new_tris;
    auto transform = node.get_net_transform();
    _internal_rebuilder.process_node_path(new_tris, node, transform);

    // De-dup the untracked tris. Is there a better way to do this?
    std::copy(_untracked_tris.begin(), _untracked_tris.end(), std::inserter(new_tris, new_tris.begin()));
    _untracked_tris.clear();
    std::copy(new_tris.begin(), new_tris.end(), std::inserter(_untracked_tris, _untracked_tris.begin()));
  }
}

/**
 * Add all the CollisionNodes under a node to the NavMesh,
 * optionally tracking its changes in the future.
 *
 * Does not affect the NavMesh until update() is called.
 */
void NavMesh::
add_coll_node_path(NodePath node, BitMask32 mask, bool tracked) {
  if (tracked) {
    _tracked_coll_nodes.push_back({node, mask});

    // De-dup.
    std::sort( _tracked_coll_nodes.begin(), _tracked_coll_nodes.end() );
    _tracked_coll_nodes.erase( std::unique( _tracked_coll_nodes.begin(), _tracked_coll_nodes.end() ), _tracked_coll_nodes.end() );
  } else {
    std::set<NavTriVertGroup> new_tris;
    auto transform = node.get_net_transform();
    _internal_rebuilder.process_coll_node_path(new_tris, node, transform, mask);

    // De-dup the untracked tris. Is there a better way to do this?
    std::copy(_untracked_tris.begin(), _untracked_tris.end(), std::inserter(new_tris, new_tris.begin()));
    _untracked_tris.clear();
    std::copy(new_tris.begin(), new_tris.end(), std::inserter(_untracked_tris, _untracked_tris.begin()));
  }
}

/**
 * Add a particular Geom to the NavMesh.
 *
 * Does not affect the NavMesh until update() is called.
 */
void NavMesh::
add_geom(PT(Geom) geom) {
    std::set<NavTriVertGroup> new_tris;
    CPT(Geom) const_geom = geom;
    _internal_rebuilder.process_geom(new_tris, const_geom, TransformState::make_identity());

    // De-dup the untracked tris. Is there a better way to do this?
    std::copy(_untracked_tris.begin(), _untracked_tris.end(), std::inserter(new_tris, new_tris.begin()));
    _untracked_tris.clear();
    std::copy(new_tris.begin(), new_tris.end(), std::inserter(_untracked_tris, _untracked_tris.begin()));
}

/**
 * Any NavObstacleNodes under this NodePath will be added to the nav mesh on next update().
 */
void NavMesh::
add_obstacles(NodePath obstacle) {
  if (std::find(_obstacle_nodes.begin(), _obstacle_nodes.end(), obstacle) == _obstacle_nodes.end()) {
    _obstacle_nodes.emplace_back(obstacle);
  }
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
 *
 * Note: Tracked nodes and obstacles are NOT saved.
 */
void NavMesh::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritableReferenceCount::write_datagram(manager, dg);

  // Version
  dg.add_int16(1);

  // Using double precision floats for storage as recast detour might
  // be able to switch to double later on.
  dg.add_float64(_params.get_cell_size());
  dg.add_float64(_params.get_cell_height());
  dg.add_float64(_params.get_actor_height());
  dg.add_float64(_params.get_actor_radius());
  dg.add_float64(_params.get_actor_max_climb());
  dg.add_float64(_params.get_actor_max_slope());
  dg.add_float64(_params.get_region_min_size());
  dg.add_float64(_params.get_region_merge_size());
  dg.add_float64(_params.get_edge_max_len());
  dg.add_float64(_params.get_edge_max_error());
  dg.add_int32(_params.get_verts_per_poly());
  dg.add_float64(_params.get_detail_sample_dist());
  dg.add_float64(_params.get_detail_sample_max_error());
  dg.add_int32(_params.get_partition_type());
  dg.add_float64(_params.get_tile_size());
  dg.add_bool(_params.get_filter_low_hanging_obstacles());
  dg.add_bool(_params.get_filter_ledge_spans());
  dg.add_bool(_params.get_filter_walkable_low_height_spans());

  int num_tiles = 0;

  for (int i = 0; i < _tile_cache->getTileCount(); ++i)
  {
    const dtCompressedTile* tile = _tile_cache->getTile(i);
    if (!tile || !tile->header || !tile->dataSize) continue;
    num_tiles++;
  }

  dg.add_uint32(num_tiles);

  navigation_cat.debug() << "Writing " << num_tiles << " tiles" << std::endl;
  if (num_tiles == 0) {
    navigation_cat.warning() << "The NavMesh being saved does not have any tiles generated. Did you forget to run update() ?" << std::endl;
  }

  for (int i = 0; i < _tile_cache->getTileCount(); ++i)
  {
    const dtCompressedTile* tile = _tile_cache->getTile(i);
    if (!tile || !tile->header || !tile->dataSize) continue;
    dg.add_blob({tile->data, tile->data + tile->dataSize});
    dg.add_uint32(_tile_cache->getTileRef(tile));
  }

  dg.add_uint64(_untracked_tris.size());

  navigation_cat.debug() << "Writing " << _untracked_tris.size() << " untracked tris" << std::endl;

  for (auto &tri_group : _untracked_tris) {
    dg.add_float64(tri_group.a[0]);
    dg.add_float64(tri_group.a[1]);
    dg.add_float64(tri_group.a[2]);
    dg.add_float64(tri_group.b[0]);
    dg.add_float64(tri_group.b[1]);
    dg.add_float64(tri_group.b[2]);
    dg.add_float64(tri_group.c[0]);
    dg.add_float64(tri_group.c[1]);
    dg.add_float64(tri_group.c[2]);
  }
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type NavMesh is encountered in the Bam file.  It should create the
 * NavMesh and extract its information from the file.
 */
TypedWritable *NavMesh::
make_from_bam(const FactoryParams &params) {
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);

  NavMesh *mesh = new NavMesh();
  mesh->fillin(scan, manager);

  return mesh;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new NavMesh.
 */
void NavMesh::
fillin(DatagramIterator &scan, BamReader *manager) {
  NavMeshParams navParams;

  // Version
  int version = scan.get_int16();
  nassertv(version == 1);

  _params.set_cell_size(scan.get_float64());
  _params.set_cell_height(scan.get_float64());
  _params.set_actor_height(scan.get_float64());
  _params.set_actor_radius(scan.get_float64());
  _params.set_actor_max_climb(scan.get_float64());
  _params.set_actor_max_slope(scan.get_float64());
  _params.set_region_min_size(scan.get_float64());
  _params.set_region_merge_size(scan.get_float64());
  _params.set_edge_max_len(scan.get_float64());
  _params.set_edge_max_error(scan.get_float64());
  _params.set_verts_per_poly(scan.get_int32());
  _params.set_detail_sample_dist(scan.get_float64());
  _params.set_detail_sample_max_error(scan.get_float64());
  _params.set_partition_type(static_cast<NavMeshParams::PartitionType>(scan.get_int32()));
  _params.set_tile_size(scan.get_float64());
  _params.set_filter_low_hanging_obstacles(scan.get_bool());
  _params.set_filter_ledge_spans(scan.get_bool());
  _params.set_filter_walkable_low_height_spans(scan.get_bool());

  // Tile cache params.
  dtTileCacheParams tcparams = {};
  memset(&tcparams, 0, sizeof(tcparams));
  rcVcopy(tcparams.orig, _params.get_orig_bound_min().get_data());
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

  _tile_cache = dtAllocTileCache();
  if (!_tile_cache)
  {
    navigation_cat.error() << "Could not allocate tile cache." << std::endl;
    return;
  }

  dtTileCacheAlloc *tile_alloc = NavMeshBuilder::make_tile_allocator();
  dtTileCacheCompressor *tile_compressor = NavMeshBuilder::make_tile_compressor();
  dtTileCacheMeshProcess *tile_mesh_proc = NavMeshBuilder::make_mesh_process();

  status = _tile_cache->init(&tcparams, tile_alloc, tile_compressor, tile_mesh_proc);
  if (dtStatusFailed(status))
  {
    navigation_cat.error() << "Could not init tile cache." << std::endl;
    return;
  }

  _nav_mesh = dtAllocNavMesh();
  if (!_nav_mesh)
  {
    navigation_cat.error() << "Could not allocate navmesh." << std::endl;
    return;
  }

  dtNavMeshParams dtParams = {};
  rcVcopy(dtParams.orig, _params.get_orig_bound_min().get_data());
  dtParams.tileWidth = _params.get_tile_size()*_params.get_cell_size();
  dtParams.tileHeight = _params.get_tile_size()*_params.get_cell_size();
  dtParams.maxTiles = _params.get_max_tiles();
  dtParams.maxPolys = _params.get_max_polys_per_tile();

  status = _nav_mesh->init(&dtParams);
  if (dtStatusFailed(status))
  {
    navigation_cat.error() << "Could not init navmesh." << std::endl;
    return;
  }

  unsigned int num_tiles = scan.get_uint32();

  navigation_cat.debug() << "Loading " << num_tiles << " tiles from bam." << std::endl;

  for (unsigned int i = 0; i < num_tiles; ++i) {
    auto data = scan.get_blob();
    // Not used.
    scan.get_uint32();
    // Need to make a copy of the tile data for the TileCache to own.
    auto tile_data = static_cast<unsigned char *>(malloc(data.size()));
    std::copy(data.begin(), data.end(), tile_data);
    dtCompressedTileRef tile = 0;
    status = _tile_cache->addTile(tile_data, data.size(), DT_COMPRESSEDTILE_FREE_DATA, &tile);
    if (dtStatusFailed(status))
    {
      navigation_cat.warning() << "Failed to load tile " << i << " from bam." << std::endl;
      continue;
    }
    if (tile)
      _tile_cache->buildNavMeshTile(tile, _nav_mesh);
  }

  uint64_t num_tris = scan.get_uint64();

  navigation_cat.debug() << "Loading " << num_tris << " untracked tris from bam." << std::endl;

  for (uint64_t i = 0; i < num_tris; ++i) {
    _untracked_tris.emplace_back(
      NavTriVertGroup {
        {
          static_cast<float>(scan.get_float64()),
          static_cast<float>(scan.get_float64()),
          static_cast<float>(scan.get_float64())
        },
        {
          static_cast<float>(scan.get_float64()),
          static_cast<float>(scan.get_float64()),
          static_cast<float>(scan.get_float64())
        },
        {
          static_cast<float>(scan.get_float64()),
          static_cast<float>(scan.get_float64()),
          static_cast<float>(scan.get_float64())
        }
      });
  }

  _tile_alloc = tile_alloc;
  _tile_compressor = tile_compressor;
  _tile_mesh_proc = tile_mesh_proc;
  _internal_rebuilder = NavMeshBuilder(_params);
  std::copy(_untracked_tris.begin(), _untracked_tris.end(), std::inserter(_internal_rebuilder._last_tris, _internal_rebuilder._last_tris.begin()));
}
