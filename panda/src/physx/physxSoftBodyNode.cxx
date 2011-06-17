// Filename: physxSoftBodyNode.cxx
// Created by:  enn0x (13Sep10)
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

#include "physxSoftBodyNode.h"
#include "physxSoftBody.h"
#include "physxFileStream.h"
#include "physxManager.h"
#include "physxMeshHash.h"

#include "geomVertexFormat.h"
#include "geomVertexWriter.h"
#include "geomVertexRewriter.h"

TypeHandle PhysxSoftBodyNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyNode::allocate
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyNode::
allocate(PhysxSoftBody *softbody) {

  _softbody = softbody;

  // Retrieve number of vertices and triangles the hard way
  NxSoftBodyMeshDesc meshDesc;
  _softbody->ptr()->getSoftBodyMesh()->saveToDesc(meshDesc);

  NxU32 numVertices = meshDesc.numVertices;
  NxU32 numTetrahedra = meshDesc.numTetrahedra;

  float factor = 1.0f; // TODO max(1.0f, factor);

  // Reserve more memory for vertices than the initial mesh takes because
  // tearing creates new vertices
  NxU32 maxVertices = factor * numVertices;
  _mesh.verticesPosBegin = (NxVec3 *)malloc(sizeof(NxVec3) * maxVertices);
  _mesh.verticesPosByteStride = sizeof(NxVec3);
  _mesh.maxVertices = maxVertices;
  _mesh.numVerticesPtr = (NxU32 *)malloc(sizeof(NxU32));

  // The number of tetrahedra is constant, even if the softbody is torn
  NxU32 maxIndices = 4 * numTetrahedra;
  _mesh.indicesBegin = (NxU32 *)malloc(sizeof(NxU32) * maxIndices);
  _mesh.indicesByteStride = sizeof(NxU32);
  _mesh.maxIndices = maxIndices;
  _mesh.numIndicesPtr = (NxU32 *)malloc(sizeof(NxU32));

  *(_mesh.numVerticesPtr) = 0;
  *(_mesh.numIndicesPtr) = 0;

  _softbody->ptr()->setMeshData(_mesh);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyNode::set_from_geom
//       Access: Published
//  Description: Reads the vertices and indices from an existing
//               Geom and makes a decomposed copy of the data.
//               Then computes links between the owning soft body
//               tetrahedron mesh in order to render an updated
//               geometry every simulation frame.
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyNode::
set_from_geom(const Geom *geom) {

  _prim->clear_vertices();

  GeomVertexWriter vwriter = GeomVertexWriter(_vdata, InternalName::get_vertex());
  GeomVertexWriter nwriter = GeomVertexWriter(_vdata, InternalName::get_normal());
  GeomVertexWriter twriter = GeomVertexWriter(_vdata, InternalName::get_texcoord());

  CPT(GeomVertexData) vdata = geom->get_vertex_data();
  GeomVertexReader vreader = GeomVertexReader(vdata, InternalName::get_vertex());
  GeomVertexReader nreader = GeomVertexReader(vdata, InternalName::get_normal());
  GeomVertexReader treader = GeomVertexReader(vdata, InternalName::get_texcoord());

  while (!vreader.is_at_end()) {
    LVecBase3f v = vreader.get_data3f();
    vwriter.add_data3f(v.get_x(), v.get_y(), v.get_z());
  }

  while (!nreader.is_at_end()) {
    LVecBase3f n = nreader.get_data3f();
    nwriter.add_data3f(n.get_x(), n.get_y(), n.get_z());
  }

  while (!treader.is_at_end()) {
    LVecBase2f t = treader.get_data2f();
    twriter.add_data2f(t.get_x(), t.get_y());
  }

  for (int i=0; i<geom->get_num_primitives(); i++) {

    CPT(GeomPrimitive) prim = geom->get_primitive(i);
    prim = prim->decompose();

    for (int j=0; j<prim->get_num_primitives(); j++) {

      int s = prim->get_primitive_start(j);
      int e = prim->get_primitive_end(j);

      for (int l=s; l<e; l++) {
        _prim->add_vertex(prim->get_vertex(l));
      }
    }
  }

  _prim->close_primitive();

  update_bounds();
  build_tetra_links();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyNode::build_tetra_links
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyNode::
build_tetra_links() {

  NxSoftBodyMeshDesc meshDesc;
  _softbody->ptr()->getSoftBodyMesh()->saveToDesc(meshDesc);
  const NxVec3 *vertices = (const NxVec3 *) meshDesc.vertices;
  const NxU32 *indices = (const NxU32 *) meshDesc.tetrahedra;
  const NxU32 numTets = meshDesc.numTetrahedra;

  _tetraLinks.clear();

  PhysxMeshHash* hash = new PhysxMeshHash();
  hash->set_grid_spacing(_bounds.min.distance(_bounds.max) * 0.1f);

  for (NxU32 i=0; i<numTets; i++) {
    const NxU32 *ix = &indices[4*i];
    NxBounds3 tetraBounds;
    tetraBounds.setEmpty();
    tetraBounds.include(vertices[*ix++]);
    tetraBounds.include(vertices[*ix++]);
    tetraBounds.include(vertices[*ix++]);
    tetraBounds.include(vertices[*ix++]);
    hash->add(tetraBounds, i);
  }

  GeomVertexReader vreader = GeomVertexReader(_vdata, InternalName::get_vertex());

  while (!vreader.is_at_end()) {

    // Prepare datastructure for drained tetras
    _drainedTriVertices.push_back(false);

    TetraLink tmpLink;

    LVecBase3f v = vreader.get_data3f();
    NxVec3 triVert = PhysxManager::vec3_to_nxVec3(v);
    pvector<int> itemIndices;
    hash->query_unique(triVert, itemIndices);

    NxReal minDist = 0.0f;
    NxVec3 b;
    int num, isize;
    num = isize = itemIndices.size();
    if (num == 0) {
        num = numTets;
    }

    for (int i=0; i<num; i++) {
      int j = i;
      if (isize > 0) {
          j = itemIndices[i];
      }

      const NxU32 *ix = &indices[j*4];
      const NxVec3 &p0 = vertices[*ix++];
      const NxVec3 &p1 = vertices[*ix++];
      const NxVec3 &p2 = vertices[*ix++];
      const NxVec3 &p3 = vertices[*ix++];

      NxVec3 b = compute_bary_coords(triVert, p0, p1, p2, p3);

      // Is the vertex inside the tetrahedron? If yes we take it
      if (b.x >= 0.0f && b.y >= 0.0f && b.z >= 0.0f && (b.x + b.y + b.z) <= 1.0f) {
        tmpLink.barycentricCoords = b;
        tmpLink.tetraNr = j;
        break;
      }

      // Otherwise, if we are not in any tetrahedron we take the closest one
      NxReal dist = 0.0f;
      if (b.x + b.y + b.z > 1.0f) dist = b.x + b.y + b.z - 1.0f;
      if (b.x < 0.0f) dist = (-b.x < dist) ? dist : -b.x;
      if (b.y < 0.0f) dist = (-b.y < dist) ? dist : -b.y;
      if (b.z < 0.0f) dist = (-b.z < dist) ? dist : -b.z;

      if (i == 0 || dist < minDist) {
        minDist = dist;
        tmpLink.barycentricCoords = b;
        tmpLink.tetraNr = j;
      }
    }

    _tetraLinks.push_back(tmpLink);
  }

  delete hash;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyNode::remove_tris_related_to_vertex
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyNode::
remove_tris_related_to_vertex(const int vertexIndex) {

  GeomVertexRewriter vrewriter = GeomVertexRewriter(_vdata, InternalName::get_vertex());
  LVecBase3f v;

  for (int j=0; j<_prim->get_num_primitives(); j++) {

    int s = _prim->get_primitive_start(j);
    int idx0 = _prim->get_vertex(s);
    int idx1 = _prim->get_vertex(s+1);
    int idx2 = _prim->get_vertex(s+2);

    if (vertexIndex == idx0 || vertexIndex == idx1 || vertexIndex == idx2) {

      // Make this triangle degenerated
      vrewriter.set_row_unsafe(idx0); v = vrewriter.get_data3f();
      vrewriter.set_row_unsafe(idx1); vrewriter.set_data3f(v.get_x(), v.get_y(), v.get_z());
      vrewriter.set_row_unsafe(idx2); vrewriter.set_data3f(v.get_x(), v.get_y(), v.get_z());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyNode::update_bounds
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyNode::
update_bounds() {

  _bounds.setEmpty();

  GeomVertexReader vreader = GeomVertexReader(_vdata, InternalName::get_vertex());

  while (!vreader.is_at_end()) {
    LVecBase3f v = vreader.get_data3f();
    _bounds.include(PhysxManager::vec3_to_nxVec3(v));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyNode::update_normals
//       Access: Public
//  Description:_bounds.include(mVertices[i]);
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyNode::
update_normals() {

  _normals.resize(_vdata->get_num_rows());

  int i;
  for (i=0; i<(int)_normals.size(); i++) { 
    _normals[i] = LVector3f::zero();
  }

  LVecBase3f n, v0, v1, v2;
  GeomVertexReader vreader = GeomVertexReader(_vdata, InternalName::get_vertex());

  for (int j=0; j<_prim->get_num_primitives(); j++) {

    int s = _prim->get_primitive_start(j);
    int idx0 = _prim->get_vertex(s);
    int idx1 = _prim->get_vertex(s+1);
    int idx2 = _prim->get_vertex(s+2);

    vreader.set_row_unsafe(idx0); v0 = vreader.get_data3f();
    vreader.set_row_unsafe(idx1); v1 = vreader.get_data3f();
    vreader.set_row_unsafe(idx2); v2 = vreader.get_data3f();

    n = (v1 - v0).cross(v2 - v0);

    _normals[idx0] += n;
    _normals[idx1] += n;
    _normals[idx2] += n;
  }

  for (i=0; i<(int)_normals.size(); i++) { 
    _normals[i].normalize();
  }

  GeomVertexWriter nwriter = GeomVertexWriter(_vdata, InternalName::get_normal());
  for (i=0; i<(int)_normals.size(); i++) { 
    n = _normals[i];
    nwriter.add_data3f(n.get_x(), n.get_y(), n.get_z());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyNode::compute_bary_coords
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
NxVec3 PhysxSoftBodyNode::
compute_bary_coords(NxVec3 vertex, NxVec3 p0, NxVec3 p1, NxVec3 p2, NxVec3 p3) const {

  NxVec3 baryCoords;

  NxVec3 q  = vertex - p3;
  NxVec3 q0 = p0 - p3;
  NxVec3 q1 = p1 - p3;
  NxVec3 q2 = p2 - p3;

  NxMat33 m;
  m.setColumn(0, q0);
  m.setColumn(1, q1);
  m.setColumn(2, q2);

  NxReal det = m.determinant();

  m.setColumn(0, q);
  baryCoords.x = m.determinant();

  m.setColumn(0, q0);
  m.setColumn(1, q);
  baryCoords.y = m.determinant();

  m.setColumn(1, q1);
  m.setColumn(2,q);
  baryCoords.z = m.determinant();

  if (det != 0.0f) {
    baryCoords /= det;
  }

  return baryCoords;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyNode::update
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxSoftBodyNode::
update() {

  update_tetra_links();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxSoftBodyNode::update_tetra_links
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool PhysxSoftBodyNode::
update_tetra_links() {

  if (_tetraLinks.size() != _vdata->get_num_rows())
  {
      return false;
  }

  NxU32 numVertices = *_mesh.numVerticesPtr;
  NxU32 numTetrahedra = *_mesh.numIndicesPtr / 4;
  const NxVec3 *vertices = (NxVec3*)_mesh.verticesPosBegin;
  NxU32* indices = (NxU32*)_mesh.indicesBegin;

  GeomVertexRewriter vwriter = GeomVertexRewriter(_vdata, InternalName::get_vertex());

  int i = 0;
  while (!vwriter.is_at_end()) {

    TetraLink &link = _tetraLinks[i];

    if (!_drainedTriVertices[i]) {
      const NxU32 *ix = &indices[4*link.tetraNr];

      if (*ix == *(ix + 1)) {
        remove_tris_related_to_vertex(i);
        _drainedTriVertices[i] = true;
        continue;
      }

      const NxVec3 &p0 = vertices[*ix++];
      const NxVec3 &p1 = vertices[*ix++];
      const NxVec3 &p2 = vertices[*ix++];
      const NxVec3 &p3 = vertices[*ix++];

      NxVec3 &b = link.barycentricCoords;
      NxVec3 tmp = p0 * b.x + p1 * b.y + p2 * b.z + p3 * (1.0f - b.x - b.y - b.z);
      vwriter.set_data3f(tmp.x, tmp.y, tmp.z);
    }
    i++;
  }

  update_normals();

  return true;
}

