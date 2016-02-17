/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file physxDebugGeomNode.I
 * @author enn0x
 * @date 2009-09-15
 */

/**

 */
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

/**

 */
INLINE PhysxDebugGeomNode::
~PhysxDebugGeomNode() {

}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_world_axes(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_WORLD_AXES, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_body_axes(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_BODY_AXES, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_body_mass_axes(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_BODY_MASS_AXES, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_body_lin_velocity(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_BODY_LIN_VELOCITY, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_body_ang_velocity(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_BODY_ANG_VELOCITY, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_body_joint_groups(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_BODY_JOINT_GROUPS, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_joint_local_axes(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_JOINT_LOCAL_AXES, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_joint_world_axes(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_JOINT_WORLD_AXES, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_joint_limits(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_JOINT_LIMITS, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_contact_point(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_CONTACT_POINT, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_contact_normal(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_CONTACT_NORMAL, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_contact_error(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_CONTACT_ERROR, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_contact_force(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_CONTACT_FORCE, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_actor_axes(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_ACTOR_AXES, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_collision_aabbs(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_AABBS, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_collision_shapes(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_SHAPES, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_collision_axes(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_AXES, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_collision_compounds(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_COMPOUNDS, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_collision_vnormals(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_VNORMALS, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_collision_fnormals(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_FNORMALS, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_collision_edges(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_EDGES, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_collision_spheres(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_SPHERES, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_collision_static(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_STATIC, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_collision_dynamic(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_DYNAMIC, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_collision_free(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_FREE, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_collision_ccd(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_CCD, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_collision_skeletons(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_COLLISION_SKELETONS, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_cloth_mesh(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_CLOTH_MESH, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_cloth_validbounds(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_CLOTH_VALIDBOUNDS, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_softbody_mesh(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_SOFTBODY_MESH, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_softbody_validbounds(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_SOFTBODY_VALIDBOUNDS, value);
}

/**

 */
INLINE void PhysxDebugGeomNode::
visualize_force_fields(bool value) {

  NxGetPhysicsSDK()->setParameter(NX_VISUALIZE_FORCE_FIELDS, value);
}
