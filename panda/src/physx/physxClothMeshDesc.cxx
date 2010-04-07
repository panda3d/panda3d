// Filename: physxClothMeshDesc.cxx
// Created by:  enn0x (28Mar10)
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

#include "physxClothMeshDesc.h"
#include "physxManager.h"

#include "nodePathCollection.h"
#include "geomNode.h"
#include "geomVertexReader.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothMeshDesc::set_num_vertices
//       Access: Published
//  Description: Sets the number of vertices to be stored within
//               this triangle mesh. The function allocates memory
//               for the vertices, but it does not set any vertices.
//
//               This method must be called before any calls to
//               set_vertex are done!
////////////////////////////////////////////////////////////////////
void PhysxClothMeshDesc::
set_num_vertices(unsigned int numVertices) {

  // Vertices
  if (_desc.points) {
    delete [] _points;
  }

  _points = new NxVec3[numVertices];

  _desc.numVertices = numVertices;
  _desc.points = _points;

  // Texture coordinates
  if (_texcoords) {
    delete [] _texcoords;
  }

  _texcoords = new LPoint2f[numVertices];
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothMeshDesc::set_vertex
//       Access: Published
//  Description: Sets a single vertex. You have to call the function
//               set_num_vertices before you can call this function.
////////////////////////////////////////////////////////////////////
void PhysxClothMeshDesc::
set_vertex(unsigned int idx, const LPoint3f &vert, const LPoint2f &texcoord) {

  nassertv(_desc.numVertices > idx);

  _points[idx] = PhysxManager::point3_to_nxVec3(vert);
  _texcoords[idx] = texcoord;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothMeshDesc::set_num_triangles
//       Access: Published
//  Description: Sets the number of triangles to be stored in this
//               triangle mesh.
//
//               This method must be called before any calls to
//               set_triangle are done!
////////////////////////////////////////////////////////////////////
void PhysxClothMeshDesc::
set_num_triangles(unsigned int numTriangles) {

  if (_desc.triangles) {
    delete [] _triangles;
  }

  _triangles = new NxU32[3 * numTriangles];

  _desc.numTriangles = numTriangles;
  _desc.triangles = _triangles;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothMeshDesc::set_triangles
//       Access: Published
//  Description: Sets a single triangle, by providing the three
//               indices i1, i2, i3.
////////////////////////////////////////////////////////////////////
void PhysxClothMeshDesc::
set_triangle(unsigned int idx,
             unsigned int i1, unsigned int i2, unsigned int i3) {

  nassertv(_desc.numTriangles > idx);

  idx = 3 * idx;
  _triangles[idx]     = i1;
  _triangles[idx + 1] = i2;
  _triangles[idx + 2] = i3;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothMeshDesc::set_from_node_path
//       Access: Published
//  Description: A convenience method to set the mesh data from
//               a NodePath in a single call. The method iterates
//               over the NodePath geoms and collects data for
//               the triangle mesh.
//
//               Do not use the following function when using this
//               one:
//               - set_num_vertices
//               - set_vertex
//               - set_num_triangles
//               - set_triangle
////////////////////////////////////////////////////////////////////
void PhysxClothMeshDesc::
set_from_node_path(const NodePath &np) {

  pvector<LPoint3f> dataVertices;
  pvector<LPoint2f> dataTexcoords;
  pvector<int> dataIndices;

  // Collect data from NodePath
  NodePathCollection npc = np.find_all_matches( "**/+GeomNode" );
  for (int i=0; i<npc.get_num_paths(); i++) {
    NodePath gnp = npc.get_path(i);
    GeomNode *gnode = DCAST(GeomNode, gnp.node());

    for (int j=0; j<gnode->get_num_geoms(); j++) {
      CPT(Geom) geom = gnode->get_geom(j);
      CPT(GeomVertexData) vdata = geom->get_vertex_data();
      GeomVertexReader reader;

      // Vertices
      reader = GeomVertexReader(vdata, InternalName::get_vertex());
      while (!reader.is_at_end()) {
        dataVertices.push_back(reader.get_data3f());
      }

      // Texcoords
      reader = GeomVertexReader(vdata, InternalName::get_texcoord());
      while (!reader.is_at_end()) {
        dataTexcoords.push_back(reader.get_data2f());
      }

      // Indices
      for (int k=0; k<geom->get_num_primitives(); k++) {

        CPT(GeomPrimitive) prim = geom->get_primitive(k);
        prim = prim->decompose();

        for (int l=0; l<prim->get_num_primitives(); l++) {

          int s = prim->get_primitive_start(l);
          int e = prim->get_primitive_end(l);

          for (int l=s; l<e; l++) {
            dataIndices.push_back(prim->get_vertex(l));
          }
        }
      }
    }
  }

  // Set descriptor members
  int i;

  NxU32 numVertices = dataVertices.size();
  NxU32 numTriangles = dataIndices.size() / 3;

  _points = new NxVec3[numVertices];
  _triangles = new NxU32[3 * numTriangles];
  _texcoords = new LPoint2f[numVertices];

  i = 0;
  pvector<LPoint3f>::const_iterator vit;
  for (vit=dataVertices.begin(); vit!=dataVertices.end(); vit++) {
    LPoint3f v = *vit;

    _points[i] = PhysxManager::point3_to_nxVec3(v);
    i++;
  }

  i = 0;
  pvector<LPoint2f>::const_iterator tcit;
  for (tcit=dataTexcoords.begin(); tcit!=dataTexcoords.end(); tcit++) {
    LPoint2f tc = *tcit;

    _texcoords[i]   = tc;
    i++;
  }

  i = 0;
  pvector<int>::const_iterator iit;
  for(iit=dataIndices.begin(); iit!=dataIndices.end(); iit++) {
    NxU32 idx = *iit;

    _triangles[i] = idx;
    i++;
  }

  _desc.numVertices = numVertices;
  _desc.points = _points;
  _desc.numTriangles = numTriangles;
  _desc.triangles = _triangles;
}

