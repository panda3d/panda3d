// Filename: bulletTriangleMesh.cxx
// Created by:  enn0x (09Feb10)
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

#include "bulletTriangleMesh.h"

#include "pvector.h"
#include "geomVertexData.h"
#include "geomVertexReader.h"

TypeHandle BulletTriangleMesh::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletTriangleMesh::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletTriangleMesh::
BulletTriangleMesh() {

  _mesh = new btTriangleMesh();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletTriangleMesh::get_num_triangles
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
int BulletTriangleMesh::
get_num_triangles() const {

  return _mesh->getNumTriangles();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletTriangleMesh::preallocate
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletTriangleMesh::
preallocate(int num_verts, int num_indices) {

  _mesh->preallocateVertices(num_verts);
  _mesh->preallocateIndices(num_indices);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletTriangleMesh::add_triangle
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletTriangleMesh::
add_triangle(const LPoint3 &p0, const LPoint3 &p1, const LPoint3 &p2, bool remove_duplicate_vertices) {

  nassertv(!p0.is_nan());
  nassertv(!p1.is_nan());
  nassertv(!p2.is_nan());

  _mesh->addTriangle(
    LVecBase3_to_btVector3(p0),
    LVecBase3_to_btVector3(p1),
    LVecBase3_to_btVector3(p2),
    remove_duplicate_vertices);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletTriangleMesh::set_welding_distance
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletTriangleMesh::
set_welding_distance(PN_stdfloat distance) {

  _mesh->m_weldingThreshold = distance;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletTriangleMesh::get_welding_distance
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletTriangleMesh::
get_welding_distance() const {

  return _mesh->m_weldingThreshold;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletTriangleMesh::add_geom
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletTriangleMesh::
add_geom(const Geom *geom, bool remove_duplicate_vertices, const TransformState *ts) {

  nassertv(geom);
  nassertv(ts);

  LMatrix4 m = ts->get_mat();

  // Collect points
  pvector<LPoint3> points;

  CPT(GeomVertexData) vdata = geom->get_vertex_data();
  GeomVertexReader reader = GeomVertexReader(vdata, InternalName::get_vertex());

  while (!reader.is_at_end()) {
    points.push_back(m.xform_point(reader.get_data3()));
  }

  // Convert points
  btVector3 *vertices = new btVector3[points.size()];

  int i = 0;
  pvector<LPoint3>::const_iterator it;
  for (it=points.begin(); it!=points.end(); it++) {
    LPoint3 v = *it;
    vertices[i] = LVecBase3_to_btVector3(v);
    i++;
  }

  // Add triangles
  for (int k=0; k<geom->get_num_primitives(); k++) {

    CPT(GeomPrimitive) prim = geom->get_primitive(k);
    prim = prim->decompose();

    for (int l=0; l<prim->get_num_primitives(); l++) {

      int s = prim->get_primitive_start(l);
      int e = prim->get_primitive_end(l);

      nassertv(e - s == 3);

      btVector3 v0 = vertices[prim->get_vertex(s)];
      btVector3 v1 = vertices[prim->get_vertex(s+1)];
      btVector3 v2 = vertices[prim->get_vertex(s+2)];

      _mesh->addTriangle(v0, v1, v2, remove_duplicate_vertices);
    }
  }

  delete [] vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletTriangleMesh::add_array
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletTriangleMesh::
add_array(const PTA_LVecBase3 &points, const PTA_int &indices, bool remove_duplicate_vertices) {

  // Convert vertices
  btVector3 *vertices = new btVector3[points.size()];

  int i = 0;
  PTA_LVecBase3::const_iterator it;
  for (it=points.begin(); it!=points.end(); it++) {
    LVecBase3 v = *it;
    vertices[i] = LVecBase3_to_btVector3(v);
    i++;
  }

  // Add triangles
  int j = 0;
  while (j+2 < (int)indices.size()) {

    btVector3 v0 = vertices[indices[j++]];
    btVector3 v1 = vertices[indices[j++]];
    btVector3 v2 = vertices[indices[j++]];

    _mesh->addTriangle(v0, v1, v2, remove_duplicate_vertices);
  }

  delete [] vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletTriangleMesh::output
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void BulletTriangleMesh::
output(ostream &out) const {

  out << get_type() << ", " << _mesh->getNumTriangles();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletTriangleMesh::write
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void BulletTriangleMesh::
write(ostream &out, int indent_level) const {

  indent(out, indent_level) << get_type() << ":" << endl;

  IndexedMeshArray& array = _mesh->getIndexedMeshArray();
  for (int i=0; i < array.size(); i++) {
    indent(out, indent_level + 2) << "IndexedMesh " << i << ":" << endl;
    btIndexedMesh meshPart = array.at(i);

    indent(out, indent_level + 4) << "num triangles:" << meshPart.m_numTriangles << endl;
    indent(out, indent_level + 4) << "num vertices:" << meshPart.m_numVertices << endl;
  }
}

