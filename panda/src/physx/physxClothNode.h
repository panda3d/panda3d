// Filename: physxClothNode.h
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

#ifndef PHYSXCLOTHNODE_H
#define PHYSXCLOTHNODE_H

#include "pandabase.h"
#include "pointerTo.h"
#include "geomNode.h"
#include "transformState.h"
#include "geom.h"
#include "geomVertexData.h"
#include "geomTriangles.h"
#include "filename.h"

#include "physx_includes.h"

class PhysxCloth;

////////////////////////////////////////////////////////////////////
//       Class : PhysxClothNode
// Description : Renderable geometry which represents a cloth mesh.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxClothNode : public GeomNode {

PUBLISHED:
  INLINE PhysxClothNode(const char *name);
  INLINE ~PhysxClothNode();

  bool set_texcoords(const Filename &filename);

public:
  void allocate(PhysxCloth *cloth);
  void update();

private:
  void create_geom();
  void update_geom();
  void update_texcoords();

  unsigned int _numVertices;

  NxMeshData _mesh;

  PT(GeomVertexData) _vdata;
  PT(Geom) _geom;
  PT(GeomTriangles) _prim;

  PT(PhysxCloth) _cloth;

  unsigned int _numTexcoords;
  float *_texcoords;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeomNode::init_type();
    register_type(_type_handle, "PhysxClothNode", 
                  GeomNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "physxClothNode.I"

#endif // PHYSXCLOTHNODE_H
