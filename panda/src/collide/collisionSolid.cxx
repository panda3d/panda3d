// Filename: collisionSolid.cxx
// Created by:  drose (24Apr00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


#include "collisionSolid.h"
#include "config_collide.h"
#include "collisionSphere.h"
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
#include "qpgeomNode.h"

TypeHandle CollisionSolid::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CollisionSolid::
CollisionSolid() {
  _viz_geom_stale = true;
  _tangible = true;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CollisionSolid::
CollisionSolid(const CollisionSolid &copy) :
  _tangible(copy._tangible)
{
  // Actually, there's not a whole lot here we want to copy.
  _viz_geom_stale = true;
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
//     Function: CollisionSolid::get_viz
//       Access: Public
//  Description: Returns a GeomNode that may be rendered to visualize
//               the CollisionSolid.  This is used during the cull
//               traversal to render the CollisionNodes that have been
//               made visible.
////////////////////////////////////////////////////////////////////
qpGeomNode *CollisionSolid::
get_viz() {
  if (_viz_geom_stale) {
    if (_viz_geom == (qpGeomNode *)NULL) {
      _viz_geom = new qpGeomNode("viz");
    } else {
      _viz_geom->remove_all_geoms();
    }
    fill_viz_geom();
    _viz_geom_stale = false;
  }
  return _viz_geom;
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
//  Description:
////////////////////////////////////////////////////////////////////
int CollisionSolid::
test_intersection_from_sphere(qpCollisionHandler *,
                              const qpCollisionEntry &) const {
  report_undefined_intersection_test(CollisionSphere::get_class_type(),
                                     get_type());
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::test_intersection_from_ray
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int CollisionSolid::
test_intersection_from_ray(qpCollisionHandler *,
                           const qpCollisionEntry &) const {
  report_undefined_intersection_test(CollisionRay::get_class_type(),
                                     get_type());
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::test_intersection_from_segment
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int CollisionSolid::
test_intersection_from_segment(qpCollisionHandler *,
                               const qpCollisionEntry &) const {
  report_undefined_intersection_test(CollisionSegment::get_class_type(),
                                     get_type());
  return 0;
}


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
  typedef pmap<TypeHandle, TypeHandle> Reported;
  static Reported reported;

  if (reported.insert(Reported::value_type(from_type, into_type)).second) {
    collide_cat.error()
      << "Invalid attempt to detect collision from " << from_type << " into "
      << into_type << "!\n"

      "This means that a " << from_type << " object attempted to test for a\n"
      "intersection into a " << into_type << " object.  This intersection\n"
      "test has not yet been defined; it is possible the " << into_type << "\n"
      "object is not intended to be collidable.  Consider calling\n"
      "set_into_collide_mask(0) on the " << into_type << " object, or\n"
      "set_from_collide_mask(0) on the " << from_type << " object.\n";
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSolid::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void CollisionSolid::
write_datagram(BamWriter *, Datagram &me)
{
  me.add_uint8(_tangible);
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
fillin(DatagramIterator& scan, BamReader*)
{
  _tangible = (scan.get_uint8() != 0);
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

  if (_tangible) {
    static CPT(RenderState) tangible_state = (const RenderState *)NULL;
    if (tangible_state == (const RenderState *)NULL) {
      tangible_state = base_state->add_attrib
        (ColorAttrib::make_flat(Colorf(1.0f, 1.0f, 1.0f, 0.5f)));
    }
    return tangible_state;

  } else {
    static CPT(RenderState) intangible_state = (const RenderState *)NULL;
    if (intangible_state == (const RenderState *)NULL) {
      intangible_state = base_state->add_attrib
        (ColorAttrib::make_flat(Colorf(1.0f, 0.3f, 0.5f, 0.5f)));
    }
    return intangible_state;
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

  if (_tangible) {
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

