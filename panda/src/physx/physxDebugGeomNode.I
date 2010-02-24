// Filename: physxDebugGeomNode.I
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



////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxDebugGeomNode::
PhysxDebugGeomNode() : GeomNode("debug") {

  _scale = 1.0f;

  _vdata = new GeomVertexData("", GeomVertexFormat::get_v3c4(), Geom::UH_stream);

  // Lines
  _prim_lines = new GeomLines(Geom::UH_stream);
  _prim_lines->set_shade_model(Geom::SM_uniform);

  _geom_lines = new Geom(_vdata);
  _geom_lines->add_primitive(_prim_lines);

  this->add_geom(_geom_lines);

  // Triangles
  _prim_triangles = new GeomTriangles(Geom::UH_stream);
  _prim_triangles->set_shade_model(Geom::SM_uniform);

  _geom_triangles = new Geom(_vdata);
  _geom_triangles->add_primitive(_prim_triangles);

  this->add_geom(_geom_triangles);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PhysxDebugGeomNode::
~PhysxDebugGeomNode() {

}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_world_axes
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_world_axes(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_WORLD_AXES, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_body_axes
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_body_axes(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_BODY_AXES, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_body_mass_axes
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_body_mass_axes(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_BODY_MASS_AXES, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_body_lin_velocity
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_body_lin_velocity(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_BODY_LIN_VELOCITY, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_body_ang_velocity
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_body_ang_velocity(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_BODY_ANG_VELOCITY, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_body_joint_groups
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_body_joint_groups(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_BODY_JOINT_GROUPS, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_joint_local_axes
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_joint_local_axes(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_JOINT_LOCAL_AXES, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_joint_world_axes
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_joint_world_axes(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_JOINT_WORLD_AXES, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_joint_limits
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_joint_limits(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_JOINT_LIMITS, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_contact_point
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_contact_point(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_CONTACT_POINT, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_contact_normal
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_contact_normal(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_CONTACT_NORMAL, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_contact_error
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_contact_error(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_CONTACT_ERROR, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_contact_force
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_contact_force(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_CONTACT_FORCE, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_actor_axes
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_actor_axes(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_ACTOR_AXES, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_collision_aabbs
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_collision_aabbs(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_AABBS, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_collision_shapes
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_collision_shapes(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_SHAPES, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_collision_axes
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_collision_axes(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_AXES, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_collision_compounds
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_collision_compounds(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_COMPOUNDS, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_collision_vnormals
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_collision_vnormals(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_VNORMALS, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_collision_fnormals
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_collision_fnormals(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_FNORMALS, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_collision_edges
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_collision_edges(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_EDGES, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_collision_spheres
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_collision_spheres(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_SPHERES, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_collision_static
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_collision_static(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_STATIC, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_collision_dynamic
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_collision_dynamic(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_DYNAMIC, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_collision_free
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_collision_free(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_FREE, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_collision_ccd
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_collision_ccd(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_CCD, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_collision_skeletons
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_collision_skeletons(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_SKELETONS, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_cloth_mesh
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_cloth_mesh(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_CLOTH_MESH, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_cloth_validbounds
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_cloth_validbounds(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_CLOTH_VALIDBOUNDS, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_softbody_mesh
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_softbody_mesh(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_SOFTBODY_MESH, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_softbody_validbounds
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_softbody_validbounds(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_SOFTBODY_VALIDBOUNDS, value);
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxDebugGeomNode::visualize_force_fields
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PhysxDebugGeomNode::
visualize_force_fields(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_FORCE_FIELDS, value);
}

