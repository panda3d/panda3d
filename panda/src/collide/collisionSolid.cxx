/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionSolid.cxx
 * @author drose
 * @date 2000-04-24
 */

#include "collisionSolid.h"
#include "config_collide.h"
#include "collisionSphere.h"
#include "collisionLine.h"
#include "collisionRay.h"
#include "collisionSegment.h"
#include "collisionCapsule.h"
#include "collisionParabola.h"
#include "collisionBox.h"
#include "collisionEntry.h"
#include "boundingSphere.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "indent.h"
#include "cullFaceAttrib.h"
#include "colorAttrib.h"
#include "renderModeAttrib.h"
#include "transparencyAttrib.h"
#include "geomNode.h"

PStatCollector CollisionSolid::_volume_pcollector(
  "Collision Volumes:CollisionSolid");
PStatCollector CollisionSolid::_test_pcollector(
  "Collision Tests:CollisionSolid");
TypeHandle CollisionSolid::_type_handle;

/**
 *
 */
CollisionSolid::
CollisionSolid() : _lock("CollisionSolid") {
  _flags = F_viz_geom_stale | F_tangible | F_internal_bounds_stale;
}

/**
 *
 */
CollisionSolid::
CollisionSolid(const CollisionSolid &copy) :
  CopyOnWriteObject(copy),
  _effective_normal(copy._effective_normal),
  _internal_bounds(copy._internal_bounds),
  _flags(copy._flags),
  _lock("CollisionSolid")
{
  _flags |= F_viz_geom_stale;
}

/**
 *
 */
CollisionSolid::
~CollisionSolid() {
}

/**
 * Required to implement CopyOnWriteObject.
 */
PT(CopyOnWriteObject) CollisionSolid::
make_cow_copy() {
  return make_copy();
}

/**
 * Returns the solid's bounding volume.
 */
CPT(BoundingVolume) CollisionSolid::
get_bounds() const {
  LightMutexHolder holder(_lock);
  if (_flags & F_internal_bounds_stale) {
    ((CollisionSolid *)this)->_internal_bounds = compute_internal_bounds();
    ((CollisionSolid *)this)->_flags &= ~F_internal_bounds_stale;
  }
  return _internal_bounds;
}

/**
 * Returns the solid's bounding volume.
 */
void CollisionSolid::
set_bounds(const BoundingVolume &bounding_volume) {
  LightMutexHolder holder(_lock);
  ((CollisionSolid *)this)->_internal_bounds = bounding_volume.make_copy();
  ((CollisionSolid *)this)->_flags &= ~F_internal_bounds_stale;
}

/**
 * Tests for a collision between this object (which is also the "from" object
 * in the entry) and the "into" object.  If a collision is detected, returns a
 * new CollisionEntry object that records the collision; otherwise, returns
 * NULL.
 */
PT(CollisionEntry) CollisionSolid::
test_intersection(const CollisionEntry &) const {
  report_undefined_from_intersection(get_type());
  return nullptr;
}

/**
 * Transforms the solid by the indicated matrix.
 */
void CollisionSolid::
xform(const LMatrix4 &mat) {
  LightMutexHolder holder(_lock);
  if ((_flags & F_effective_normal) != 0) {
    _effective_normal = _effective_normal * mat;
    _effective_normal.normalize();
  }

  _flags |= F_viz_geom_stale | F_internal_bounds_stale;
}

/**
 * Returns a GeomNode that may be rendered to visualize the CollisionSolid.
 * This is used during the cull traversal to render the CollisionNodes that
 * have been made visible.
 */
PT(PandaNode) CollisionSolid::
get_viz(const CullTraverser *, const CullTraverserData &, bool bounds_only) const {
  LightMutexHolder holder(_lock);
  if ((_flags & F_viz_geom_stale) != 0) {
    if (_viz_geom == nullptr) {
      ((CollisionSolid *)this)->_viz_geom = new GeomNode("viz");
      ((CollisionSolid *)this)->_bounds_viz_geom = new GeomNode("bounds_viz");
    } else {
      _viz_geom->remove_all_geoms();
      _bounds_viz_geom->remove_all_geoms();
    }
    ((CollisionSolid *)this)->fill_viz_geom();
    ((CollisionSolid *)this)->_flags &= ~F_viz_geom_stale;
  }

  if (bounds_only) {
    return _bounds_viz_geom;
  } else {
    return _viz_geom;
  }
}

/**
 * Returns a PStatCollector that is used to count the number of bounding
 * volume tests made against a solid of this type in a given frame.
 */
PStatCollector &CollisionSolid::
get_volume_pcollector() {
  return _volume_pcollector;
}

/**
 * Returns a PStatCollector that is used to count the number of intersection
 * tests made against a solid of this type in a given frame.
 */
PStatCollector &CollisionSolid::
get_test_pcollector() {
  return _test_pcollector;
}

/**
 *
 */
void CollisionSolid::
output(std::ostream &out) const {
  out << get_type();
}

/**
 *
 */
void CollisionSolid::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << (*this) << "\n";
}

/**
 *
 */
PT(BoundingVolume) CollisionSolid::
compute_internal_bounds() const {
  return new BoundingSphere;
}

/**
 * This is part of the double-dispatch implementation of test_intersection().
 * It is called when the "from" object is a sphere.
 */
PT(CollisionEntry) CollisionSolid::
test_intersection_from_sphere(const CollisionEntry &) const {
  report_undefined_intersection_test(CollisionSphere::get_class_type(),
                                     get_type());
  return nullptr;
}

/**
 * This is part of the double-dispatch implementation of test_intersection().
 * It is called when the "from" object is a line.
 */
PT(CollisionEntry) CollisionSolid::
test_intersection_from_line(const CollisionEntry &) const {
  report_undefined_intersection_test(CollisionLine::get_class_type(),
                                     get_type());
  return nullptr;
}

/**
 * This is part of the double-dispatch implementation of test_intersection().
 * It is called when the "from" object is a ray.
 */
PT(CollisionEntry) CollisionSolid::
test_intersection_from_ray(const CollisionEntry &) const {
  report_undefined_intersection_test(CollisionRay::get_class_type(),
                                     get_type());
  return nullptr;
}

/**
 * This is part of the double-dispatch implementation of test_intersection().
 * It is called when the "from" object is a segment.
 */
PT(CollisionEntry) CollisionSolid::
test_intersection_from_segment(const CollisionEntry &) const {
  report_undefined_intersection_test(CollisionSegment::get_class_type(),
                                     get_type());
  return nullptr;
}

/**
 * This is part of the double-dispatch implementation of test_intersection().
 * It is called when the "from" object is a capsule.
 */
PT(CollisionEntry) CollisionSolid::
test_intersection_from_capsule(const CollisionEntry &) const {
  report_undefined_intersection_test(CollisionCapsule::get_class_type(),
                                     get_type());
  return nullptr;
}

/**
 * This is part of the double-dispatch implementation of test_intersection().
 * It is called when the "from" object is a parabola.
 */
PT(CollisionEntry) CollisionSolid::
test_intersection_from_parabola(const CollisionEntry &) const {
  report_undefined_intersection_test(CollisionParabola::get_class_type(),
                                     get_type());
  return nullptr;
}

/**
 * This is part of the double-dispatch implementation of test_intersection().
 * It is called when the "from" object is a box.
 */
PT(CollisionEntry) CollisionSolid::
test_intersection_from_box(const CollisionEntry &) const {
  report_undefined_intersection_test(CollisionBox::get_class_type(),
                                     get_type());
  return nullptr;
}


#ifndef NDEBUG
class CollisionSolidUndefinedPair {
public:
  CollisionSolidUndefinedPair(TypeHandle a, TypeHandle b) :
    _a(a), _b(b)
  {}
  bool operator < (const CollisionSolidUndefinedPair &other) const {
    if (_a != other._a) {
      return _a < other._a;
    }
    return _b < other._b;
  }

  TypeHandle _a;
  TypeHandle _b;
};
#endif  // NDEBUG

/**
 * Outputs a message the first time an intersection test is attempted that
 * isn't defined, and explains a bit about what it means.
 */
void CollisionSolid::
report_undefined_intersection_test(TypeHandle from_type, TypeHandle into_type) {
#ifndef NDEBUG
  typedef pset<CollisionSolidUndefinedPair> Reported;
  static Reported reported;

  if (reported.insert(CollisionSolidUndefinedPair(from_type, into_type)).second) {
    collide_cat.error()
      << "Invalid attempt to detect collision from " << from_type << " into "
      << into_type << "!\n\n"

      "This means that a " << from_type << " object attempted to test for an\n"
      "intersection into a " << into_type << " object.  This intersection\n"
      "test has not yet been defined; it is possible the " << into_type << "\n"
      "object is not intended to be collidable.  Consider calling\n"
      "set_into_collide_mask(0) on the " << into_type << " object, or\n"
      "set_from_collide_mask(0) on the " << from_type << " object.\n\n";
  }
#endif  // NDEBUG
}

/**
 * Outputs a message the first time an intersection test is attempted that
 * isn't defined, and explains a bit about what it means.
 */
void CollisionSolid::
report_undefined_from_intersection(TypeHandle from_type) {
#ifndef NDEBUG
  typedef pset<TypeHandle> Reported;
  static Reported reported;

  if (reported.insert(from_type).second) {
    collide_cat.error()
      << "Invalid attempt to detect collision from " << from_type << "!\n\n"

      "This means that a " << from_type << " object was added to a\n"
      "CollisionTraverser as if it were a colliding object.  However,\n"
      "no implementation for this kind of object has yet been defined\n"
      "to collide with other objects.\n\n";
  }
#endif  // NDEBUG
}

/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
void CollisionSolid::
write_datagram(BamWriter *, Datagram &me) {
  // For now, we need only 8 bits of flags.  If we need to expand this later,
  // we will have to increase the bam version.
  LightMutexHolder holder(_lock);
  me.add_uint8(_flags);
  if ((_flags & F_effective_normal) != 0) {
    _effective_normal.write_datagram(me);
  }
}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
void CollisionSolid::
fillin(DatagramIterator &scan, BamReader *manager) {
  _flags = scan.get_uint8();
  if ((_flags & F_effective_normal) != 0) {
    _effective_normal.read_datagram(scan);
  }

  // The viz is always stale after reading from a bam file.  So is the
  // bounding volume.
  _flags |= F_viz_geom_stale | F_internal_bounds_stale;
}


/**
 * Fills the _viz_geom GeomNode up with Geoms suitable for rendering this
 * solid.
 */
void CollisionSolid::
fill_viz_geom() {
}

/**
 * Returns a RenderState for rendering collision visualizations in solid.
 * This automatically returns the appropriate state according to the setting
 * of _tangible.
 *
 * Assumes the lock is already held.
 */
CPT(RenderState) CollisionSolid::
get_solid_viz_state() {
  // Once someone asks for this pointer, we hold its reference count and never
  // free it.
  static CPT(RenderState) base_state = nullptr;
  if (base_state == nullptr) {
    base_state = RenderState::make
      (CullFaceAttrib::make(CullFaceAttrib::M_cull_clockwise),
       RenderModeAttrib::make(RenderModeAttrib::M_filled),
       TransparencyAttrib::make(TransparencyAttrib::M_alpha));
  }

  if (!do_is_tangible()) {
    static CPT(RenderState) intangible_state = nullptr;
    if (intangible_state == nullptr) {
      intangible_state = base_state->add_attrib
        (ColorAttrib::make_flat(LColor(1.0f, 0.3, 0.5f, 0.5f)));
    }
    return intangible_state;

  } else if (do_has_effective_normal()) {
    static CPT(RenderState) fakenormal_state = nullptr;
    if (fakenormal_state == nullptr) {
      fakenormal_state = base_state->add_attrib
        (ColorAttrib::make_flat(LColor(0.5f, 0.5f, 1.0f, 0.5f)));
    }
    return fakenormal_state;

  } else {
    static CPT(RenderState) tangible_state = nullptr;
    if (tangible_state == nullptr) {
      tangible_state = base_state->add_attrib
        (ColorAttrib::make_flat(LColor(1.0f, 1.0f, 1.0f, 0.5f)));
    }
    return tangible_state;
  }
}


/**
 * Returns a RenderState for rendering collision visualizations in wireframe.
 * This automatically returns the appropriate state according to the setting
 * of _tangible.
 *
 * Assumes the lock is already held.
 */
CPT(RenderState) CollisionSolid::
get_wireframe_viz_state() {
  // Once someone asks for this pointer, we hold its reference count and never
  // free it.
  static CPT(RenderState) base_state = nullptr;
  if (base_state == nullptr) {
    base_state = RenderState::make
      (CullFaceAttrib::make(CullFaceAttrib::M_cull_none),
       RenderModeAttrib::make(RenderModeAttrib::M_wireframe),
       TransparencyAttrib::make(TransparencyAttrib::M_none));
  }

  if (!do_is_tangible()) {
    static CPT(RenderState) intangible_state = nullptr;
    if (intangible_state == nullptr) {
      intangible_state = base_state->add_attrib
        (ColorAttrib::make_flat(LColor(1.0f, 1.0f, 0.0f, 1.0f)));
    }
    return intangible_state;

  } else if (do_has_effective_normal()) {
    static CPT(RenderState) fakenormal_state = nullptr;
    if (fakenormal_state == nullptr) {
      fakenormal_state = base_state->add_attrib
        (ColorAttrib::make_flat(LColor(0.0f, 0.0f, 1.0f, 1.0f)));
    }
    return fakenormal_state;

  } else {
    static CPT(RenderState) tangible_state = nullptr;
    if (tangible_state == nullptr) {
      tangible_state = base_state->add_attrib
        (ColorAttrib::make_flat(LColor(0.0f, 0.0f, 1.0f, 1.0f)));
    }
    return tangible_state;
  }
}


/**
 * Returns a RenderState for rendering collision visualizations for things
 * that are neither solid nor exactly wireframe, like rays and segments.
 *
 * Assumes the lock is already held.
 */
CPT(RenderState) CollisionSolid::
get_other_viz_state() {
  // Once someone asks for this pointer, we hold its reference count and never
  // free it.
  static CPT(RenderState) base_state = nullptr;
  if (base_state == nullptr) {
    base_state = RenderState::make
      (CullFaceAttrib::make(CullFaceAttrib::M_cull_clockwise),
       RenderModeAttrib::make(RenderModeAttrib::M_filled),
       TransparencyAttrib::make(TransparencyAttrib::M_alpha));
  }

  // We don't bother to make a distinction here between tangible and
  // intangible.
  return base_state;
}

/**
 * Returns a RenderState for rendering collision visualizations in solid.
 * This automatically returns the appropriate state according to the setting
 * of _tangible.
 *
 * Assumes the lock is already held.
 */
CPT(RenderState) CollisionSolid::
get_solid_bounds_viz_state() {
  // Once someone asks for this pointer, we hold its reference count and never
  // free it.
  static CPT(RenderState) base_state = nullptr;
  if (base_state == nullptr) {
    base_state = RenderState::make
      (CullFaceAttrib::make(CullFaceAttrib::M_cull_clockwise),
       RenderModeAttrib::make(RenderModeAttrib::M_filled),
       TransparencyAttrib::make(TransparencyAttrib::M_alpha));
  }

  if (!do_is_tangible()) {
    static CPT(RenderState) intangible_state = nullptr;
    if (intangible_state == nullptr) {
      intangible_state = base_state->add_attrib
        (ColorAttrib::make_flat(LColor(1.0f, 1.0f, 0.5f, 0.3)));
    }
    return intangible_state;

  } else if (do_has_effective_normal()) {
    static CPT(RenderState) fakenormal_state = nullptr;
    if (fakenormal_state == nullptr) {
      fakenormal_state = base_state->add_attrib
        (ColorAttrib::make_flat(LColor(0.5f, 0.5f, 1.0f, 0.3)));
    }
    return fakenormal_state;

  } else {
    static CPT(RenderState) tangible_state = nullptr;
    if (tangible_state == nullptr) {
      tangible_state = base_state->add_attrib
        (ColorAttrib::make_flat(LColor(1.0f, 1.0f, 0.5f, 0.3)));
    }
    return tangible_state;
  }
}


/**
 * Returns a RenderState for rendering collision visualizations in wireframe.
 * This automatically returns the appropriate state according to the setting
 * of _tangible.
 *
 * Assumes the lock is already held.
 */
CPT(RenderState) CollisionSolid::
get_wireframe_bounds_viz_state() {
  // Once someone asks for this pointer, we hold its reference count and never
  // free it.
  static CPT(RenderState) base_state = nullptr;
  if (base_state == nullptr) {
    base_state = RenderState::make
      (CullFaceAttrib::make(CullFaceAttrib::M_cull_none),
       RenderModeAttrib::make(RenderModeAttrib::M_wireframe),
       TransparencyAttrib::make(TransparencyAttrib::M_none),
       ColorAttrib::make_flat(LColor(1.0f, 0.0f, 0.0f, 1.0f)));
  }

  return base_state;
}


/**
 * Returns a RenderState for rendering collision visualizations for things
 * that are neither solid nor exactly wireframe, like rays and segments.
 *
 * Assumes the lock is already held.
 */
CPT(RenderState) CollisionSolid::
get_other_bounds_viz_state() {
  // Once someone asks for this pointer, we hold its reference count and never
  // free it.
  static CPT(RenderState) base_state = nullptr;
  if (base_state == nullptr) {
    base_state = RenderState::make
      (CullFaceAttrib::make(CullFaceAttrib::M_cull_clockwise),
       RenderModeAttrib::make(RenderModeAttrib::M_filled),
       TransparencyAttrib::make(TransparencyAttrib::M_alpha));
  }

  // We don't bother to make a distinction here between tangible and
  // intangible.
  return base_state;
}
