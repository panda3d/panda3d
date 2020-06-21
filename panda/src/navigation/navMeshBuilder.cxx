#include "navMeshBuilder.h"
#include "Recast.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "DetourNavMeshBuilder.h"
#include "DetourCrowd.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include "geom.h"
#include "geomVertexFormat.h"
#include "geomVertexWriter.h"
#include "geomTrifans.h"


#ifdef WIN32
# define snprintf _snprintf
#endif

NavMeshBuilder::NavMeshBuilder() :
  _nav_mesh(0),
  _nav_query(0),
  _crowd(0),
  _filter_low_hanging_obstacles(true),
  _filter_ledge_spans(true),
  _filter_walkable_low_height_spans(true),
  _ctx(0),
  _triareas(0),
  _solid(0),
  _chf(0),
  _cset(0),
  _pmesh(0),
  _dmesh(0),
  _scale(1.0f),
  _verts(0),
  _tris(0),
  _normals(0),
  _vert_count(0),
  _tri_count(0) {
  mp.clear();
  vc.clear();
  fc.clear();
  _ctx = new rcContext;
  reset_common_settings();
  _nav_query = dtAllocNavMeshQuery();
  _crowd = dtAllocCrowd();
}

NavMeshBuilder::~NavMeshBuilder() {
  delete[] _verts;
  delete[] _normals;
  delete[] _tris;

  dtFreeNavMeshQuery(_nav_query);
  dtFreeCrowd(_crowd);

  cleanup();

}


void NavMeshBuilder::add_vertex(float x, float y, float z, int &cap) {
  if (_vert_count + 1 > cap) {
    cap = !cap ? 8 : cap * 2;
    float *nv = new float[cap * 3];
    if (_vert_count) {
      memcpy(nv, _verts, _vert_count * 3 * sizeof(float));
    }
    delete[] _verts;
    _verts = nv;
  }
  float *dst = &_verts[_vert_count * 3];
  *dst++ = x * _scale;
  *dst++ = y * _scale;
  *dst++ = z * _scale;
  _vert_count++;
}

void NavMeshBuilder::add_triangle(int a, int b, int c, int &cap) {
  if (_tri_count + 1 > cap) {
    cap = !cap ? 8 : cap * 2;
    int *nv = new int[cap * 3];
    if (_tri_count) {
      memcpy(nv, _tris, _tri_count * 3 * sizeof(int));
    }
    delete[] _tris;
    _tris = nv;
  }
  int *dst = &_tris[_tri_count * 3];
  *dst++ = a;
  *dst++ = b;
  *dst++ = c;
  _tri_count++;
}

bool NavMeshBuilder::from_node_path(NodePath node) {
  NodePathCollection geom_node_collection = node.find_all_matches("**/+GeomNode");

  int vcap = 0;
  int tcap = 0;

  for (size_t i = 0; i < geom_node_collection.get_num_paths(); ++i) {

    PT(GeomNode) g = DCAST(GeomNode, geom_node_collection.get_path(i).node());
    process_geom_node(g, vcap, tcap);

  }
  loaded = true;
  return true;
}

void NavMeshBuilder::process_primitive(const GeomPrimitive *orig_prim, const GeomVertexData *vdata, int &tcap) {

  GeomVertexReader vertex(vdata, "vertex");

  CPT(GeomPrimitive) prim = orig_prim->decompose();

  for (size_t k = 0; k < prim->get_num_primitives(); ++k) {

    int s = prim->get_primitive_start(k);
    int e = prim->get_primitive_end(k);

    LVector3 v;
    if (e - s == 3) {
      int a = prim->get_vertex(s);
      vertex.set_row(a);
      v = vertex.get_data3();
      a = mp[v];
      int b = prim->get_vertex(s + 1);
      vertex.set_row(b);
      v = vertex.get_data3();
      b = mp[v];

      int c = prim->get_vertex(s + 2);
      vertex.set_row(c);
      v = vertex.get_data3();
      c = mp[v];


      LVector3 xvx = { float(a + 1), float(b + 1), float(c + 1) };
      fc.push_back(xvx);
      add_triangle(a, b, c, tcap);
    }
    else if (e - s > 3) {

      for (int i = s + 2; i < e; ++i) {
        int a = prim->get_vertex(s);
        vertex.set_row(a);
        v = vertex.get_data3();
        a = mp[v];

        int b = prim->get_vertex(i - 1);
        vertex.set_row(b);
        v = vertex.get_data3();
        b = mp[v];

        int c = prim->get_vertex(i);
        vertex.set_row(c);
        v = vertex.get_data3();
        c = mp[v];

        LVector3 xvx = { float(a + 1), float(b + 1), float(c + 1) };
        fc.push_back(xvx);
        add_triangle(a, b, c, tcap);
      }
    }
    else continue;

  }
  return;
}

void NavMeshBuilder::process_vertex_data(const GeomVertexData *vdata, int &vcap) {
  GeomVertexReader vertex(vdata, "vertex");
  float x, y, z;

  while (!vertex.is_at_end()) {

    LVector3 v = vertex.get_data3();
    x = v[0];
    y = v[1];
    z = v[2];
    if (mp.find(v) == mp.end()) {
      //add_vertex(x, z, -y, vcap); //if input model is originally z-up
      add_vertex(x, y, z, vcap); //if input model is originally y-up
      mp[v] = index_temp++;
      LVector3 xvx = { v[0],v[2],-v[1] };
      vc.push_back(xvx);
    }

  }
  return;
}

void NavMeshBuilder::process_geom(CPT(Geom) geom, int& vcap, int& tcap) {

  CPT(GeomVertexData) vdata = geom->get_vertex_data();

  process_vertex_data(vdata, vcap);

  for (size_t i = 0; i < geom->get_num_primitives(); ++i) {
    CPT(GeomPrimitive) prim = geom->get_primitive(i);
    process_primitive(prim, vdata, tcap);
  }
  return;
}

void NavMeshBuilder::process_geom_node(GeomNode *geomnode, int &vcap, int &tcap) {


  if (geomnode->get_num_geoms() > 1) {
    std::cout << "Cannot proceed: Make sure number of geoms = 1\n";
    return;
  }
  for (size_t j = 0; j < geomnode->get_num_geoms(); ++j) {
    CPT(Geom) geom = geomnode->get_geom(j);
    process_geom(geom, vcap, tcap);
  }
  return;
}





void NavMeshBuilder::cleanup()
{
  delete[] _triareas;
  _triareas = 0;
  rcFreeHeightField(_solid);
  _solid = 0;
  rcFreeCompactHeightfield(_chf);
  _chf = 0;
  rcFreeContourSet(_cset);
  _cset = 0;
  rcFreePolyMesh(_pmesh);
  _pmesh = 0;
  rcFreePolyMeshDetail(_dmesh);
  _dmesh = 0;
  dtFreeNavMesh(_nav_mesh);
  _nav_mesh = 0;
}

void NavMeshBuilder::set_partition_type(std::string p) {
  if (p == "watershed" || p == "Watershed" || p == "WATERSHED") {
    _partition_type = SAMPLE_PARTITION_WATERSHED;
    return;
  }
  if (p == "monotone" || p == "Monotone" || p == "MONOTONE") {
    _partition_type = SAMPLE_PARTITION_MONOTONE;
    return;
  }
  if (p == "layer" || p == "Layer" || p == "LAYER") {
    _partition_type = SAMPLE_PARTITION_LAYERS;
    return;
  }
}

void NavMeshBuilder::collect_settings(BuildSettings& settings)
{
  settings.cell_size = _cell_size;
  settings.cell_height = _cell_height;
  settings.agent_height = _agent_height;
  settings.agent_radius = _agent_radius;
  settings.agent_max_climb = _agent_max_climb;
  settings.agent_max_slope = _agent_max_slope;
  settings.region_min_size = _region_min_size;
  settings.region_merge_size = _region_merge_size;
  settings.edge_max_len = _edge_max_len;
  settings.edge_max_error = _edge_max_error;
  settings.verts_per_poly = _verts_per_poly;
  settings.detail_sample_dist = _detail_sample_dist;
  settings.detail_sample_max_error = _detail_sample_max_error;
  settings.partition_type = _partition_type;
}

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

bool NavMeshBuilder::build() {
  if (!loaded_geom()) {

    _ctx->log(RC_LOG_ERROR, "buildNavigation: Input mesh is not specified.");
    std::cout << "\nbuildNavigation: Input mesh is not specified.\n";

    return false;
  }

  cleanup();

  rcCalcBounds(get_verts(), get_vert_count(), _mesh_bMin, _mesh_bMax);

  const float *bmin = _mesh_bMin;
  const float *bmax = _mesh_bMax;
  const float *verts = get_verts();
  const int nverts = get_vert_count();
  const int *tris = get_tris();
  const int ntris = get_tri_count();
  std::cout << "BMIN: " << bmin[0] << " " << bmin[1] << std::endl;
  std::cout << "BMAX: " << bmax[0] << " " << bmax[1] << std::endl;
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

  _ctx->log(RC_LOG_PROGRESS, "Building navigation:");
  std::cout << "\nBuilding navigation:\n";
  _ctx->log(RC_LOG_PROGRESS, " - %d x %d cells", _cfg.width, _cfg.height);
  std::cout << "\n - " << _cfg.width << " x " << _cfg.height << " cells\n";
  _ctx->log(RC_LOG_PROGRESS, " - %.1fK verts, %.1fK tris", nverts / 1000.0f, ntris / 1000.0f);
  std::cout << "\n - " << nverts / 1000.0f << "K vetrs, " << ntris / 1000.0f << "K tris\n";

  //
  // Step 2. Rasterize input polygon soup.
  //

  // Allocate voxel heightfield where we rasterize our input data to.
  _solid = rcAllocHeightfield();
  if (!_solid) {
    _ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'solid'.");
    std::cout << "\nbuildNavigation: Out of memory 'solid'.\n";
    return false;
  }
  if (!rcCreateHeightfield(_ctx, *_solid, _cfg.width, _cfg.height, _cfg.bmin, _cfg.bmax, _cfg.cs, _cfg.ch)) {
    _ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create solid heightfield.");
    std::cout << "\nbuildNavigation: Could not create solid heightfield.\n";
    return false;
  }

  // Allocate array that can hold triangle area types.
  // If you have multiple meshes you need to process, allocate
  // and array which can hold the max number of triangles you need to process.
  _triareas = new unsigned char[ntris];
  if (!_triareas) {
    _ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory '_triareas' (%d).", ntris);
    std::cout << "\nbuildNavigation: Out of memory '_triareas' (" << ntris << ").\n";
    return false;
  }

  // Find triangles which are walkable based on their slope and rasterize them.
  // If your input data is multiple meshes, you can transform them here, calculate
  // the are type for each of the meshes and rasterize them.
  memset(_triareas, 0, ntris * sizeof(unsigned char));
  rcMarkWalkableTriangles(_ctx, _cfg.walkableSlopeAngle, verts, nverts, tris, ntris, _triareas);
  if (!rcRasterizeTriangles(_ctx, verts, nverts, tris, _triareas, ntris, *_solid, _cfg.walkableClimb)) {
    _ctx->log(RC_LOG_ERROR, "buildNavigation: Could not rasterize triangles.");
    std::cout << "\nbuildNavigation: Could not rasterize triangles.\n";
    return false;
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
    _ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'chf'.");
    std::cout << "\nbuildNavigation: Out of memory 'chf'.\n";
    return false;
  }
  if (!rcBuildCompactHeightfield(_ctx, _cfg.walkableHeight, _cfg.walkableClimb, *_solid, *_chf)) {
    _ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build compact data.");
    std::cout << "\nbuildNavigation: Could not build compact data.\n";
    return false;
  }


  // Erode the walkable area by agent radius.
  if (!rcErodeWalkableArea(_ctx, _cfg.walkableRadius, *_chf)) {
    _ctx->log(RC_LOG_ERROR, "buildNavigation: Could not erode.");
    std::cout << "\nbuildNavigation: Could not erode.\n";
    return false;
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

  if (_partition_type == SAMPLE_PARTITION_WATERSHED) {
    // Prepare for region partitioning, by calculating distance field along the walkable surface.
    if (!rcBuildDistanceField(_ctx, *_chf)) {
      _ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build distance field.");
      std::cout << "\nbuildNavigation: Could not build distance field.\n";
      return false;
    }

    // Partition the walkable surface into simple regions without holes.
    if (!rcBuildRegions(_ctx, *_chf, 0, _cfg.minRegionArea, _cfg.mergeRegionArea)) {
      _ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build watershed regions.");
      std::cout << "\nbuildNavigation: Could not build watershed regions.\n";
      return false;
    }
  }
  else if (_partition_type == SAMPLE_PARTITION_MONOTONE) {
    // Partition the walkable surface into simple regions without holes.
    // Monotone partitioning does not need distancefield.
    if (!rcBuildRegionsMonotone(_ctx, *_chf, 0, _cfg.minRegionArea, _cfg.mergeRegionArea)) {
      _ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build monotone regions.");
      std::cout << "\nbuildNavigation: Could not build monotone regions.\n";
      return false;
    }
  }
  else { // SAMPLE_PARTITION_LAYERS
    // Partition the walkable surface into simple regions without holes.
    if (!rcBuildLayerRegions(_ctx, *_chf, 0, _cfg.minRegionArea)) {
      _ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build layer regions.");
      std::cout << "\nbuildNavigation: Could not build layer regions.\n";
      return false;
    }
  }

  //
  // Step 5. Trace and simplify region contours.
  //

  // Create contours.
  _cset = rcAllocContourSet();
  if (!_cset) {
    _ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'cset'.");
    std::cout << "\nbuildNavigation: Out of memory 'cset'.\n";
    return false;
  }
  if (!rcBuildContours(_ctx, *_chf, _cfg.maxSimplificationError, _cfg.maxEdgeLen, *_cset)) {
    _ctx->log(RC_LOG_ERROR, "buildNavigation: Could not create contours.");
    std::cout << "\nbuildNavigation: Could not create contours.\n";
    return false;
  }

  //
  // Step 6. Build polygons mesh from contours.
  //

  // Build polygon navmesh from the contours.
  _pmesh = rcAllocPolyMesh();
  if (!_pmesh) {
    _ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmesh'.");
    std::cout << "\nbuildNavigation: Out of memory 'pmesh'.\n";
    return false;
  }
  if (!rcBuildPolyMesh(_ctx, *_cset, _cfg.maxVertsPerPoly, *_pmesh)) {
    _ctx->log(RC_LOG_ERROR, "buildNavigation: Could not triangulate contours.");
    std::cout << "\nbuildNavigation: Could not triangulate contours.\n";
    return false;
  }

  //
  // Step 7. Create detail mesh which allows to access approximate height on each polygon.
  //

  _dmesh = rcAllocPolyMeshDetail();
  if (!_dmesh) {
    _ctx->log(RC_LOG_ERROR, "buildNavigation: Out of memory 'pmdtl'.");
    std::cout << "\nbuildNavigation: Out of memory 'pmdt1'.\n";
    return false;
  }

  if (!rcBuildPolyMeshDetail(_ctx, *_pmesh, *_chf, _cfg.detailSampleDist, _cfg.detailSampleMaxError, *_dmesh)) {
    _ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build detail mesh.");
    std::cout << "\nbuildNavigation: Could not build detail mesh.\n";
    return false;
  }
  std::cout << "Number of vertices: " << _pmesh->nverts << std::endl;
  std::cout << "Number of polygons: " << _pmesh->npolys << std::endl;
  std::cout << "Number of allocated polygons: " << _pmesh->maxpolys << std::endl;

  // At this point the navigation mesh data is ready, you can access it from _pmesh.
  // See duDebugDrawPolyMesh or dtCreateNavMeshData as examples how to access the data.


  //
  // (Optional) Step 8. Create Detour data from Recast poly mesh.
  //
  std::cout << "\nSample_SoloMesh::handleBuild() : (Optional) Step 8. Create Detour data from Recast poly mesh.\n";
  // The GUI may allow more max points per polygon than Detour can handle.
  // Only build the detour navmesh if we do not exceed the limit.
  if (_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON) {
    unsigned char *nav_data = 0;
    int nav_data_size = 0;

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
    rcVcopy(params.bmin, _pmesh->bmin);
    rcVcopy(params.bmax, _pmesh->bmax);
    params.cs = _cfg.cs;
    params.ch = _cfg.ch;
    params.buildBvTree = true;

    if (!dtCreateNavMeshData(&params, &nav_data, &nav_data_size)) {
      _ctx->log(RC_LOG_ERROR, "Could not build Detour navmesh.");
      return false;
    }

    _nav_mesh = dtAllocNavMesh();

    if (!_nav_mesh) {
      dtFree(nav_data);
      _ctx->log(RC_LOG_ERROR, "Could not create Detour navmesh");
      return false;
    }

    dtStatus status;

    status = _nav_mesh->init(nav_data, nav_data_size, DT_TILE_FREE_DATA);
    if (dtStatusFailed(status)) {
      dtFree(nav_data);
      _ctx->log(RC_LOG_ERROR, "Could not init Detour navmesh");
      return false;
    }

    status = _nav_query->init(_nav_mesh, 2048);
    if (dtStatusFailed(status)) {
      _ctx->log(RC_LOG_ERROR, "Could not init Detour navmesh query");
      return false;
    }

  }


  _ctx->stopTimer(RC_TIMER_TOTAL);

  // Show performance stats.
  _ctx->log(RC_LOG_PROGRESS, ">> Polymesh: %d vertices  %d polygons", _pmesh->nverts, _pmesh->npolys);

  _total_build_time_ms = _ctx->getAccumulatedTime(RC_TIMER_TOTAL) / 1000.0f;

  //if (_tool)
    //_tool->init(this);
  //initToolStates(this);
  //std::cout << "\nExiting Sample_SoloMesh::handleBuild()\n";
  return true;
}


NavMesh NavMeshBuilder::get_navmesh() {
  NavMesh nav(_nav_mesh, _pmesh, _dmesh);
  return nav;
}
