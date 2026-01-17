/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file navMeshBuilder.h
 * @author Ashwani / 
 * @date 2024
 */

#ifndef NAVMESHBUILDER_H
#define NAVMESHBUILDER_H

#include "pandabase.h"
#include "config_navmesh.h" // Added config include
#include "navMeshSettings.h"
#include "navMesh.h"
#include "nodePath.h"
#include "pointerTo.h"
#include "pvector.h"
#include "luse.h"

/**
 * This class is responsible for building a navigation mesh from the scene graph.
 * It traverses the geometry, extracts triangles, and invokes the Recast
 * generation process.
 */
class EXPCL_PANDA_NAVMESH NavMeshBuilder {
PUBLISHED:
  NavMeshBuilder();
  ~NavMeshBuilder();

  // Builds a navigation mesh from the given scene root using the provided settings.
  // Returns a pointer to the generated NavMeshPoly, or nullptr if generation failed.
  static PT(NavMeshPoly) build(const NodePath &scene_root, const NavMeshSettings &settings);

private:
  // Internal structure to hold raw geometry for Recast
  struct RawGeometry {
    pvector<PN_stdfloat> vertices; // x, y, z packed (Panda precision)
    pvector<int> indices;          // v1, v2, v3...

    void add_triangle(const LPoint3 &p1, const LPoint3 &p2, const LPoint3 &p3) {
      int base = vertices.size() / 3;
      vertices.push_back(p1[0]); vertices.push_back(p1[1]); vertices.push_back(p1[2]);
      vertices.push_back(p2[0]); vertices.push_back(p2[1]); vertices.push_back(p2[2]);
      vertices.push_back(p3[0]); vertices.push_back(p3[1]); vertices.push_back(p3[2]);

      indices.push_back(base);
      indices.push_back(base + 1);
      indices.push_back(base + 2);
    }
  };

  // Helper to recursively extract geometry from the scene graph
  static void r_extract_geometry(PandaNode *node, const LMatrix4 &transform,
                                 RawGeometry &out_geom);
};

#endif // NAVMESHBUILDER_H
