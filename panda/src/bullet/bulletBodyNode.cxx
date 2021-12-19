/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bulletBodyNode.cxx
 * @author enn0x
 * @date 2010-11-19
 */

#include "bulletBodyNode.h"

#include "config_bullet.h"

#include "bulletShape.h"
#include "bulletBoxShape.h"
#include "bulletCapsuleShape.h"
#include "bulletPlaneShape.h"
#include "bulletSphereShape.h"
#include "bulletTriangleMeshShape.h"
#include "bulletTriangleMesh.h"
#include "bulletWorld.h"

#include "collisionBox.h"
#include "collisionPlane.h"
#include "collisionSphere.h"
#include "collisionPolygon.h"
#include "collisionCapsule.h"

TypeHandle BulletBodyNode::_type_handle;

/**
 *
 */
BulletBodyNode::
BulletBodyNode(const char *name) : PandaNode(name) {

  // Shape
  _shape = new btEmptyShape();

  // Default collide mask
  set_into_collide_mask(CollideMask::all_on());
}

/**
 *
 */
BulletBodyNode::
BulletBodyNode(const BulletBodyNode &copy) :
  PandaNode(copy)
{
  LightMutexHolder holder(BulletWorld::get_global_lock());

  _shapes = copy._shapes;
  if (copy._shape && copy._shape->getShapeType() == COMPOUND_SHAPE_PROXYTYPE) {
    // btCompoundShape does not define a copy constructor.  Manually copy.
    btCompoundShape *shape = new btCompoundShape;
    _shape = shape;

    btCompoundShape *copy_shape = (btCompoundShape *)copy._shape;
    int num_children = copy_shape->getNumChildShapes();
    for (int i = 0; i < num_children; ++i) {
      shape->addChildShape(copy_shape->getChildTransform(i),
                           copy_shape->getChildShape(i));
    }
  }
  else if (copy._shape && copy._shape->getShapeType() == EMPTY_SHAPE_PROXYTYPE) {
    _shape = new btEmptyShape();
  }
  else {
    _shape = copy._shape;
  }
}

/**
 * Returns the subset of CollideMask bits that may be set for this particular
 * type of PandaNode.  For BodyNodes this returns all bits on.
 */
CollideMask BulletBodyNode::
get_legal_collide_mask() const {

  return CollideMask::all_on();
}

/**
 * Returns true if it is generally safe to flatten out this particular kind of
 * Node by duplicating instances, false otherwise (for instance, a Camera
 * cannot be safely flattened, because the Camera pointer itself is
 * meaningful).
 */
bool BulletBodyNode::
safe_to_flatten() const {

  return false;
}

/**
 * Returns true if it is generally safe to transform this particular kind of
 * Node by calling the xform() method, false otherwise.  For instance, it's
 * usually a bad idea to attempt to xform a Character.
 */
bool BulletBodyNode::
safe_to_transform() const {

  return false;
}

/**
 * Returns true if it is safe to automatically adjust the transform on this
 * kind of node.  Usually, this is only a bad idea if the user expects to find
 * a particular transform on the node.
 *
 * ModelNodes with the preserve_transform flag set are presently the only
 * kinds of nodes that should not have their transform even adjusted.
 */
bool BulletBodyNode::
safe_to_modify_transform() const {

  return false;
}

/**
 * Returns true if it is generally safe to combine this particular kind of
 * PandaNode with other kinds of PandaNodes of compatible type, adding
 * children or whatever.  For instance, an LODNode should not be combined with
 * any other PandaNode, because its set of children is meaningful.
 */
bool BulletBodyNode::
safe_to_combine() const {

  return false;
}

/**
 * Returns true if it is generally safe to combine the children of this
 * PandaNode with each other.  For instance, an LODNode's children should not
 * be combined with each other, because the set of children is meaningful.
 */
bool BulletBodyNode::
safe_to_combine_children() const {

  return false;
}

/**
 * Returns true if a flatten operation may safely continue past this node, or
 * false if nodes below this node may not be molested.
 */
bool BulletBodyNode::
safe_to_flatten_below() const {

  return false;
}

/**
 *
 */
void BulletBodyNode::
do_output(std::ostream &out) const {

  PandaNode::output(out);

  out << " (" << _shapes.size() << " shapes)";

  out << (get_object()->isActive() ? " active" : " inactive");

  if (get_object()->isStaticObject()) out << " static";
  if (get_object()->isKinematicObject()) out << " kinematic";
}

/**
 *
 */
void BulletBodyNode::
output(std::ostream &out) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  do_output(out);
}

/**
 *
 */
void BulletBodyNode::
set_collision_flag(int flag, bool value) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

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
 *
 */
bool BulletBodyNode::
get_collision_flag(int flag) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (get_object()->getCollisionFlags() & flag) ? true : false;
}

/**
 *
 */
bool BulletBodyNode::
is_static() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return get_object()->isStaticObject();
}

/**
 *
 */
bool BulletBodyNode::
is_kinematic() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return get_object()->isKinematicObject();
}

/**
 *
 */
PN_stdfloat BulletBodyNode::
get_restitution() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return get_object()->getRestitution();
}

/**
 *
 */
void BulletBodyNode::
set_restitution(PN_stdfloat restitution) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return get_object()->setRestitution(restitution);
}

/**
 *
 */
PN_stdfloat BulletBodyNode::
get_friction() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return get_object()->getFriction();
}

/**
 *
 */
void BulletBodyNode::
set_friction(PN_stdfloat friction) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return get_object()->setFriction(friction);
}

#if BT_BULLET_VERSION >= 281
/**
 *
 */
PN_stdfloat BulletBodyNode::
get_rolling_friction() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return get_object()->getRollingFriction();
}

/**
 *
 */
void BulletBodyNode::
set_rolling_friction(PN_stdfloat friction) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return get_object()->setRollingFriction(friction);
}
#endif

/**
 *
 */
bool BulletBodyNode::
has_anisotropic_friction() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return get_object()->hasAnisotropicFriction();
}

/**
 *
 */
int BulletBodyNode::
get_num_shapes() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return _shapes.size();
}

/**
 *
 */
BulletShape *BulletBodyNode::
get_shape(int idx) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertr(idx >= 0 && idx < (int)_shapes.size(), nullptr);
  return _shapes[idx];
}

/**
 *
 */
void BulletBodyNode::
add_shape(BulletShape *bullet_shape, const TransformState *ts) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  do_add_shape(bullet_shape, ts);
}

/**
 * Assumes the lock(bullet global lock) is held by the caller
 */
void BulletBodyNode::
do_add_shape(BulletShape *bullet_shape, const TransformState *ts) {

  nassertv(get_object());
  nassertv(ts);

  btCollisionShape *shape = bullet_shape->ptr();
  nassertv(shape != nullptr);

  nassertv(!(shape->getShapeType() == CONVEX_HULL_SHAPE_PROXYTYPE &&
            ((btConvexHullShape *)shape)->getNumVertices() == 0));

  // Transform
  btTransform trans = TransformState_to_btTrans(ts);

  // Reset the shape scaling before we add a shape, and remember the current
  // Scale so we can restore it later...
  CPT(TransformState) prev_transform = get_transform();
  bool scale_changed = false;
  if (!prev_transform->is_identity() && prev_transform->get_scale() != LVecBase3(1.0, 1.0, 1.0)) {
    // As a hack, temporarily release the lock, since transform_changed will
    // otherwise deadlock trying to grab it again.  See GitHub issue #689.
    LightMutex &lock = BulletWorld::get_global_lock();
    lock.release();
    set_transform(prev_transform->set_scale(LVecBase3(1.0, 1.0, 1.0)));
    lock.acquire();
    scale_changed = true;
  }

  // Root shape
  btCollisionShape *previous = get_object()->getCollisionShape();
  btCollisionShape *next;

  if (_shapes.size() == 0) {
    nassertv(previous->getShapeType() == EMPTY_SHAPE_PROXYTYPE);

    if (ts->is_identity()) {
      // After adding the shape we will have one shape, but with transform.
      // We need to wrap the shape within a compound shape, in oder to be able
      // to set the local transform.
      next = shape;
    }
    else {
      // After adding the shape we will have a total of one shape, without
      // local transform.  We can set the shape directly.
      next = new btCompoundShape();
      ((btCompoundShape *)next)->addChildShape(trans, shape);
    }

    get_object()->setCollisionShape(next);
    _shape = next;

    delete previous;
  }
  else if (_shapes.size() == 1) {
    if (previous->getShapeType() == COMPOUND_SHAPE_PROXYTYPE) {
      // We have one shape, and add another shape.  The previous shape is
      // already a compound shape.  So we just need to add the second shape to
      // the compound shape.
      next = previous;

      ((btCompoundShape *)next)->addChildShape(trans, shape);
    }
    else {
      // We have one shape which is NOT a compound shape, and want to add a
      // second shape.  We need to wrap both shapes within a compound shape.
      next = new btCompoundShape();

      btTransform previous_trans = btTransform::getIdentity();
      ((btCompoundShape *)next)->addChildShape(previous_trans, previous);
      ((btCompoundShape *)next)->addChildShape(trans, shape);

      get_object()->setCollisionShape(next);
      _shape = next;
    }
  }
  else {
    // We already have two or more shapes, and want to add another.  So we
    // already have a compound shape as wrapper, and just need to add the new
    // shape to the compound.
    nassertv(previous->getShapeType() == COMPOUND_SHAPE_PROXYTYPE);

    next = previous;
    ((btCompoundShape *)next)->addChildShape(trans, shape);
  }

  _shapes.push_back(bullet_shape);

  // Restore the local scaling again
  if (scale_changed) {
    CPT(TransformState) transform = get_transform()->set_scale(prev_transform->get_scale());
    LightMutex &lock = BulletWorld::get_global_lock();
    lock.release();
    set_transform(std::move(transform));
    lock.acquire();
  }

  do_shape_changed();
}

/**
 *
 */
void BulletBodyNode::
remove_shape(BulletShape *shape) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(get_object());

  BulletShapes::iterator found;
  PT(BulletShape) ptshape = shape;
  found = find(_shapes.begin(), _shapes.end(), ptshape);

  if (found == _shapes.end()) {
    bullet_cat.warning() << "shape not attached" << std::endl;
  }
  else {
    _shapes.erase(found);

    // Determine the new root shape
    btCollisionShape *previous = get_object()->getCollisionShape();
    btCollisionShape *next;

    if (_shapes.size() == 0) {
      // No more shapes remaining
      next = new btEmptyShape();

      get_object()->setCollisionShape(next);
      _shape = next;

      // The previous shape might be a compound.  Then delete it.
      if (previous->getShapeType() == COMPOUND_SHAPE_PROXYTYPE) {
        delete previous;
      }
    }
    else if (_shapes.size() == 1) {
      // Only one shape remaining
      nassertv(previous->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)

      btCompoundShape *compound = (btCompoundShape *)previous;
      compound->removeChildShape(shape->ptr());

      nassertv(compound->getNumChildShapes() == 1);

      // The compound is no longer required if the remaining shape has no
      // transform
      btTransform trans = compound->getChildTransform(0);
      if (is_identity(trans)) {
        next = compound->getChildShape(0);

        get_object()->setCollisionShape(next);
        _shape = next;

        delete compound;
      }
    }
    else {
      // More than one shape are remaining
      nassertv(previous->getShapeType() == COMPOUND_SHAPE_PROXYTYPE)

      btCompoundShape *compound = (btCompoundShape *)previous;
      compound->removeChildShape(shape->ptr());
    }

    do_shape_changed();
  }
}

/**
 * Returns TRUE if the transform is an identity transform, otherwise FALSE.
 */
bool BulletBodyNode::
is_identity(btTransform &trans) {

  btVector3 null(0, 0, 0);

  return (trans.getOrigin() == null
       && trans.getRotation().getAxis() == null);
}

/**
 *
 */
LPoint3 BulletBodyNode::
get_shape_pos(int idx) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertr(idx >= 0 && idx < (int)_shapes.size(), LPoint3::zero());

  btCollisionShape *root = get_object()->getCollisionShape();
  if (root->getShapeType() == COMPOUND_SHAPE_PROXYTYPE) {
    btCompoundShape *compound = (btCompoundShape *)root;

    btTransform trans = compound->getChildTransform(idx);
    return btVector3_to_LVector3(trans.getOrigin());
  }

  return LPoint3::zero();
}

/**
 *
 */
LMatrix4 BulletBodyNode::
get_shape_mat(int idx) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return do_get_shape_transform(idx)->get_mat();
}

/**
 *
 */
CPT(TransformState) BulletBodyNode::
do_get_shape_transform(int idx) const {

  nassertr(idx >= 0 && idx < (int)_shapes.size(), TransformState::make_identity());

  btCollisionShape *root = get_object()->getCollisionShape();
  if (root->getShapeType() == COMPOUND_SHAPE_PROXYTYPE) {
    btCompoundShape *compound = (btCompoundShape *)root;

    btTransform trans = compound->getChildTransform(idx);
    return btTrans_to_TransformState(trans);

    // The above code assumes that shape's index in _shapes member is the same
    // as the shapes index within the compound.  If it turns out that this is
    // not always true we could use the following code:
    /*
    btCollisionShape *shape = get_shape(idx)->ptr();
    for (int i=0; i<compound->getNumChildShapes(); i++) {
      if (compound->getChildShape(i) == shape) {
         btTransform trans = compound->getChildTransform(idx);
         return btTrans_to_LMatrix4(trans);
      }
    }
    */
  }

  return TransformState::make_identity();
}

/**
 *
 */
CPT(TransformState) BulletBodyNode::
get_shape_transform(int idx) const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return do_get_shape_transform(idx);
}


/**
 * Hook which will be called whenever the total shape of a body changed.  Used
 * for example to update the mass properties (inertia) of a rigid body.  The
 * default implementation does nothing.
 * Assumes the lock(bullet global lock) is held
 */
void BulletBodyNode::
do_shape_changed() {

}

/**
 *
 */
void BulletBodyNode::
set_deactivation_time(PN_stdfloat dt) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  get_object()->setDeactivationTime(dt);
}

/**
 *
 */
PN_stdfloat BulletBodyNode::
get_deactivation_time() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return get_object()->getDeactivationTime();
}

/**
 *
 */
bool BulletBodyNode::
is_active() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return get_object()->isActive();
}

/**
 *
 */
void BulletBodyNode::
set_active(bool active, bool force) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  if (active) {
    get_object()->activate(force);
  }
  else {
    if (force) {
      get_object()->forceActivationState(ISLAND_SLEEPING);
    }
    else {
      get_object()->setActivationState(ISLAND_SLEEPING);
    }
  }
}

/**
 *
 */
void BulletBodyNode::
force_active(bool active) {

  set_active(active, true);
}

/**
 * If true, this object will be deactivated after a certain amount of time has
 * passed without movement.  If false, the object will always remain active.
 */
void BulletBodyNode::
set_deactivation_enabled(bool enabled) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  // Don't change the state if it's currently active and we enable
  // deactivation.
  bool is_enabled = get_object()->getActivationState() != DISABLE_DEACTIVATION;
  if (enabled != is_enabled) {

    // It's OK to set to ACTIVE_TAG even if we don't mean to activate it; it
    // will be disabled right away if the deactivation timer has run out.
    int state = (enabled) ? ACTIVE_TAG : DISABLE_DEACTIVATION;
    get_object()->forceActivationState(state);
  }
}

/**
 *
 */
bool BulletBodyNode::
is_deactivation_enabled() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return (get_object()->getActivationState() != DISABLE_DEACTIVATION);
}

/**
 *
 */
bool BulletBodyNode::
check_collision_with(PandaNode *node) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  btCollisionObject *obj = BulletWorld::get_collision_object(node);

  if (obj) {
    return get_object()->checkCollideWith(obj);
  }
  else {
    return false;
  }
}

/**
 *
 */
LVecBase3 BulletBodyNode::
get_anisotropic_friction() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return btVector3_to_LVecBase3(get_object()->getAnisotropicFriction());
}

/**
 *
 */
void BulletBodyNode::
set_anisotropic_friction(const LVecBase3 &friction) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  nassertv(!friction.is_nan());
  get_object()->setAnisotropicFriction(LVecBase3_to_btVector3(friction));
}

/**
 *
 */
bool BulletBodyNode::
has_contact_response() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return get_object()->hasContactResponse();
}

/**
 *
 */
PN_stdfloat BulletBodyNode::
get_contact_processing_threshold() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());
  
  return get_object()->getContactProcessingThreshold();
}

/**
 * The constraint solver can discard solving contacts, if the distance is
 * above this threshold.
 */
void BulletBodyNode::
set_contact_processing_threshold(PN_stdfloat threshold) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  get_object()->setContactProcessingThreshold(threshold);
}

/**
 *
 */
PN_stdfloat BulletBodyNode::
get_ccd_swept_sphere_radius() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return get_object()->getCcdSweptSphereRadius();
}

/**
 *
 */
void BulletBodyNode::
set_ccd_swept_sphere_radius(PN_stdfloat radius) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return get_object()->setCcdSweptSphereRadius(radius);
}

/**
 *
 */
PN_stdfloat BulletBodyNode::
get_ccd_motion_threshold() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return get_object()->getCcdMotionThreshold();
}

/**
 *
 */
void BulletBodyNode::
set_ccd_motion_threshold(PN_stdfloat threshold) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  return get_object()->setCcdMotionThreshold(threshold);
}

/**
 *
 */
void BulletBodyNode::
add_shapes_from_collision_solids(CollisionNode *cnode) {
  LightMutexHolder holder(BulletWorld::get_global_lock());

  PT(BulletTriangleMesh) mesh = nullptr;

  for (size_t j = 0; j < cnode->get_num_solids(); ++j) {
    CPT(CollisionSolid) solid = cnode->get_solid(j);
    TypeHandle type = solid->get_type();

    // CollisionSphere
    if (CollisionSphere::get_class_type() == type) {
      CPT(CollisionSphere) sphere = DCAST(CollisionSphere, solid);
      CPT(TransformState) ts = TransformState::make_pos(sphere->get_center());

      do_add_shape(BulletSphereShape::make_from_solid(sphere), ts);
    }

    // CollisionBox
    else if (CollisionBox::get_class_type() == type) {
      CPT(CollisionBox) box = DCAST(CollisionBox, solid);
      CPT(TransformState) ts = TransformState::make_pos(box->get_center());

      do_add_shape(BulletBoxShape::make_from_solid(box), ts);
    }

    // CollisionCapsule
    else if (CollisionCapsule::get_class_type() == type) {
      CPT(CollisionCapsule) capsule = DCAST(CollisionCapsule, solid);
      CPT(TransformState) ts = TransformState::make_pos((capsule->get_point_b() + capsule->get_point_a()) / 2.0);

      do_add_shape(BulletCapsuleShape::make_from_solid(capsule), ts);
    }

    // CollisionPlane
    else if (CollisionPlane::get_class_type() == type) {
      CPT(CollisionPlane) plane = DCAST(CollisionPlane, solid);

      do_add_shape(BulletPlaneShape::make_from_solid(plane));
    }

    // CollisionGeom
    else if (CollisionPolygon::get_class_type() == type) {
      CPT(CollisionPolygon) polygon = DCAST(CollisionPolygon, solid);

      if (!mesh) {
         mesh = new BulletTriangleMesh();
      }

      for (size_t i = 2; i < polygon->get_num_points(); ++i) {
        LPoint3 p1 = polygon->get_point(0);
        LPoint3 p2 = polygon->get_point(i - 1);
        LPoint3 p3 = polygon->get_point(i);

        mesh->do_add_triangle(p1, p2, p3, true);
      }
    }
  }

  if (mesh && mesh->do_get_num_triangles() > 0) {
    do_add_shape(new BulletTriangleMeshShape(mesh, true));
  }
}

/**
 * This method enforces an update of the Bullet transform, that is copies the
 * scene graph transform to the Bullet transform.  This is achieved by alling
 * the protected PandaNode hook 'transform_changed'.
 */
void BulletBodyNode::
set_transform_dirty() {

  transform_changed();
}

/**
 * Returns the current bounds of all collision shapes owned by this body.
 */
BoundingSphere BulletBodyNode::
get_shape_bounds() const {
  LightMutexHolder holder(BulletWorld::get_global_lock());

/*
  btTransform tr;
  tr.setIdentity();
  btVector3 aabbMin,aabbMax;
  ptr()->getAabb(tr,aabbMin,aabbMax);
  btVector3 o = tr.getOrigin();
cout << "aabbMin " << aabbMin.x() << " " << aabbMin.y() << " " << aabbMin.z() << endl;
cout << "aabbMax " << aabbMax.x() << " " << aabbMax.y() << " " << aabbMax.z() << endl;
cout << "origin " << aabbMin.x() << " " << aabbMin.y() << " " << aabbMin.z() << endl;
*/

  btVector3 center;
  btScalar radius = 0;

  if (_shape) {
    _shape->getBoundingSphere(center, radius);
  }

  BoundingSphere bounds(btVector3_to_LPoint3(center), (PN_stdfloat)radius);

  return bounds;
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void BulletBodyNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);

  dg.add_bool(is_static());
  dg.add_bool(is_kinematic());
  dg.add_bool(notifies_collisions());
  dg.add_bool(get_collision_response());
  dg.add_stdfloat(get_contact_processing_threshold());
  // dg.add_bool(is_active());
  dg.add_stdfloat(get_deactivation_time());
  dg.add_bool(is_deactivation_enabled());
  // dg.add_bool(is_debug_enabled());
  dg.add_stdfloat(get_restitution());
  dg.add_stdfloat(get_friction());
#if BT_BULLET_VERSION >= 281
  dg.add_stdfloat(get_rolling_friction());
#else
  dg.add_stdfloat(0);
#endif
  if (has_anisotropic_friction()) {
    dg.add_bool(true);
    get_anisotropic_friction().write_datagram(dg);
  } else {
    dg.add_bool(false);
  }
  dg.add_stdfloat(get_ccd_swept_sphere_radius());
  dg.add_stdfloat(get_ccd_motion_threshold());

  for (unsigned int i = 0; i < _shapes.size(); ++i) {
    manager->write_pointer(dg, get_shape(i));
    manager->write_pointer(dg, get_shape_transform(i));
  }

  // Write NULL pointer to indicate the end of the list.
  manager->write_pointer(dg, nullptr);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int BulletBodyNode::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = PandaNode::complete_pointers(p_list, manager);

  PT(BulletShape) shape = DCAST(BulletShape, p_list[pi++]);

  while (shape != nullptr) {
    const TransformState *trans = DCAST(TransformState, p_list[pi++]);
    add_shape(shape, trans);

    shape = DCAST(BulletShape, p_list[pi++]);
  }

  return pi;
}

/**
 * Some objects require all of their nested pointers to have been completed
 * before the objects themselves can be completed.  If this is the case,
 * override this method to return true, and be careful with circular
 * references (which would make the object unreadable from a bam file).
 */
bool BulletBodyNode::
require_fully_complete() const {
  // We require the shape pointers to be complete before we add them.
  return true;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new BulletBodyNode.
 */
void BulletBodyNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);

  set_static(scan.get_bool());
  set_kinematic(scan.get_bool());
  notify_collisions(scan.get_bool());
  set_collision_response(scan.get_bool());
  set_contact_processing_threshold(scan.get_stdfloat());
  // set_active(scan.get_bool(), true);
  set_deactivation_time(scan.get_stdfloat());
  set_deactivation_enabled(scan.get_bool());
  set_restitution(scan.get_stdfloat());
  set_friction(scan.get_stdfloat());
#if BT_BULLET_VERSION >= 281
  set_rolling_friction(scan.get_stdfloat());
#else
  scan.get_stdfloat();
#endif
  if (scan.get_bool()) {
    LVector3 friction;
    friction.read_datagram(scan);
    set_anisotropic_friction(friction);
  }
  set_ccd_swept_sphere_radius(scan.get_stdfloat());
  set_ccd_motion_threshold(scan.get_stdfloat());

  // Read shapes.  The list is bounded by a NULL pointer.
  while (manager->read_pointer(scan)) {
    // Each shape comes with a TransformState.
    manager->read_pointer(scan);
  }
}
