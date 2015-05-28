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

////////////////////////////////////////////////////////////////////
//     Function: BulletTriangleMesh::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               BulletTriangleMesh.
////////////////////////////////////////////////////////////////////
void BulletTriangleMesh::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletTriangleMesh::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void BulletTriangleMesh::
write_datagram(BamWriter *manager, Datagram &dg) {
  dg.add_stdfloat(get_welding_distance());

  // In case we ever want to represent more than 1 indexed mesh.
  dg.add_int32(1);

  btIndexedMesh &mesh = _mesh->getIndexedMeshArray()[0];
  dg.add_int32(mesh.m_numVertices);
  dg.add_int32(mesh.m_numTriangles);

  // In case we want to use this to distinguish 16-bit vs 32-bit indices.
  dg.add_bool(true);

  // Add the vertices.
  const unsigned char *vptr = mesh.m_vertexBase;
  nassertv(vptr != NULL || mesh.m_numVertices == 0);

  for (int i = 0; i < mesh.m_numVertices; ++i) {
    const btVector3 &vertex = *((btVector3 *)vptr);
    dg.add_stdfloat(vertex.getX());
    dg.add_stdfloat(vertex.getY());
    dg.add_stdfloat(vertex.getZ());
    vptr += mesh.m_vertexStride;
  }

  // Now add the triangle indices.
  const unsigned char *iptr = mesh.m_triangleIndexBase;
  nassertv(iptr != NULL || mesh.m_numTriangles == 0);

  if (_mesh->getUse32bitIndices()) {
    for (int i = 0; i < mesh.m_numTriangles; ++i) {
      int *triangle = (int *)iptr;
      dg.add_int32(triangle[0]);
      dg.add_int32(triangle[1]);
      dg.add_int32(triangle[2]);
      iptr += mesh.m_triangleIndexStride;
    }
  } else {
    for (int i = 0; i < mesh.m_numTriangles; ++i) {
      short int *triangle = (short int *)iptr;
      dg.add_int32(triangle[0]);
      dg.add_int32(triangle[1]);
      dg.add_int32(triangle[2]);
      iptr += mesh.m_triangleIndexStride;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BulletTriangleMesh::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type BulletShape is encountered
//               in the Bam file.  It should create the BulletShape
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *BulletTriangleMesh::
make_from_bam(const FactoryParams &params) {
  BulletTriangleMesh *param = new BulletTriangleMesh;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  param->fillin(scan, manager);

  return param;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletTriangleMesh::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new BulletTriangleMesh.
////////////////////////////////////////////////////////////////////
void BulletTriangleMesh::
fillin(DatagramIterator &scan, BamReader *manager) {
  set_welding_distance(scan.get_stdfloat());

  nassertv(scan.get_int32() == 1);
  int num_vertices = scan.get_int32();
  int num_triangles = scan.get_int32();
  nassertv(scan.get_bool() == true);

  // Read and add the vertices.
  _mesh->preallocateVertices(num_vertices);
  for (int i = 0; i < num_vertices; ++i) {
    PN_stdfloat x = scan.get_stdfloat();
    PN_stdfloat y = scan.get_stdfloat();
    PN_stdfloat z = scan.get_stdfloat();
    _mesh->findOrAddVertex(btVector3(x, y, z), false);
  }

  // Now read and add the indices.
  int num_indices = num_triangles * 3;
  _mesh->preallocateIndices(num_indices);
  for (int i = 0; i < num_indices; ++i) {
    _mesh->addIndex(scan.get_int32());
  }

  // Since we manually added the vertices individually, we have to
  // update the triangle count appropriately.
  _mesh->getIndexedMeshArray()[0].m_numTriangles = num_triangles;
}
