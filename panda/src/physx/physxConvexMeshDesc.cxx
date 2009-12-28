// Filename: physxConvexMeshDesc.cxx
// Created by:  enn0x (11Oct09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "physxConvexMeshDesc.h"
#include "physxManager.h"

#include "nodePathCollection.h"
#include "geomNode.h"
#include "geomVertexReader.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxConvexMeshDesc::set_num_vertices
//       Access: Published
//  Description: Sets the number of vertices to be stored within
//               this convex mesh. The function allocates memory
//               for the vertices, but it does not set any vertices.
//
//               This method must be called before any calls to
//               set_vertex are done!
//
//               The number of vertices in a single convex mesh has
//               to be smaller than 256.
////////////////////////////////////////////////////////////////////
void PhysxConvexMeshDesc::
set_num_vertices(unsigned int numVertices) {

  nassertv_always(numVertices < 256);

  if (_desc.points) {
    delete [] _vertices;
  }

  _vertices = new NxVec3[numVertices];

  _desc.numVertices = numVertices;
  _desc.points = _vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxConvexMeshDesc::set_vertex
//       Access: Published
//  Description: Sets a single vertex. You have to call the function
//               set_num_vertices before you can call this function.
////////////////////////////////////////////////////////////////////
void PhysxConvexMeshDesc::
set_vertex(unsigned int idx, const LPoint3f &vert) {

  nassertv(_desc.numVertices > idx);
  _vertices[idx] = PhysxManager::point3_to_nxVec3(vert);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxConvexMeshDesc::get_desc
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
const NxConvexMeshDesc &PhysxConvexMeshDesc::
get_desc() const {

  return _desc;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxConvexMeshDesc::set_from_node_path
//       Access: Published
//  Description: A convenience method to set the mesh data from
//               a NodePath in a single call. The method iterates
//               over the NodePath geoms and collects data for
//               the convex mesh.
//
//               Do not use the following function when using this
//               one:
//               - set_num_vertices
//               - set_vertex
////////////////////////////////////////////////////////////////////
void PhysxConvexMeshDesc::
set_from_node_path(const NodePath &np) {

  pvector<LPoint3f> dataVertices;

  // Collect data from NodePath
  NodePathCollection npc = np.find_all_matches( "**/+GeomNode" );
  for (int i=0; i<npc.get_num_paths(); i++) {
    NodePath gnp = npc.get_path(i);
    GeomNode *gnode = DCAST(GeomNode, gnp.node());

    for (int j=0; j<gnode->get_num_geoms(); j++) {
      CPT(Geom) geom = gnode->get_geom(j);
      CPT(GeomVertexData) vdata = geom->get_vertex_data();
      GeomVertexReader reader = GeomVertexReader(vdata, InternalName::get_vertex());

      while (!reader.is_at_end()) {
        dataVertices.push_back(reader.get_data3f());
      }
    }
  }

  // Set descriptor members
  int i;

  NxU32 numVertices = dataVertices.size();

  _vertices = new NxVec3[numVertices];

  i = 0;
  pvector<LPoint3f>::const_iterator it;
  for (it=dataVertices.begin(); it!=dataVertices.end(); it++) {
    LPoint3f v = *it;

    _vertices[i].x = v.get_x();
    _vertices[i].y = v.get_y();
    _vertices[i].z = v.get_z();
    i++;
  }

  _desc.numVertices = numVertices;
  _desc.points = _vertices;
}

