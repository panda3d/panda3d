// Filename: collisionSolid.cxx
// Created by:  drose (24Apr00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "collisionSolid.h"
#include "config_collide.h"
#include "collisionSphere.h"
#include "collisionLine.h"
#include "collisionRay.h"
#include "collisionSegment.h"

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

TypeHandle CollisionSolid::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CollisionSolid::
CollisionSolid() {
  _flags = F_viz_geom_stale | F_tangible;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CollisionSolid::
CollisionSolid(const CollisionSolid &copy) :
  _effective_normal(copy._effective_normal),
  _flags(copy._flags)
{
  _flags |= F_viz_geom_stale;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CollisionSolid::
~CollisionSolid() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::test_intersection
//       Access: Public, Virtual
//  Description: Tests for a collision between this object (which is
//               also the "from" object in the entry) and the "into"
//               object.  If a collision is detected, returns a new
//               CollisionEntry object that records the collision;
//               otherwise, returns NULL.
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionSolid::
test_intersection(const CollisionEntry &) const {
  report_undefined_from_intersection(get_type());
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::xform
//       Access: Public, Virtual
//  Description: Transforms the solid by the indicated matrix.
////////////////////////////////////////////////////////////////////
void CollisionSolid::
xform(const LMatrix4f &mat) {
  if ((_flags & F_effective_normal) != 0) {
    _effective_normal = _effective_normal * mat;
    _effective_normal.normalize();
  }

  mark_viz_stale();
  mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::get_viz
//       Access: Public, Virtual
//  Description: Returns a GeomNode that may be rendered to visualize
//               the CollisionSolid.  This is used during the cull
//               traversal to render the CollisionNodes that have been
//               made visible.
////////////////////////////////////////////////////////////////////
PT(PandaNode) CollisionSolid::
get_viz(const CullTraverserData &) const {
  if ((_flags & F_viz_geom_stale) != 0) {
    if (_viz_geom == (GeomNode *)NULL) {
      ((CollisionSolid *)this)->_viz_geom = new GeomNode("viz");
    } else {
      _viz_geom->remove_all_geoms();
    }
    ((CollisionSolid *)this)->fill_viz_geom();
    ((CollisionSolid *)this)->_flags &= ~F_viz_geom_stale;
  }
  return _viz_geom.p();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionSolid::
output(ostream &out) const {
  out << get_type();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionSolid::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << (*this) << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::test_intersection_from_sphere
//       Access: Protected, Virtual
//  Description: This is part of the double-dispatch implementation of
//               test_intersection().  It is called when the "from"
//               object is a sphere.
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionSolid::
test_intersection_from_sphere(const CollisionEntry &) const {
  report_undefined_intersection_test(CollisionSphere::get_class_type(),
                                     get_type());
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::test_intersection_from_line
//       Access: Protected, Virtual
//  Description: This is part of the double-dispatch implementation of
//               test_intersection().  It is called when the "from"
//               object is a line.
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionSolid::
test_intersection_from_line(const CollisionEntry &) const {
  report_undefined_intersection_test(CollisionLine::get_class_type(),
                                     get_type());
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::test_intersection_from_ray
//       Access: Protected, Virtual
//  Description: This is part of the double-dispatch implementation of
//               test_intersection().  It is called when the "from"
//               object is a ray.
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionSolid::
test_intersection_from_ray(const CollisionEntry &) const {
  report_undefined_intersection_test(CollisionRay::get_class_type(),
                                     get_type());
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::test_intersection_from_segment
//       Access: Protected, Virtual
//  Description: This is part of the double-dispatch implementation of
//               test_intersection().  It is called when the "from"
//               object is a segment.
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionSolid::
test_intersection_from_segment(const CollisionEntry &) const {
  report_undefined_intersection_test(CollisionSegment::get_class_type(),
                                     get_type());
  return NULL;
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

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::report_undefined_intersection_test
//       Access: Protected, Static
//  Description: Outputs a message the first time an intersection test
//               is attempted that isn't defined, and explains a bit
//               about what it means.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::report_undefined_from_intersection
//       Access: Protected, Static
//  Description: Outputs a message the first time an intersection test
//               is attempted that isn't defined, and explains a bit
//               about what it means.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void CollisionSolid::
write_datagram(BamWriter *, Datagram &me) {
  // For now, we need only 8 bits of flags.  If we need to expand this
  // later, we will have to increase the bam version.
  me.add_uint8(_flags);
  if ((_flags & F_effective_normal) != 0) {
    _effective_normal.write_datagram(me);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void CollisionSolid::
fillin(DatagramIterator &scan, BamReader *manager) {
  if (manager->get_file_minor_ver() < 7) {
    bool tangible = scan.get_bool();
    if (!tangible) {
      _flags &= ~F_tangible;
    }
  } else {
    _flags = scan.get_uint8();
    if ((_flags & F_effective_normal) != 0) {
      _effective_normal.read_datagram(scan);
    }
  }

  // The viz is always stale after reading from a bam file.
  _flags |= F_viz_geom_stale;
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::fill_viz_geom
//       Access: Protected, Virtual
//  Description: Fills the _viz_geom GeomNode up with Geoms suitable
//               for rendering this solid.
////////////////////////////////////////////////////////////////////
void CollisionSolid::
fill_viz_geom() {
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::get_solid_viz_state
//       Access: Protected
//  Description: Returns a RenderState for rendering collision
//               visualizations in solid.  This automatically returns
//               the appropriate state according to the setting of
//               _tangible.
////////////////////////////////////////////////////////////////////
CPT(RenderState) CollisionSolid::
get_solid_viz_state() {
  // Once someone asks for this pointer, we hold its reference count
  // and never free it.
  static CPT(RenderState) base_state = (const RenderState *)NULL;
  if (base_state == (const RenderState *)NULL) {
    base_state = RenderState::make
      (CullFaceAttrib::make(CullFaceAttrib::M_cull_clockwise),
       RenderModeAttrib::make(RenderModeAttrib::M_filled),
       TransparencyAttrib::make(TransparencyAttrib::M_alpha));
  }

  if (!is_tangible()) {
    static CPT(RenderState) intangible_state = (const RenderState *)NULL;
    if (intangible_state == (const RenderState *)NULL) {
      intangible_state = base_state->add_attrib
        (ColorAttrib::make_flat(Colorf(1.0f, 0.3f, 0.5f, 0.5f)));
    }
    return intangible_state;

  } else if (has_effective_normal()) {
    static CPT(RenderState) fakenormal_state = (const RenderState *)NULL;
    if (fakenormal_state == (const RenderState *)NULL) {
      fakenormal_state = base_state->add_attrib
        (ColorAttrib::make_flat(Colorf(0.5f, 0.5f, 1.0f, 0.5f)));
    }
    return fakenormal_state;

  } else {
    static CPT(RenderState) tangible_state = (const RenderState *)NULL;
    if (tangible_state == (const RenderState *)NULL) {
      tangible_state = base_state->add_attrib
        (ColorAttrib::make_flat(Colorf(1.0f, 1.0f, 1.0f, 0.5f)));
    }
    return tangible_state;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::get_wireframe_viz_state
//       Access: Protected
//  Description: Returns a RenderState for rendering collision
//               visualizations in wireframe.  This automatically returns
//               the appropriate state according to the setting of
//               _tangible.
////////////////////////////////////////////////////////////////////
CPT(RenderState) CollisionSolid::
get_wireframe_viz_state() {
  // Once someone asks for this pointer, we hold its reference count
  // and never free it.
  static CPT(RenderState) base_state = (const RenderState *)NULL;
  if (base_state == (const RenderState *)NULL) {
    base_state = RenderState::make
      (CullFaceAttrib::make(CullFaceAttrib::M_cull_none),
       RenderModeAttrib::make(RenderModeAttrib::M_wireframe),
       TransparencyAttrib::make(TransparencyAttrib::M_none));
  }

  if (is_tangible()) {
    static CPT(RenderState) tangible_state = (const RenderState *)NULL;
    if (tangible_state == (const RenderState *)NULL) {
      tangible_state = base_state->add_attrib
        (ColorAttrib::make_flat(Colorf(0.0f, 0.0f, 1.0f, 1.0f)));
    }
    return tangible_state;

  } else {
    static CPT(RenderState) intangible_state = (const RenderState *)NULL;
    if (intangible_state == (const RenderState *)NULL) {
      intangible_state = base_state->add_attrib
        (ColorAttrib::make_flat(Colorf(1.0f, 1.0f, 0.0f, 1.0f)));
    }
    return intangible_state;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::get_other_viz_state
//       Access: Protected
//  Description: Returns a RenderState for rendering collision
//               visualizations for things that are neither solid nor
//               exactly wireframe, like rays and segments.
////////////////////////////////////////////////////////////////////
CPT(RenderState) CollisionSolid::
get_other_viz_state() {
  // Once someone asks for this pointer, we hold its reference count
  // and never free it.
  static CPT(RenderState) base_state = (const RenderState *)NULL;
  if (base_state == (const RenderState *)NULL) {
    base_state = RenderState::make
      (CullFaceAttrib::make(CullFaceAttrib::M_cull_clockwise),
       RenderModeAttrib::make(RenderModeAttrib::M_filled),
       TransparencyAttrib::make(TransparencyAttrib::M_alpha));
  }

  // We don't bother to make a distinction here between tangible and
  // intangible.
  return base_state;
}

