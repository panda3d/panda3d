// Filename: xFileMesh.cxx
// Created by:  drose (19Jun01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "xFileMesh.h"
#include "xFileFace.h"
#include "xFileVertex.h"
#include "xFileNormal.h"

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileMesh::
XFileMesh() {
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileMesh::
~XFileMesh() {
  // ** Delete stuff.
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::add_polygon
//       Access: Public
//  Description: Adds the indicated polygon to the mesh.
////////////////////////////////////////////////////////////////////
void XFileMesh::
add_polygon(EggPolygon *egg_poly) {
  XFileFace *face = new XFileFace(this, egg_poly);
  _faces.push_back(face);
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::add_vertex
//       Access: Public
//  Description: Creates a new XFileVertex, if one does not already
//               exist for the indicated vertex, and returns its
//               index.
////////////////////////////////////////////////////////////////////
int XFileMesh::
add_vertex(EggVertex *egg_vertex, EggPrimitive *egg_prim) {
  int next_index = _vertices.size();
  XFileVertex *vertex = new XFileVertex(egg_vertex, egg_prim);
  pair<UniqueVertices::iterator, bool> result =
    _unique_vertices.insert(UniqueVertices::value_type(vertex, next_index));

  if (result.second) {
    // Successfully added; this is a new vertex.
    _vertices.push_back(vertex);
    return next_index;
  } else {
    // Not successfully added; there is already a vertex with these
    // properties.  Return that one instead.
    delete vertex;
    return (*result.first).second;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::add_normal
//       Access: Public
//  Description: Creates a new XFileNormal, if one does not already
//               exist for the indicated normal, and returns its
//               index.
////////////////////////////////////////////////////////////////////
int XFileMesh::
add_normal(EggVertex *egg_vertex, EggPrimitive *egg_prim) {
  int next_index = _normals.size();
  XFileNormal *normal = new XFileNormal(egg_vertex, egg_prim);
  pair<UniqueNormals::iterator, bool> result =
    _unique_normals.insert(UniqueNormals::value_type(normal, next_index));

  if (result.second) {
    // Successfully added; this is a new normal.
    _normals.push_back(normal);
    return next_index;
  } else {
    // Not successfully added; there is already a normal with these
    // properties.  Return that one instead.
    delete normal;
    return (*result.first).second;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::make_mesh_data
//       Access: Public
//  Description: Fills the datagram with the raw data for the DX
//               Mesh template.
////////////////////////////////////////////////////////////////////
void XFileMesh::
make_mesh_data(Datagram &raw_data) {
  raw_data.add_int32(_vertices.size());
  
  Vertices::const_iterator vi;
  for (vi = _vertices.begin(); vi != _vertices.end(); ++vi) {
    XFileVertex *vertex = (*vi);
    const Vertexf &point = vertex->_point;
    raw_data.add_float32(point[0]);
    raw_data.add_float32(point[1]);
    raw_data.add_float32(point[2]);
  }

  raw_data.add_int32(_faces.size());
  Faces::const_iterator fi;
  for (fi = _faces.begin(); fi != _faces.end(); ++fi) {
    XFileFace *face = (*fi);

    raw_data.add_int32(face->_vertices.size());
    XFileFace::Vertices::const_iterator fvi;
    for (fvi = face->_vertices.begin();
         fvi != face->_vertices.end();
         ++fvi) {
      raw_data.add_int32((*fvi)._vertex_index);
    }
  }
}
