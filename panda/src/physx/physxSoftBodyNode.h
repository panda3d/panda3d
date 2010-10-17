// Filename: physxSoftBodyNode.h
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

#ifndef PHYSXSOFTBODYNODE_H
#define PHYSXSOFTBODYNODE_H

#include "pandabase.h"
#include "pointerTo.h"
#include "geomNode.h"
#include "transformState.h"
#include "geom.h"
#include "geomVertexData.h"
#include "geomTriangles.h"
#include "filename.h"

#include "physx_includes.h"

class PhysxSoftBody;

////////////////////////////////////////////////////////////////////
//       Class : PhysxSoftBodyNode
// Description : Renderable geometry which represents a soft body
//               mesh.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxSoftBodyNode : public GeomNode {

PUBLISHED:
  INLINE PhysxSoftBodyNode(const char *name);
  INLINE ~PhysxSoftBodyNode();

  void set_from_geom(const Geom *geom);

public:
  void allocate(PhysxSoftBody *cloth);
  void update();

private:

  struct TetraLink {
    int tetraNr;
    NxVec3 barycentricCoords;
  };

  void update_bounds();
  void build_tetra_links();
  bool update_tetra_links();
  void update_normals();
  void remove_tris_related_to_vertex(const int vertexIndex);
  NxVec3 compute_bary_coords(NxVec3 vertex, NxVec3 p0, NxVec3 p1, NxVec3 p2, NxVec3 p3) const;

  pvector<TetraLink> _tetraLinks;
  pvector<bool> _drainedTriVertices;
  pvector<LVecBase3f> _normals;

  NxBounds3 _bounds;
  NxMeshData _mesh;

  PT(GeomVertexData) _vdata;
  PT(Geom) _geom;
  PT(GeomTriangles) _prim;

  PT(PhysxSoftBody) _softbody;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeomNode::init_type();
    register_type(_type_handle, "PhysxSoftBodyNode", 
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

#include "physxSoftBodyNode.I"

#endif // PHYSXSOFTBODYNODE_H
