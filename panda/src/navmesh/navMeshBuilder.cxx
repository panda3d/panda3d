/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshBuilder.cxx
 * @author Ashwani / 
 * @date 2024
 */

#include "navMeshBuilder.h"
#include "config_navmesh.h"
#include "geomNode.h"
#include "geom.h"
#include "geomPrimitive.h"
#include "geomTriangles.h"
#include "geomVertexReader.h"
#include "geomVertexWriter.h"
#include "transformState.h"

// Recast/Detour includes - Hide from Interrogate
#ifndef CPPPARSER
#include <Recast.h>
#include <RecastAssert.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include <DetourAssert.h>

// Implement Recast/Detour assertion hooks
static rcAssertFailFunc* sRecastAssertFailFunc = nullptr;
void rcAssertFailSetCustom(rcAssertFailFunc* func) { sRecastAssertFailFunc = func; }
rcAssertFailFunc* rcAssertFailGetCustom() { return sRecastAssertFailFunc; }

static dtAssertFailFunc* sDetourAssertFailFunc = nullptr;
void dtAssertFailSetCustom(dtAssertFailFunc* func) { sDetourAssertFailFunc = func; }
dtAssertFailFunc* dtAssertFailGetCustom() { return sDetourAssertFailFunc; }

/**
 * Custom Recast Context to redirect logs to Panda's Notify system
 */
class PandaRecastContext : public rcContext {
public:
  virtual void doLog(const rcLogCategory category, const char* msg, const int len) {
    if (category == RC_LOG_ERROR) {
      navmesh_cat.error() << msg << "\n";
    } else if (category == RC_LOG_WARNING) {
      navmesh_cat.warning() << msg << "\n";
    }
  }
};
#endif // CPPPARSER

/**
 * Constructor
 */
NavMeshBuilder::
NavMeshBuilder() {
}

/**
 * Destructor
 */
NavMeshBuilder::
~NavMeshBuilder() {
}

/**
 * Builds a navigation mesh from the given scene root using the provided settings.
 */
PT(NavMeshPoly) NavMeshBuilder::
build(const NodePath &scene_root, const NavMeshSettings &settings) {
  if (scene_root.is_empty()) {
    navmesh_cat.error() << "Cannot build navmesh from empty NodePath.\n";
    return nullptr;
  }

  if (!settings.validate()) {
    navmesh_cat.error() << "Invalid NavMeshSettings.\n";
    return nullptr;
  }

  // 1. Extract Geometry (World Space, Z-Up)
  RawGeometry geom;
  LMatrix4 net_transform = scene_root.get_net_transform()->get_mat();
  r_extract_geometry(scene_root.node(), net_transform, geom);

  if (geom.vertices.empty()) {
    navmesh_cat.warning() << "No geometry found for navmesh generation.\n";
    return nullptr;
  }

#ifndef CPPPARSER
  int num_verts = geom.vertices.size() / 3;
  int num_tris = geom.indices.size() / 3;
  
  // FIX 1: Float/Double Precision
  // Recast requires float* (single precision), so we must allocate a temporary buffer
  // if PN_stdfloat is double. Even if it is float, a dedicated vector is safer for ownership.
  pvector<float> recast_verts;
  recast_verts.resize(geom.vertices.size());

  // 2. Coordinate Swizzle (Z-Up -> Y-Up) AND Precision Conversion
  // Recast expects Y as up. We rotate -90 around X.
  // (x, y, z) -> (x, z, -y)
  for (size_t i = 0; i < geom.vertices.size(); i += 3) {
    PN_stdfloat x = geom.vertices[i];
    PN_stdfloat y = geom.vertices[i + 1];
    PN_stdfloat z = geom.vertices[i + 2];
    
    recast_verts[i] = (float)x;
    recast_verts[i + 1] = (float)z;     // Y = Z
    recast_verts[i + 2] = (float)(-y);  // Z = -Y
  }

  // 3. Setup Recast Config
  rcConfig cfg;
  memset(&cfg, 0, sizeof(cfg));
  cfg.cs = (float)settings.get_cell_size();
  cfg.ch = (float)settings.get_cell_height();
  cfg.walkableSlopeAngle = (float)settings.get_agent_max_slope();
  cfg.walkableHeight = (int)ceilf((float)settings.get_agent_height() / cfg.ch);
  cfg.walkableClimb = (int)floorf((float)settings.get_agent_max_climb() / cfg.ch);
  cfg.walkableRadius = (int)ceilf((float)settings.get_agent_radius() / cfg.cs);
  cfg.maxEdgeLen = (int)((float)settings.get_edge_max_len() / cfg.cs);
  cfg.maxSimplificationError = (float)settings.get_edge_max_error();
  cfg.minRegionArea = (int)rcSqr((float)settings.get_region_min_size());
  cfg.mergeRegionArea = (int)rcSqr((float)settings.get_region_merge_size());
  cfg.maxVertsPerPoly = settings.get_verts_per_poly();
  cfg.detailSampleDist = (float)settings.get_detail_sample_dist() < 0.9f ? 0 : (float)settings.get_cell_size() * (float)settings.get_detail_sample_dist();
  cfg.detailSampleMaxError = (float)settings.get_cell_height() * (float)settings.get_detail_sample_max_error();

  rcCalcBounds(&recast_verts[0], num_verts, cfg.bmin, cfg.bmax);
  rcCalcGridSize(cfg.bmin, cfg.bmax, cfg.cs, &cfg.width, &cfg.height);

  // RAII Cleanup
  struct RecastCleanup {
    rcHeightfield* hf = nullptr;
    rcCompactHeightfield* chf = nullptr;
    rcContourSet* cset = nullptr;
    rcPolyMesh* pmesh = nullptr;
    rcPolyMeshDetail* dmesh = nullptr;
    ~RecastCleanup() {
      rcFreeHeightField(hf);
      rcFreeCompactHeightfield(chf);
      rcFreeContourSet(cset);
      rcFreePolyMesh(pmesh);
      rcFreePolyMeshDetail(dmesh);
    }
  } rc;

  PandaRecastContext ctx;

  // 4. Recast Pipeline Steps
  rc.hf = rcAllocHeightfield();
  if (!rcCreateHeightfield(&ctx, *rc.hf, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch)) {
    navmesh_cat.error() << "Could not create Recast heightfield.\n";
    return nullptr;
  }

  // Rasterize
  pvector<unsigned char> tri_areas(num_tris, RC_WALKABLE_AREA);
  rcRasterizeTriangles(&ctx, &recast_verts[0], num_verts, &geom.indices[0], &tri_areas[0], num_tris, *rc.hf, cfg.walkableClimb);

  rcFilterLowHangingWalkableObstacles(&ctx, cfg.walkableClimb, *rc.hf);
  rcFilterLedgeSpans(&ctx, cfg.walkableHeight, cfg.walkableClimb, *rc.hf);
  rcFilterWalkableLowHeightSpans(&ctx, cfg.walkableHeight, *rc.hf);

  rc.chf = rcAllocCompactHeightfield();
  if (!rcBuildCompactHeightfield(&ctx, cfg.walkableHeight, cfg.walkableClimb, *rc.hf, *rc.chf)) {
    navmesh_cat.error() << "Could not build compact heightfield.\n";
    return nullptr;
  }

  if (!rcErodeWalkableArea(&ctx, cfg.walkableRadius, *rc.chf)) {
    navmesh_cat.error() << "Could not erode walkable area.\n";
    return nullptr;
  }

  if (!rcBuildDistanceField(&ctx, *rc.chf)) {
    navmesh_cat.error() << "Could not build distance field.\n";
    return nullptr;
  }

  if (!rcBuildRegions(&ctx, *rc.chf, 0, cfg.minRegionArea, cfg.mergeRegionArea)) {
    navmesh_cat.error() << "Could not build regions.\n";
    return nullptr;
  }

  rc.cset = rcAllocContourSet();
  if (!rcBuildContours(&ctx, *rc.chf, cfg.maxSimplificationError, cfg.maxEdgeLen, *rc.cset)) {
    navmesh_cat.error() << "Could not build contours.\n";
    return nullptr;
  }

  rc.pmesh = rcAllocPolyMesh();
  if (!rcBuildPolyMesh(&ctx, *rc.cset, cfg.maxVertsPerPoly, *rc.pmesh)) {
    navmesh_cat.error() << "Could not build poly mesh.\n";
    return nullptr;
  }

  rc.dmesh = rcAllocPolyMeshDetail();
  if (!rcBuildPolyMeshDetail(&ctx, *rc.pmesh, *rc.chf, cfg.detailSampleDist, cfg.detailSampleMaxError, *rc.dmesh)) {
    navmesh_cat.error() << "Could not build detail poly mesh.\n";
    return nullptr;
  }

  static const unsigned short NAVMESH_POLYAREA_GROUND = 0x01;
  static const unsigned short NAVMESH_POLYFLAGS_WALK = 0x01;
  for (int i = 0; i < rc.pmesh->npolys; ++i) {
    if (rc.pmesh->areas[i] == RC_WALKABLE_AREA) {
      rc.pmesh->areas[i] = NAVMESH_POLYAREA_GROUND;
    }
    rc.pmesh->flags[i] = NAVMESH_POLYFLAGS_WALK;
  }

  // 5. Build Detour Data
  dtNavMeshCreateParams params;
  memset(&params, 0, sizeof(params));
  params.verts = rc.pmesh->verts;
  params.vertCount = rc.pmesh->nverts;
  params.polys = rc.pmesh->polys;
  params.polyAreas = rc.pmesh->areas;
  params.polyFlags = rc.pmesh->flags;
  params.polyCount = rc.pmesh->npolys;
  params.nvp = rc.pmesh->nvp;
  params.detailMeshes = rc.dmesh->meshes;
  params.detailVerts = rc.dmesh->verts;
  params.detailVertsCount = rc.dmesh->nverts;
  params.detailTris = rc.dmesh->tris;
  params.detailTriCount = rc.dmesh->ntris;

  // No off-mesh connections for v1
  params.offMeshConVerts = nullptr;
  params.offMeshConRad = nullptr;
  params.offMeshConDir = nullptr;
  params.offMeshConAreas = nullptr;
  params.offMeshConFlags = nullptr;
  params.offMeshConUserID = nullptr;
  params.offMeshConCount = 0;

  params.walkableHeight = (float)settings.get_agent_height();
  params.walkableRadius = (float)settings.get_agent_radius();
  params.walkableClimb = (float)settings.get_agent_max_climb();
  rcVcopy(params.bmin, rc.pmesh->bmin);
  rcVcopy(params.bmax, rc.pmesh->bmax);
  params.cs = cfg.cs;
  params.ch = cfg.ch;
  params.buildBvTree = true;

  unsigned char* navData = nullptr;
  int navDataSize = 0;
  if (!dtCreateNavMeshData(&params, &navData, &navDataSize)) {
    navmesh_cat.error() << "Could not build Detour navmesh data.\n";
    return nullptr;
  }

  PT(NavMeshPoly) mesh = new NavMeshPoly();
  mesh->setup(navData, navDataSize);
  
  return mesh;
#else
  // Fallback for Interrogate (should never happen at runtime)
  return nullptr;
#endif
}

/**
 * Helper to recursively extract geometry from the scene graph.
 */
void NavMeshBuilder::
r_extract_geometry(PandaNode *node, const LMatrix4 &transform,
                   RawGeometry &out_geom) {
  if (node->is_geom_node()) {
    GeomNode *gnode = (GeomNode *)node;
    for (int i = 0; i < gnode->get_num_geoms(); ++i) {
      CPT(Geom) geom = gnode->get_geom(i);
      
      // Iterate primitives
      for (int k = 0; k < geom->get_num_primitives(); ++k) {
        CPT(GeomPrimitive) prim = geom->get_primitive(k);
        
        // Decompose complex primitives (strips, fans) into triangles
        CPT(GeomPrimitive) tri_prim = prim->decompose();
        
        // Read vertices via GeomVertexReader
        CPT(GeomVertexData) vdata = geom->get_vertex_data();
        GeomVertexReader reader(vdata, "vertex");
        
        for (int p = 0; p < tri_prim->get_num_primitives(); ++p) {
          int s = tri_prim->get_primitive_start(p);
          int e = tri_prim->get_primitive_end(p);
          
          // We expect triangles, so we should get 3 vertices
          if (e - s < 3) continue;
          
          int i1 = tri_prim->get_vertex(s);
          int i2 = tri_prim->get_vertex(s + 1);
          int i3 = tri_prim->get_vertex(s + 2);
          
          reader.set_row(i1); LPoint3 p1 = reader.get_data3f();
          reader.set_row(i2); LPoint3 p2 = reader.get_data3f();
          reader.set_row(i3); LPoint3 p3 = reader.get_data3f();
          
          // Apply transform and add
          out_geom.add_triangle(p1 * transform, p2 * transform, p3 * transform);
        }
      }
    }
  }

  // Recurse to children
  PandaNode::Children children = node->get_children();
  for (int i = 0; i < children.get_num_children(); ++i) {
    PandaNode *child = children.get_child(i);
    // Accumulate transform (parent first, then child's local transform)
    // Note: Panda3D uses row-vector matrices, so Local * Parent = World
    LMatrix4 child_transform = child->get_transform()->get_mat() * transform;
    r_extract_geometry(child, child_transform, out_geom);
  }
}
