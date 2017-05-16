/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletTriangleMesh.cxx
 * @author enn0x
 * @date 2010-02-09
 */

#include "bulletTriangleMesh.h"

#include "pvector.h"
#include "geomVertexData.h"
#include "geomVertexReader.h"

TypeHandle BulletTriangleMesh::_type_handle;

/**
 *
 */
BulletTriangleMesh::
BulletTriangleMesh()
 : _welding_distance(0) {
  btIndexedMesh mesh;
  mesh.m_numTriangles = 0;
  mesh.m_numVertices = 0;
  mesh.m_indexType = PHY_INTEGER;
  mesh.m_triangleIndexBase = nullptr;
  mesh.m_triangleIndexStride = 3 * sizeof(int);
  mesh.m_vertexBase = nullptr;
  mesh.m_vertexStride = sizeof(btVector3);
  _mesh.addIndexedMesh(mesh);
}

/**
 * Returns the number of triangles in this triangle mesh.
 */
int BulletTriangleMesh::
get_num_triangles() const {
  return _indices.size() / 3;
}

/**
 * Used to reserve memory in anticipation of the given amount of vertices and
 * indices being added to the triangle mesh.  This is useful if you are about
 * to call add_triangle() many times, to prevent unnecessary reallocations.
 */
void BulletTriangleMesh::
preallocate(int num_verts, int num_indices) {
  _vertices.reserve(num_verts);
  _indices.reserve(num_indices);
}

/**
 * Adds a triangle with the indicated coordinates.
 *
 * If remove_duplicate_vertices is true, it will make sure that it does not
 * add duplicate vertices if they already exist in the triangle mesh, within
 * the tolerance specified by set_welding_distance().  This comes at a
 * significant performance cost, especially for large meshes.
 */
void BulletTriangleMesh::
add_triangle(const LPoint3 &p0, const LPoint3 &p1, const LPoint3 &p2, bool remove_duplicate_vertices) {
  nassertv(!p0.is_nan());
  nassertv(!p1.is_nan());
  nassertv(!p2.is_nan());

  btIndexedMesh &mesh = _mesh.getIndexedMeshArray()[0];
  mesh.m_numTriangles++;

  if (!remove_duplicate_vertices) {
    unsigned int index = _vertices.size();
    _indices.push_back(index++);
    _indices.push_back(index++);
    _indices.push_back(index++);

    _vertices.push_back(LVecBase3_to_btVector3(p0));
    _vertices.push_back(LVecBase3_to_btVector3(p1));
    _vertices.push_back(LVecBase3_to_btVector3(p2));
    mesh.m_numVertices += 3;
    mesh.m_vertexBase = (unsigned char*)&_vertices[0];
  } else {
    _indices.push_back(find_or_add_vertex(p0));
    _indices.push_back(find_or_add_vertex(p1));
    _indices.push_back(find_or_add_vertex(p2));
  }

  mesh.m_triangleIndexBase = (unsigned char *)&_indices[0];
}

/**
 * Sets the square of the distance at which vertices will be merged
 * together when adding geometry with remove_duplicate_vertices set to true.
 *
 * The default is 0, meaning vertices will only be merged if they have the
 * exact same position.
 */
void BulletTriangleMesh::
set_welding_distance(PN_stdfloat distance) {
  _welding_distance = distance;
}

/**
 * Returns the value previously set with set_welding_distance(), or the
 * value of 0 if none was set.
 */
PN_stdfloat BulletTriangleMesh::
get_welding_distance() const {
  return _welding_distance;
}

/**
 * Adds the geometry from the indicated Geom from the triangle mesh.  This is
 * a one-time copy operation, and future updates to the Geom will not be
 * reflected.
 *
 * If remove_duplicate_vertices is true, it will make sure that it does not
 * add duplicate vertices if they already exist in the triangle mesh, within
 * the tolerance specified by set_welding_distance().  This comes at a
 * significant performance cost, especially for large meshes.
 */
void BulletTriangleMesh::
add_geom(const Geom *geom, bool remove_duplicate_vertices, const TransformState *ts) {
  nassertv(geom);
  nassertv(ts);

  CPT(GeomVertexData) vdata = geom->get_vertex_data();
  size_t num_vertices = vdata->get_num_rows();
  GeomVertexReader reader = GeomVertexReader(vdata, InternalName::get_vertex());

  btIndexedMesh &mesh = _mesh.getIndexedMeshArray()[0];

  if (!remove_duplicate_vertices) {
    // Fast path: directly copy the vertices and indices.
    mesh.m_numVertices += num_vertices;
    unsigned int index_offset = _vertices.size();
    _vertices.reserve(_vertices.size() + num_vertices);

    if (ts->is_identity()) {
      while (!reader.is_at_end()) {
        _vertices.push_back(LVecBase3_to_btVector3(reader.get_data3()));
      }
    } else {
      LMatrix4 m = ts->get_mat();
      while (!reader.is_at_end()) {
        _vertices.push_back(LVecBase3_to_btVector3(m.xform_point(reader.get_data3())));
      }
    }

    for (int k = 0; k < geom->get_num_primitives(); ++k) {
      CPT(GeomPrimitive) prim = geom->get_primitive(k);
      prim = prim->decompose();

      if (prim->is_of_type(GeomTriangles::get_class_type())) {
        int num_vertices = prim->get_num_vertices();
        _indices.reserve(_indices.size() + num_vertices);
        mesh.m_numTriangles += num_vertices / 3;

        GeomVertexReader index(prim->get_vertices(), 0);
        while (!index.is_at_end()) {
          _indices.push_back(index_offset + index.get_data1i());
        }
      }
    }

  } else {
    // Collect points
    pvector<LPoint3> points;
    points.reserve(_vertices.size() + num_vertices);

    if (ts->is_identity()) {
      while (!reader.is_at_end()) {
        points.push_back(reader.get_data3());
      }
    } else {
      LMatrix4 m = ts->get_mat();
      while (!reader.is_at_end()) {
        points.push_back(m.xform_point(reader.get_data3()));
      }
    }

    // Add triangles
    for (int k = 0; k < geom->get_num_primitives(); ++k) {
      CPT(GeomPrimitive) prim = geom->get_primitive(k);
      prim = prim->decompose();

      if (prim->is_of_type(GeomTriangles::get_class_type())) {
        int num_vertices = prim->get_num_vertices();
        _indices.reserve(_indices.size() + num_vertices);
        mesh.m_numTriangles += num_vertices / 3;

        GeomVertexReader index(prim->get_vertices(), 0);
        while (!index.is_at_end()) {
          _indices.push_back(find_or_add_vertex(points[index.get_data1i()]));
        }
      }
    }
  }

  // Reset the pointers, since the vectors may have been reallocated.
  mesh.m_vertexBase = (unsigned char*)&_vertices[0];
  mesh.m_triangleIndexBase = (unsigned char *)&_indices[0];
}

/**
 * Adds triangle information from an array of points and indices referring to
 * these points.  This is more efficient than adding triangles one at a time.
 *
 * If remove_duplicate_vertices is true, it will make sure that it does not
 * add duplicate vertices if they already exist in the triangle mesh, within
 * the tolerance specified by set_welding_distance().  This comes at a
 * significant performance cost, especially for large meshes.
 */
void BulletTriangleMesh::
add_array(const PTA_LVecBase3 &points, const PTA_int &indices, bool remove_duplicate_vertices) {
  btIndexedMesh &mesh = _mesh.getIndexedMeshArray()[0];

  _indices.reserve(_indices.size() + indices.size());

  if (!remove_duplicate_vertices) {
    unsigned int index_offset = _vertices.size();
    for (size_t i = 0; i < indices.size(); ++i) {
      _indices.push_back(index_offset + indices[i]);
    }

    _vertices.reserve(_vertices.size() + points.size());
    for (size_t i = 0; i < points.size(); ++i) {
      _vertices.push_back(LVecBase3_to_btVector3(points[i]));
    }

    mesh.m_numVertices += points.size();

  } else {
    // Add the points one by one.
    _indices.reserve(_indices.size() + indices.size());
    for (size_t i = 0; i < indices.size(); ++i) {
      LVecBase3 p = points[indices[i]];
      _indices.push_back(find_or_add_vertex(p));
    }
  }

  mesh.m_numTriangles += indices.size() / 3;

  // Reset the pointers, since the vectors may have been reallocated.
  mesh.m_vertexBase = (unsigned char*)&_vertices[0];
  mesh.m_triangleIndexBase = (unsigned char *)&_indices[0];
}

/**
 *
 */
void BulletTriangleMesh::
output(ostream &out) const {
  out << get_type() << ", " << get_num_triangles() << " triangles";
}

/**
 *
 */
void BulletTriangleMesh::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << get_type() << ":" << endl;

  const IndexedMeshArray &array = _mesh.getIndexedMeshArray();
  for (size_t i = 0; i < array.size(); ++i) {
    indent(out, indent_level + 2) << "IndexedMesh " << i << ":" << endl;
    const btIndexedMesh &mesh = array[0];
    indent(out, indent_level + 4) << "num triangles:" << mesh.m_numTriangles << endl;
    indent(out, indent_level + 4) << "num vertices:" << mesh.m_numVertices << endl;
  }
}

/**
 * Finds the indicated vertex and returns its index.  If it was not found,
 * adds it as a new vertex and returns its index.
 */
unsigned int BulletTriangleMesh::
find_or_add_vertex(const LVecBase3 &p) {
  btVector3 vertex = LVecBase3_to_btVector3(p);

  for (unsigned int i = 0; i < _vertices.size(); ++i) {
    if ((_vertices[i] - vertex).length2() <= _welding_distance) {
      return i;
    }
  }

  _vertices.push_back(vertex);

  btIndexedMesh &mesh = _mesh.getIndexedMeshArray()[0];
  mesh.m_numVertices++;
  mesh.m_vertexBase = (unsigned char*)&_vertices[0];
  return _vertices.size() - 1;
}

/**
 * Tells the BamReader how to create objects of type BulletTriangleMesh.
 */
void BulletTriangleMesh::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void BulletTriangleMesh::
write_datagram(BamWriter *manager, Datagram &dg) {
  dg.add_stdfloat(get_welding_distance());

  // In case we ever want to represent more than 1 indexed mesh.
  dg.add_int32(1);

  btIndexedMesh &mesh = _mesh.getIndexedMeshArray()[0];
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

  for (int i = 0; i < mesh.m_numTriangles; ++i) {
    int *triangle = (int *)iptr;
    dg.add_int32(triangle[0]);
    dg.add_int32(triangle[1]);
    dg.add_int32(triangle[2]);
    iptr += mesh.m_triangleIndexStride;
  }
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type BulletShape is encountered in the Bam file.  It should create the
 * BulletShape and extract its information from the file.
 */
TypedWritable *BulletTriangleMesh::
make_from_bam(const FactoryParams &params) {
  BulletTriangleMesh *param = new BulletTriangleMesh;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  param->fillin(scan, manager);

  return param;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new BulletTriangleMesh.
 */
void BulletTriangleMesh::
fillin(DatagramIterator &scan, BamReader *manager) {
  set_welding_distance(scan.get_stdfloat());

  nassertv(scan.get_int32() == 1);
  int num_vertices = scan.get_int32();
  int num_triangles = scan.get_int32();
  nassertv(scan.get_bool() == true);

  btIndexedMesh &mesh = _mesh.getIndexedMeshArray()[0];
  mesh.m_numVertices = num_vertices;
  mesh.m_numTriangles = num_triangles;

  // Read and add the vertices.
  _vertices.clear();
  _vertices.reserve(num_vertices);
  for (int i = 0; i < num_vertices; ++i) {
    PN_stdfloat x = scan.get_stdfloat();
    PN_stdfloat y = scan.get_stdfloat();
    PN_stdfloat z = scan.get_stdfloat();
    _vertices.push_back(btVector3(x, y, z));
  }

  // Now read and add the indices.
  size_t num_indices = (size_t)num_triangles * 3;
  _indices.resize(num_indices);
  scan.extract_bytes((unsigned char *)&_indices[0], num_indices * sizeof(int));

  // Reset the pointers, since the vectors may have been reallocated.
  mesh.m_vertexBase = (unsigned char*)&_vertices[0];
  mesh.m_triangleIndexBase = (unsigned char *)&_indices[0];
}
