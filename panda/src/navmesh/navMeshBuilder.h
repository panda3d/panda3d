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
#include "navMeshSettings.h"
#include "navMesh.h"
#include "nodePath.h"
#include "pointerTo.h"

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
  // Returns a pointer to the generated NavMesh, or nullptr if generation failed.
  PT(NavMesh) build(const NodePath &scene_root, const NavMeshSettings &settings);

private:
  // Helper to recursively extract geometry from the scene graph
  void r_extract_geometry(PandaNode *node, const LMatrix4 &transform,
                          class GeomVertexData *vdata,
                          class GeomTriangles *triangles);
};

#endif // NAVMESHBUILDER_H

