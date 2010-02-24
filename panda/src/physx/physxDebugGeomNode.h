// Filename: physxDebugGeomNode.h
// Created by:  enn0x (15Sep09)
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

#ifndef PHYSXDEBUGGEOMNODE_H
#define PHYSXDEBUGGEOMNODE_H

#include "pandabase.h"
#include "pointerTo.h"
#include "geomNode.h"
#include "transformState.h"
#include "geom.h"
#include "geomVertexData.h"
#include "geomLines.h"
#include "geomTriangles.h"

#include "physx_includes.h"

class PhysxScene;

////////////////////////////////////////////////////////////////////
//       Class : PhysxDebugGeomNode
// Description : Renderable geometry which represents visualizations
//               of physics objects. Intended to help with
//               debugging code.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSX PhysxDebugGeomNode : public GeomNode {

PUBLISHED:
  INLINE PhysxDebugGeomNode();
  INLINE ~PhysxDebugGeomNode();

  void on();
  void off();
  void toggle();

  INLINE void visualize_world_axes(bool value);
  INLINE void visualize_body_axes(bool value);
  INLINE void visualize_body_mass_axes(bool value);
  INLINE void visualize_body_lin_velocity(bool value);
  INLINE void visualize_body_ang_velocity(bool value);
  INLINE void visualize_body_joint_groups(bool value);
  INLINE void visualize_joint_local_axes(bool value);
  INLINE void visualize_joint_world_axes(bool value);
  INLINE void visualize_joint_limits(bool value);
  INLINE void visualize_contact_point(bool value);
  INLINE void visualize_contact_normal(bool value);
  INLINE void visualize_contact_error(bool value);
  INLINE void visualize_contact_force(bool value);
  INLINE void visualize_actor_axes(bool value);
  INLINE void visualize_collision_aabbs(bool value);
  INLINE void visualize_collision_shapes(bool value);
  INLINE void visualize_collision_axes(bool value);
  INLINE void visualize_collision_compounds(bool value);
  INLINE void visualize_collision_vnormals(bool value);
  INLINE void visualize_collision_fnormals(bool value);
  INLINE void visualize_collision_edges(bool value);
  INLINE void visualize_collision_spheres(bool value);
  INLINE void visualize_collision_static(bool value);
  INLINE void visualize_collision_dynamic(bool value);
  INLINE void visualize_collision_free(bool value);
  INLINE void visualize_collision_ccd(bool value);
  INLINE void visualize_collision_skeletons(bool value);
  INLINE void visualize_cloth_mesh(bool value);
  INLINE void visualize_cloth_validbounds(bool value);
  INLINE void visualize_softbody_mesh(bool value);
  INLINE void visualize_softbody_validbounds(bool value);
  INLINE void visualize_force_fields(bool value);

public:
  void update(NxScene *scenePtr);

private:
  float _scale;

  PT(GeomVertexData) _vdata;
  PT(Geom) _geom_lines;
  PT(GeomLines) _prim_lines;
  PT(Geom) _geom_triangles;
  PT(GeomTriangles) _prim_triangles;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeomNode::init_type();
    register_type(_type_handle, "PhysxDebugGeomNode", 
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

#include "physxDebugGeomNode.I"

#endif // PHYSXDEBUGGEOMNODE_H
