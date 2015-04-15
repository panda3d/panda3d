// Filename: bulletBodyNode.cxx
// Created by:  enn0x (19Nov10)
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

#include "bulletBodyNode.h"
#include "bulletShape.h"
#include "bulletWorld.h"
#include "bulletTriangleMesh.h"

#include "collisionBox.h"
#include "collisionPlane.h"
#include "collisionSphere.h"
#include "collisionPolygon.h"

TypeHandle BulletBodyNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
BulletBodyNode::
BulletBodyNode(const char *name) : PandaNode(name) {

  // Shape
  _shape = new btEmptyShape();

  // Default collide mask
  set_into_collide_mask(CollideMask::all_on());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::get_legal_collide_mask
//       Access: Public, virtual
//  Description: Returns the subset of CollideMask bits that may be
//               set for this particular type of PandaNode.  For 
//               BodyNodes this returns all bits on.
////////////////////////////////////////////////////////////////////
CollideMask BulletBodyNode::
get_legal_collide_mask() const {

  return CollideMask::all_on();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::safe_to_flatten
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to flatten out
//               this particular kind of Node by duplicating
//               instances, false otherwise (for instance, a Camera
//               cannot be safely flattened, because the Camera
//               pointer itself is meaningful).
////////////////////////////////////////////////////////////////////
bool BulletBodyNode::
safe_to_flatten() const {

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::safe_to_transform
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to transform
//               this particular kind of Node by calling the xform()
//               method, false otherwise.  For instance, it's usually
//               a bad idea to attempt to xform a Character.
////////////////////////////////////////////////////////////////////
bool BulletBodyNode::
safe_to_transform() const {

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::safe_to_modify_transform
//       Access: Public, Virtual
//  Description: Returns true if it is safe to automatically adjust
//               the transform on this kind of node.  Usually, this is
//               only a bad idea if the user expects to find a
//               particular transform on the node.
//
//               ModelNodes with the preserve_transform flag set are
//               presently the only kinds of nodes that should not
//               have their transform even adjusted.
////////////////////////////////////////////////////////////////////
bool BulletBodyNode::
safe_to_modify_transform() const {

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::safe_to_combine
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to combine this
//               particular kind of PandaNode with other kinds of
//               PandaNodes of compatible type, adding children or
//               whatever.  For instance, an LODNode should not be
//               combined with any other PandaNode, because its set of
//               children is meaningful.
////////////////////////////////////////////////////////////////////
bool BulletBodyNode::
safe_to_combine() const {

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::safe_to_combine_children
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to combine the
//               children of this PandaNode with each other.  For
//               instance, an LODNode's children should not be
//               combined with each other, because the set of children
//               is meaningful.
////////////////////////////////////////////////////////////////////
bool BulletBodyNode::
safe_to_combine_children() const {

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::safe_to_flatten_below
//       Access: Public, Virtual
//  Description: Returns true if a flatten operation may safely
//               continue past this node, or false if nodes below this
//               node may not be molested.
////////////////////////////////////////////////////////////////////
bool BulletBodyNode::
safe_to_flatten_below() const {

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void BulletBodyNode::
output(ostream &out) const {

  PandaNode::output(out);

  out << " (" << get_num_shapes() << " shapes)";

  out << (is_active() ? " active" : " inactive");

  if (is_static()) out << " static";
  if (is_kinematic()) out << " kinematic";
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::add_shape
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletBodyNode::
add_shape(BulletShape *shape, const TransformState *ts) {

  nassertv(get_object());
  nassertv(ts);

  nassertv(!(shape->ptr()->getShapeType() == CONVEX_HULL_SHAPE_PROXYTYPE 
    && ((btConvexHullShape *)shape->ptr())->getNumVertices() == 0));

  // Transform
  btTransform trans = TransformState_to_btTrans(ts);

  // Reset the shape scaling before we add a shape, and remember the current
  // Scale so we can restore it later...
  NodePath np = NodePath::any_path((PandaNode *)this);
  LVector3 scale = np.get_scale();
  np.set_scale(1.0);

  // Root shape
  btCollisionShape *previous = get_object()->getCollisionShape();
  btCollisionShape *next;

  if (_shapes.size() == 0) {
    nassertv(previous->getShapeType() == EMPTY_SHAPE_PROXYTYPE);

    if (ts->is_identity()) {
      // After adding the shape we will have one shape, but with transform.
      // We need to wrap the shape within a compound shape, in oder to
      // be able to set the local transform.
      next = shape->ptr();
    }
    else {
      // After adding the shape we will have a total of one shape, without 
      // local transform. We can set the shape directly.
      next = new btCompoundShape();
      ((btCompoundShape *)next)->addChildShape(trans, shape->ptr());
    }

    get_object()->setCollisionShape(next);
    _shape = next;

    delete previous;
  }
  else if (_shapes.size() == 1) {
    if (previous->getShapeType() == COMPOUND_SHAPE_PROXYTYPE) {
      // We have one shape, and add another shape. The previous shape is
      // already a compound shape. So we just need to add the second shape
      // to the compound shape.
      next = previous;

      ((btCompoundShape *)next)->addChildShape(trans, shape->ptr());
    }
    else {
      // We have one shape which is NOT a compound shape, and want to add
      // a second shape. We need to wrap both shapes within a compound shape.
      next = new btCompoundShape();

      btTransform previous_trans = btTransform::getIdentity();
      ((btCompoundShape *)next)->addChildShape(previous_trans, previous);
      ((btCompoundShape *)next)->addChildShape(trans, shape->ptr());

      get_object()->setCollisionShape(next);
      _shape = next;
    }
  }
  else {
    // We already have two or more shapes, and want to add another. So we
    // already have a compound shape as wrapper, and just need to add the
    // new shape to the compound.
    nassertv(previous->getShapeType() == COMPOUND_SHAPE_PROXYTYPE);

    next = previous;
    ((btCompoundShape *)next)->addChildShape(trans, shape->ptr());
  }

  _shapes.push_back(shape);

  // Restore the local scaling again
  np.set_scale(scale);

  shape_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::remove_shape
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletBodyNode::
remove_shape(BulletShape *shape) {

  nassertv(get_object());

  BulletShapes::iterator found;
  PT(BulletShape) ptshape = shape;
  found = find(_shapes.begin(), _shapes.end(), ptshape);

  if (found == _shapes.end()) {
    bullet_cat.warning() << "shape not attached" << endl;
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

      // The previous shape might be a compound. Then delete it.
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

      // The compound is no longer required if the remaining shape 
      // has no transform
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

    shape_changed();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::is_identity
//       Access: Private
//  Description: Returns TRUE if the transform is an identity
//               transform, otherwise FALSE.
////////////////////////////////////////////////////////////////////
bool BulletBodyNode::
is_identity(btTransform &trans) {

  btVector3 null(0, 0, 0);

  return (trans.getOrigin() == null 
       && trans.getRotation().getAxis() == null);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::get_shape_pos
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LPoint3 BulletBodyNode::
get_shape_pos(int idx) const {

  nassertr(idx >= 0 && idx < (int)_shapes.size(), LPoint3::zero());
  return get_shape_mat(idx).get_row3(3);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::get_shape_mat
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LMatrix4 BulletBodyNode::
get_shape_mat(int idx) const {

  nassertr(idx >= 0 && idx < (int)_shapes.size(), LMatrix4::ident_mat());

  btCollisionShape *root = get_object()->getCollisionShape();
  if (root->getShapeType() == COMPOUND_SHAPE_PROXYTYPE) {
    btCompoundShape *compound = (btCompoundShape *)root;

    btTransform trans = compound->getChildTransform(idx);
    return btTrans_to_LMatrix4(trans);

    // The above code assumes that shape's index in _shapes member
    // is the same as the shapes index within the compound. If it
    // turns out that this is not always true we could use the
    // following code:
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

  return LMatrix4::ident_mat();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::shape_changed
//       Access: Published
//  Description: Hook which will be called whenever the total shape
//               of a body changed. Used for example to update
//               the mass properties (inertia) of a rigid body.
//               The default implementation does nothing.
////////////////////////////////////////////////////////////////////
void BulletBodyNode::
shape_changed() {

}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::set_deactivation_time
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletBodyNode::
set_deactivation_time(PN_stdfloat dt) {

  get_object()->setDeactivationTime(dt);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::get_deactivation_time
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletBodyNode::
get_deactivation_time() const {

  return get_object()->getDeactivationTime();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::is_active
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool BulletBodyNode::
is_active() const {

  return get_object()->isActive();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::set_active
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletBodyNode::
set_active(bool active, bool force) {

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

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::set_deactivation_enabled
//       Access: Published
//  Description: If true, this object will be deactivated after a
//               certain amount of time has passed without movement.
//               If false, the object will always remain active.
////////////////////////////////////////////////////////////////////
void BulletBodyNode::
set_deactivation_enabled(bool enabled) {

  // Don't change the state if it's currently active and we enable
  // deactivation.
  if (enabled != is_deactivation_enabled()) {

    // It's OK to set to ACTIVE_TAG even if we don't mean to activate it; it
    // will be disabled right away if the deactivation timer has run out.
    int state = (enabled) ? ACTIVE_TAG : DISABLE_DEACTIVATION;
    get_object()->forceActivationState(state);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::is_deactivation_enabled
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool BulletBodyNode::
is_deactivation_enabled() const {

  return (get_object()->getActivationState() != DISABLE_DEACTIVATION);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::check_collision_with
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool BulletBodyNode::
check_collision_with(PandaNode *node) {

  btCollisionObject *obj = BulletWorld::get_collision_object(node);

  if (obj) {
    return get_object()->checkCollideWith(obj);
  }
  else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::get_anisotropic_friction
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
LVecBase3 BulletBodyNode::
get_anisotropic_friction() const {

  return btVector3_to_LVecBase3(get_object()->getAnisotropicFriction());
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::set_anisotropic_friction
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletBodyNode::
set_anisotropic_friction(const LVecBase3 &friction) {

  nassertv(!friction.is_nan());
  get_object()->setAnisotropicFriction(LVecBase3_to_btVector3(friction));
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::has_contact_response
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
bool BulletBodyNode::
has_contact_response() const {

  return get_object()->hasContactResponse();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::get_contact_processing_threshold
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletBodyNode::
get_contact_processing_threshold() const {

  return get_object()->getContactProcessingThreshold();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::set_contact_processing_threshold
//       Access: Published
//  Description: The constraint solver can discard solving
//               contacts, if the distance is above this threshold.
////////////////////////////////////////////////////////////////////
void BulletBodyNode::
set_contact_processing_threshold(PN_stdfloat threshold) {

  get_object()->setContactProcessingThreshold(threshold);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::get_ccd_swept_sphere_radius
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletBodyNode::
get_ccd_swept_sphere_radius() const {

  return get_object()->getCcdSweptSphereRadius();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::set_ccd_swept_sphere_radius
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletBodyNode::
set_ccd_swept_sphere_radius(PN_stdfloat radius) {

  return get_object()->setCcdSweptSphereRadius(radius);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::get_ccd_motion_threshold
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PN_stdfloat BulletBodyNode::
get_ccd_motion_threshold() const {

  return get_object()->getCcdMotionThreshold();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::set_ccd_motion_threshold
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletBodyNode::
set_ccd_motion_threshold(PN_stdfloat threshold) {

  return get_object()->setCcdMotionThreshold(threshold);
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::add_shapes_from_collision_solids
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void BulletBodyNode::
add_shapes_from_collision_solids(CollisionNode *cnode) {

  PT(BulletTriangleMesh) mesh = NULL;

  for (int j=0; j<cnode->get_num_solids(); j++) {
    CPT(CollisionSolid) solid = cnode->get_solid(j);
    TypeHandle type = solid->get_type();

    // CollisionSphere
    if (CollisionSphere::get_class_type() == type) {
      CPT(CollisionSphere) sphere = DCAST(CollisionSphere, solid);
      CPT(TransformState) ts = TransformState::make_pos(sphere->get_center());

      add_shape(BulletSphereShape::make_from_solid(sphere), ts);
    }

    // CollisionBox
    else if (CollisionBox::get_class_type() == type) {
      CPT(CollisionBox) box = DCAST(CollisionBox, solid);
      CPT(TransformState) ts = TransformState::make_pos(box->get_approx_center());

      add_shape(BulletBoxShape::make_from_solid(box), ts);
    }

    // CollisionPlane
    else if (CollisionPlane::get_class_type() == type) {
      CPT(CollisionPlane) plane = DCAST(CollisionPlane, solid);

      add_shape(BulletPlaneShape::make_from_solid(plane));
    }

    // CollisionGeom
    else if (CollisionPolygon::get_class_type() == type) {
      CPT(CollisionPolygon) polygon = DCAST(CollisionPolygon, solid);

      if (!mesh) {
         mesh = new BulletTriangleMesh();
      }

      for (int i=2; i < polygon->get_num_points(); i++ ) {
        LPoint3 p1 = polygon->get_point(0);
        LPoint3 p2 = polygon->get_point(i-1);
        LPoint3 p3 = polygon->get_point(i);

        mesh->add_triangle(p1, p2, p3, true);
      }
    }
  }

  if (mesh && mesh->get_num_triangles() > 0) {
    add_shape(new BulletTriangleMeshShape(mesh, true));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::set_transform_dirty
//       Access: Published
//  Description: This method enforces an update of the Bullet
//               transform, that is copies the scene graph transform
//               to the Bullet transform.
//               This is achieved by alling the protected PandaNode 
//               hook 'transform_changed'. 
////////////////////////////////////////////////////////////////////
void BulletBodyNode::
set_transform_dirty() {

  transform_changed();
}

////////////////////////////////////////////////////////////////////
//     Function: BulletBodyNode::get_shape_bounds
//       Access: Published
//  Description: Returns the current bounds of all collision shapes
//               owned by this body.
////////////////////////////////////////////////////////////////////
BoundingSphere BulletBodyNode::
get_shape_bounds() const {

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
  btScalar radius;

  if (_shape) {
    _shape->getBoundingSphere(center, radius);
  }

  BoundingSphere bounds(btVector3_to_LPoint3(center), (PN_stdfloat)radius);

  return bounds;
}

