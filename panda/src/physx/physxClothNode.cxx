// Filename: physxClothNode.cxx
// Created by:  enn0x (05Apr10)
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

#include "physxClothNode.h"
#include "physxCloth.h"
#include "physxFileStream.h"

#include "geomVertexFormat.h"
#include "geomVertexWriter.h"

TypeHandle PhysxClothNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothNode::allocate
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxClothNode::
allocate(PhysxCloth *cloth) {

  _cloth = cloth;

  // Retrieve number of vertices and triangles the hard way
  NxClothMeshDesc meshDesc;
  _cloth->ptr()->getClothMesh()->saveToDesc(meshDesc);

  NxU32 numVertices = meshDesc.numVertices;
  NxU32 numTriangles = meshDesc.numTriangles;

  NxReal factor = 1.0f; // TODO: max(1.0f, factor);

  // Reserve more memory for vertices than the initial mesh takes because
  // tearing creates new vertices
  NxU32 maxVertices = factor * numVertices;
  _mesh.verticesPosBegin = (NxVec3 *)malloc(sizeof(NxVec3) * maxVertices);
  _mesh.verticesNormalBegin = (NxVec3 *)malloc(sizeof(NxVec3) * maxVertices);
  _mesh.verticesPosByteStride = sizeof(NxVec3);
  _mesh.verticesNormalByteStride = sizeof(NxVec3);
  _mesh.maxVertices = maxVertices;
  _mesh.numVerticesPtr = (NxU32 *)malloc(sizeof(NxU32));

  // The number of triangles is constant, even if the cloth is torn
  NxU32 maxIndices = 3 * numTriangles;
  _mesh.indicesBegin = (NxU32 *)malloc(sizeof(NxU32) * maxIndices);
  _mesh.indicesByteStride = sizeof(NxU32);
  _mesh.maxIndices = maxIndices;
  _mesh.numIndicesPtr = (NxU32 *)malloc(sizeof(NxU32));

  NxU32 maxParentIndices = maxVertices;
  _mesh.parentIndicesBegin = (NxU32 *)malloc(sizeof(NxU32)*maxParentIndices);
  _mesh.parentIndicesByteStride = sizeof(NxU32);
  _mesh.maxParentIndices = maxParentIndices;
  _mesh.numParentIndicesPtr = (NxU32 *)malloc(sizeof(NxU32));

  *(_mesh.numVerticesPtr) = 0;
  *(_mesh.numIndicesPtr) = 0;

  _cloth->ptr()->setMeshData(_mesh);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothNode::update
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxClothNode::
update() {

  NxU32 numVertices = *(_mesh.numVerticesPtr);

  if (numVertices == _numVertices) {
    update_geom();
  }
  else {
    update_texcoords();
    create_geom();
    _numVertices = numVertices;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothNode::create_geom
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxClothNode::
create_geom() {

  _prim->clear_vertices();

  GeomVertexWriter vwriter = GeomVertexWriter(_vdata, InternalName::get_vertex());
  GeomVertexWriter nwriter = GeomVertexWriter(_vdata, InternalName::get_normal());
  GeomVertexWriter twriter = GeomVertexWriter(_vdata, InternalName::get_texcoord());

  // Vertices and normals
  NxU32 numVertices = *(_mesh.numVerticesPtr);
  NxVec3 *v = (NxVec3 *)_mesh.verticesPosBegin;
  NxVec3 *n = (NxVec3 *)_mesh.verticesNormalBegin;

  for (unsigned int i=0; i < numVertices; i++) {
    vwriter.add_data3f(v->x, v->y, v->z);
    v++;
    nwriter.add_data3f(n->x, n->y, n->z);
    n++;
  }

  // Texture coordinates
  NxReal tex_u;
  NxReal tex_v;

  if (_texcoords) {
    for (unsigned int i=0; i < numVertices; i++) {
      tex_u = _texcoords[2*i];
      tex_v = _texcoords[2*i+1];
      twriter.add_data2f(tex_u, tex_v);
    }
  }

  // Indices
  NxU32 numIndices = *(_mesh.numIndicesPtr);
  NxU32 *idx = (NxU32 *)_mesh.indicesBegin;

  for (unsigned int i=0; i < numIndices; i++) {
    _prim->add_vertex(*idx);
    idx++;
  }

  _prim->close_primitive();
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothNode::update_geom
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxClothNode::
update_geom() {

  GeomVertexWriter vwriter = GeomVertexWriter(_vdata, InternalName::get_vertex());
  GeomVertexWriter nwriter = GeomVertexWriter(_vdata, InternalName::get_normal());

  NxU32 numVertices = *(_mesh.numVerticesPtr);
  NxVec3 *v = (NxVec3 *)_mesh.verticesPosBegin;
  NxVec3 *n = (NxVec3 *)_mesh.verticesNormalBegin;

  for (unsigned int i=0; i < numVertices; i++) {
    vwriter.set_data3f(v->x, v->y, v->z);
    v++;
    nwriter.set_data3f(n->x, n->y, n->z);
    n++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothNode::update_texcoords
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void PhysxClothNode::
update_texcoords() {

  if (!_texcoords) {
    return;
  }

  NxU32 numVertices = *(_mesh.numVerticesPtr);
  NxU32 *parent = (NxU32 *)_mesh.parentIndicesBegin + _numTexcoords;

  for (NxU32 i=_numTexcoords; i < numVertices; i++, parent++) {
    _texcoords[2*i] = _texcoords[2*(*parent)];
    _texcoords[2*i+1] = _texcoords[2*(*parent)+1];
  }

  _numTexcoords = numVertices;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxClothNode::set_texcoords
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
bool PhysxClothNode::
set_texcoords(const Filename &filename) {

  if (filename.empty()) {
    return false;
  }

  Filename fn(filename);
  fn.resolve_filename(get_model_path());

  if (!filename.exists()) {
    return false;
  }

  PhysxFileStream fs(filename.c_str(), true);

  fs.readByte(); // N
  fs.readByte(); // X
  fs.readByte(); // X
  fs.readByte(); // 1
  fs.readByte(); // T
  fs.readByte(); // E
  fs.readByte(); // X
  fs.readByte(); // C
  fs.readByte(); // 1

  _numTexcoords = fs.readDword();
  _texcoords = new float[2 * _numTexcoords];

  for (unsigned int i=0; i<_numTexcoords; i++) {
    _texcoords[2*i]   = fs.readFloat();
    _texcoords[2*i+1] = fs.readFloat();
  }

  return true;
}

