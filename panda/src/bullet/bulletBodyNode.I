/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletBodyNode.I
 * @author enn0x
 * @date 2010-11-19
 */

/**

 */
INLINE BulletBodyNode::
~BulletBodyNode() {

  if (_shape && (
      _shape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE ||
      _shape->getShapeType() == EMPTY_SHAPE_PROXYTYPE)) {
    delete _shape;
  }
}

/**

 */
INLINE void BulletBodyNode::
set_into_collide_mask(CollideMask mask) {

  PandaNode::set_into_collide_mask(mask);

/*
  TODO: we would need a handle to the BulletWorld first
        possible, but has to be set/cleared upon attach/remove to world

  if (!_world) return;

  btBroadphaseProxy* proxy = get_object()->getBroadphaseHandle();
  if (proxy) {
    btBroadphaseInterface *broadphase = _world->get_broadphase();
    btDispatcher *dispatcher = _world->get_dispatcher();

    broadphase->getOverlappingPairCache()->cleanProxyFromPairs(proxy, dispatcher);
  }
*/
}

/**

 */
INLINE void BulletBodyNode::
notify_collisions(bool value) {

  set_collision_flag(btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK, value);
}

/**

 */
INLINE bool BulletBodyNode::
notifies_collisions() const {

  return get_collision_flag(btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK);
}

/**

 */
INLINE void BulletBodyNode::
set_collision_response(bool value) {

  set_collision_flag(btCollisionObject::CF_NO_CONTACT_RESPONSE, !value);
}

/**

 */
INLINE bool BulletBodyNode::
get_collision_response() const {

  return !get_collision_flag(btCollisionObject::CF_NO_CONTACT_RESPONSE);
}

/**

 */
INLINE void BulletBodyNode::
set_collision_flag(int flag, bool value) {

  int flags = get_object()->getCollisionFlags();

  if (value == true) {
    flags |= flag;
  }
  else {
    flags &= ~(flag);
  }

  get_object()->setCollisionFlags(flags);
}

/**

 */
INLINE bool BulletBodyNode::
get_collision_flag(int flag) const {

  return (get_object()->getCollisionFlags() & flag) ? true : false;
}

/**

 */
INLINE bool BulletBodyNode::
is_static() const {

  return get_object()->isStaticObject();
}

/**

 */
INLINE bool BulletBodyNode::
is_kinematic() const {

  return get_object()->isKinematicObject();
}

/**

 */
INLINE void BulletBodyNode::
set_static(bool value) {

  set_collision_flag(btCollisionObject::CF_STATIC_OBJECT, value);
}

/**

 */
INLINE void BulletBodyNode::
set_kinematic(bool value) {

  set_collision_flag(btCollisionObject::CF_KINEMATIC_OBJECT, value);
}

/**

 */
INLINE PN_stdfloat BulletBodyNode::
get_restitution() const {

  return get_object()->getRestitution();
}

/**

 */
INLINE void BulletBodyNode::
set_restitution(PN_stdfloat restitution) {

  return get_object()->setRestitution(restitution);
}

/**

 */
INLINE PN_stdfloat BulletBodyNode::
get_friction() const {

  return get_object()->getFriction();
}

/**

 */
INLINE void BulletBodyNode::
set_friction(PN_stdfloat friction) {

  return get_object()->setFriction(friction);
}

#if BT_BULLET_VERSION >= 281
/**

 */
INLINE PN_stdfloat BulletBodyNode::
get_rolling_friction() const {

  return get_object()->getRollingFriction();
}

/**

 */
INLINE void BulletBodyNode::
set_rolling_friction(PN_stdfloat friction) {

  return get_object()->setRollingFriction(friction);
}
#endif

/**

 */
INLINE bool BulletBodyNode::
has_anisotropic_friction() const {

  return get_object()->hasAnisotropicFriction();
}

/**

 */
INLINE int BulletBodyNode::
get_num_shapes() const {

  return _shapes.size();
}

/**

 */
INLINE BulletShape *BulletBodyNode::
get_shape(int idx) const {

  nassertr(idx >= 0 && idx < (int)_shapes.size(), NULL);
  return _shapes[idx];
}

/**
 * Enables or disables the debug visualisation for this collision object.  By
 * default the debug visualisation is enabled.
 */
INLINE void BulletBodyNode::
set_debug_enabled(const bool enabled) {

  set_collision_flag(btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT, !enabled);
}

/**
 * Returns TRUE if the debug visualisation is enabled for this collision object,
 * and FALSE if the debug visualisation is disabled.
 */
INLINE bool BulletBodyNode::
is_debug_enabled() const {

  return !get_collision_flag(btCollisionObject::CF_DISABLE_VISUALIZE_OBJECT);
}
