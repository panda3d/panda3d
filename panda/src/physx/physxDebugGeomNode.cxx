/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxDebugGeomNode.cxx
 * @author enn0x
 * @date 2009-09-15
 */

#include "physxDebugGeomNode.h"

#include "geomVertexFormat.h"
#include "geomVertexWriter.h"

TypeHandle PhysxDebugGeomNode::_type_handle;

/**
 *
 */
void PhysxDebugGeomNode::
update(NxScene *scenePtr) {

  if (get_num_parents() == 0) {
    return;
  }

  const NxDebugRenderable *renderable = scenePtr->getDebugRenderable();
  if (!renderable) {
    remove_all_geoms();
    physx_cat.warning() << "Could no get debug renderable." << std::endl;
    return;
  }

  GeomVertexWriter vwriter = GeomVertexWriter(_vdata, InternalName::get_vertex());
  GeomVertexWriter cwriter = GeomVertexWriter(_vdata, InternalName::get_color());

  int v = 0;

  _prim_lines->clear_vertices();
  _prim_triangles->clear_vertices();

  // Lines
  {
    NxU32 n = renderable->getNbLines();
    const NxDebugLine *lines = renderable->getLines();

    for (NxU32 i=0; i<n; i++)
    {
      NxF32 b = NxF32((lines[i].color)&0xff) / 255.0f;
      NxF32 g = NxF32((lines[i].color>>8)&0xff) / 255.0f;
      NxF32 r = NxF32((lines[i].color>>16)&0xff) / 255.0f;

      NxVec3 p0 = lines[i].p0;
      NxVec3 p1 = lines[i].p1;

      cwriter.add_data4f(r, g, b, 1.0f);
      vwriter.add_data3f(p0.x, p0.y, p0.z);
      _prim_lines->add_vertex(v++);

      cwriter.add_data4f(r, g, b, 1.0f);
      vwriter.add_data3f(p1.x, p1.y, p1.z);
      _prim_lines->add_vertex(v++);
    }

  }

  // Triangles
  {
    NxU32 n = renderable->getNbTriangles();
    const NxDebugTriangle *triangles = renderable->getTriangles();

    for (NxU32 i=0; i<n; i++)
    {
      NxF32 b = NxF32((triangles[i].color)&0xff) / 255.0f;
      NxF32 g = NxF32((triangles[i].color>>8)&0xff) / 255.0f;
      NxF32 r = NxF32((triangles[i].color>>16)&0xff) / 255.0f;

      NxVec3 p0 = triangles[i].p0;
      NxVec3 p1 = triangles[i].p1;
      NxVec3 p2 = triangles[i].p2;

      cwriter.add_data4f(r, g, b, 1.0f);
      vwriter.add_data3f(p0.x, p0.y, p0.z);
      _prim_triangles->add_vertex(v++);

      cwriter.add_data4f(r, g, b, 1.0f);
      vwriter.add_data3f(p1.x, p1.y, p1.z);
      _prim_triangles->add_vertex(v++);

      cwriter.add_data4f(r, g, b, 1.0f);
      vwriter.add_data3f(p2.x, p2.y, p2.z);
      _prim_triangles->add_vertex(v++);
    }
  }

  _prim_lines->close_primitive();
  _prim_triangles->close_primitive();

  if (physx_cat.is_spam()) {
    physx_cat.spam() << "Updated PhysxDebugGeomNode geometry\n";
  }
}

/**
 *
 */
void PhysxDebugGeomNode::
on() {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZATION_SCALE, _scale);
}

/**
 *
 */
void PhysxDebugGeomNode::
off() {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZATION_SCALE, 0.0f);
}

/**
 *
 */
void PhysxDebugGeomNode::
toggle() {

  if (NxGetPhysicsSDK()->getParameter(NX_VISUALIZATION_SCALE) == 0.0f) {
    on();
  }
  else {
    off();
  }
}
