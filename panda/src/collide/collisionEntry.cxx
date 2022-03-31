/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionEntry.cxx
 * @author drose
 * @date 2002-03-16
 */

#include "collisionEntry.h"
#include "dcast.h"
#include "indent.h"

TypeHandle CollisionEntry::_type_handle;

/**
 *
 */
CollisionEntry::
CollisionEntry(const CollisionEntry &copy) :
  _from(copy._from),
  _into(copy._into),
  _from_node(copy._from_node),
  _into_node(copy._into_node),
  _from_node_path(copy._from_node_path),
  _into_node_path(copy._into_node_path),
  _into_clip_planes(copy._into_clip_planes),
  _t(copy._t),
  _flags(copy._flags),
  _surface_point(copy._surface_point),
  _surface_normal(copy._surface_normal),
  _interior_point(copy._interior_point),
  _contact_pos(copy._contact_pos),
  _contact_normal(copy._contact_normal)
{
}

/**
 *
 */
void CollisionEntry::
operator = (const CollisionEntry &copy) {
  _from = copy._from;
  _into = copy._into;
  _from_node = copy._from_node;
  _into_node = copy._into_node;
  _from_node_path = copy._from_node_path;
  _into_node_path = copy._into_node_path;
  _into_clip_planes = copy._into_clip_planes;
  _t = copy._t;
  _flags = copy._flags;
  _surface_point = copy._surface_point;
  _surface_normal = copy._surface_normal;
  _interior_point = copy._interior_point;
  _contact_pos = copy._contact_pos;
  _contact_normal = copy._contact_normal;
}

/**
 * Returns the point, on the surface of the "into" object, at which a
 * collision is detected.  This can be thought of as the first point of
 * intersection.  However the contact point is the actual first point of
 * intersection.
 *
 * The point will be converted into whichever coordinate space the caller
 * specifies.
 */
LPoint3 CollisionEntry::
get_surface_point(const NodePath &space) const {
  nassertr(has_surface_point(), LPoint3::zero());
  CPT(TransformState) transform = _into_node_path.get_transform(space);
  return _surface_point * transform->get_mat();
}

/**
 * Returns the surface normal of the "into" object at the point at which a
 * collision is detected.
 *
 * The normal will be converted into whichever coordinate space the caller
 * specifies.
 */
LVector3 CollisionEntry::
get_surface_normal(const NodePath &space) const {
  nassertr(has_surface_normal(), LVector3::zero());
  CPT(TransformState) transform = _into_node_path.get_transform(space);
  return _surface_normal * transform->get_mat();
}

/**
 * Returns the point, within the interior of the "into" object, which
 * represents the depth to which the "from" object has penetrated.  This can
 * also be described as the intersection point on the surface of the "from"
 * object (which is inside the "into" object).  It can be thought of as the
 * deepest point of intersection.
 *
 * The point will be converted into whichever coordinate space the caller
 * specifies.
 */
LPoint3 CollisionEntry::
get_interior_point(const NodePath &space) const {
  if (!has_interior_point()) {
    return get_surface_point(space);
  }
  CPT(TransformState) transform = _into_node_path.get_transform(space);
  return _interior_point * transform->get_mat();
}

/**
 * Simultaneously transforms the surface point, surface normal, and interior
 * point of the collision into the indicated coordinate space.
 *
 * Returns true if all three properties are available, or false if any one of
 * them is not.
 */
bool CollisionEntry::
get_all(const NodePath &space, LPoint3 &surface_point,
        LVector3 &surface_normal, LPoint3 &interior_point) const {
  CPT(TransformState) transform = _into_node_path.get_transform(space);
  const LMatrix4 &mat = transform->get_mat();
  bool all_ok = true;

  if (!has_surface_point()) {
    surface_point = LPoint3::zero();
    all_ok = false;
  } else {
    surface_point = _surface_point * mat;
  }

  if (!has_surface_normal()) {
    surface_normal = LVector3::zero();
    all_ok = false;
  } else {
    surface_normal = _surface_normal * mat;
  }

  if (!has_interior_point()) {
    interior_point = surface_point;
    all_ok = false;
  } else {
    interior_point = _interior_point * mat;
  }

  return all_ok;
}

/**
 * Returns the position of the "from" object at the instant that a collision
 * is first detected.
 *
 * The position will be converted into whichever coordinate space the caller
 * specifies.
 */
LPoint3 CollisionEntry::
get_contact_pos(const NodePath &space) const {
  nassertr(has_contact_pos(), LPoint3::zero());
  CPT(TransformState) transform = _into_node_path.get_transform(space);
  return _contact_pos * transform->get_mat();
}

/**
 * Returns the surface normal of the "into" object at the contact position.
 *
 * The normal will be converted into whichever coordinate space the caller
 * specifies.
 */
LVector3 CollisionEntry::
get_contact_normal(const NodePath &space) const {
  nassertr(has_contact_normal(), LVector3::zero());
  CPT(TransformState) transform = _into_node_path.get_transform(space);
  return _contact_normal * transform->get_mat();
}

/**
 * Simultaneously transforms the contact position and contact normal of the
 * collision into the indicated coordinate space.
 *
 * Returns true if all three properties are available, or false if any one of
 * them is not.
 */
bool CollisionEntry::
get_all_contact_info(const NodePath &space, LPoint3 &contact_pos,
                     LVector3 &contact_normal) const {
  CPT(TransformState) transform = _into_node_path.get_transform(space);
  const LMatrix4 &mat = transform->get_mat();
  bool all_ok = true;

  if (!has_contact_pos()) {
    contact_pos = LPoint3::zero();
    all_ok = false;
  } else {
    contact_pos = _contact_pos * mat;
  }

  if (!has_contact_normal()) {
    contact_normal = LVector3::zero();
    all_ok = false;
  } else {
    contact_normal = _contact_normal * mat;
  }

  return all_ok;
}

/**
 *
 */
void CollisionEntry::
output(std::ostream &out) const {
  out << _from_node_path;
  if (!_into_node_path.is_empty()) {
    out << " into " << _into_node_path;
  }
  if (has_surface_point()) {
    out << " at " << get_surface_point(NodePath());
  }
}

/**
 *
 */
void CollisionEntry::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "CollisionEntry:\n";
  if (!_from_node_path.is_empty()) {
    indent(out, indent_level + 2)
      << "from " << _from_node_path << "\n";
  }
  if (!_into_node_path.is_empty()) {
    indent(out, indent_level + 2)
      << "into " << _into_node_path;

    out << " [";
    _into_node_path.node()->list_tags(out, ", ");
    out << "]";

    const ClipPlaneAttrib *cpa = get_into_clip_planes();
    if (cpa != nullptr) {
      out << " (clipped)";
    }
    out << "\n";
  }
  if (has_surface_point()) {
    indent(out, indent_level + 2)
      << "at " << get_surface_point(NodePath()) << "\n";
  }
  if (has_surface_normal()) {
    indent(out, indent_level + 2)
      << "normal " << get_surface_normal(NodePath()) << "\n";
  }
  if (has_interior_point()) {
    indent(out, indent_level + 2)
      << "interior " << get_interior_point(NodePath())
      << " (depth "
      << (get_interior_point(NodePath()) - get_surface_point(NodePath())).length()
      << ")\n";
  }
  indent(out, indent_level + 2)
    << "respect_prev_transform = " << get_respect_prev_transform() << "\n";
}

/**
 * Checks whether the into_node_path has a ClipPlaneAttrib defined.
 */
void CollisionEntry::
check_clip_planes() {
  _into_clip_planes = DCAST(ClipPlaneAttrib, _into_node_path.get_net_state()->get_attrib(ClipPlaneAttrib::get_class_slot()));
  _flags |= F_checked_clip_planes;
}
