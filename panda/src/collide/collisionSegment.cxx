// Filename: collisionSegment.cxx
// Created by:  drose (30Jan01)
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


#include "collisionSegment.h"
#include "collisionHandler.h"
#include "collisionEntry.h"
#include "config_collide.h"

#include "geom.h"
#include "lensNode.h"
#include "geomNode.h"
#include "lens.h"
#include "geomLine.h"
#include "geometricBoundingVolume.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle CollisionSegment::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CollisionSolid *CollisionSegment::
make_copy() {
  return new CollisionSegment(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::test_intersection
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionSegment::
test_intersection(const CollisionEntry &entry) const {
  return entry.get_into()->test_intersection_from_segment(entry);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::xform
//       Access: Public, Virtual
//  Description: Transforms the solid by the indicated matrix.
////////////////////////////////////////////////////////////////////
void CollisionSegment::
xform(const LMatrix4f &mat) {
  _a = _a * mat;
  _b = _b * mat;

  CollisionSolid::xform(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::get_collision_origin
//       Access: Public, Virtual
//  Description: Returns the point in space deemed to be the "origin"
//               of the solid for collision purposes.  The closest
//               intersection point to this origin point is considered
//               to be the most significant.
////////////////////////////////////////////////////////////////////
LPoint3f CollisionSegment::
get_collision_origin() const {
  return get_point_a();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionSegment::
output(ostream &out) const {
  out << "segment, a (" << _a << "), b (" << _b << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::set_from_lens
//       Access: Public
//  Description: Accepts a LensNode and a 2-d point in the range
//               [-1,1].  Sets the CollisionSegment so that it begins at
//               the LensNode's near plane and extends to the
//               far plane, making it suitable for picking objects
//               from the screen given a camera and a mouse location.
//
//               Returns true if the point was acceptable, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool CollisionSegment::
set_from_lens(LensNode *camera, const LPoint2f &point) {
  Lens *proj = camera->get_lens();

  bool success = true;
  if (!proj->extrude(point, _a, _b)) {
    _a = LPoint3f::origin();
    _b = _a + LVector3f::forward();
    success = false;
  }

  mark_bound_stale();
  mark_viz_stale();

  return success;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::recompute_bound
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
BoundingVolume *CollisionSegment::
recompute_bound() {
  BoundingVolume *bound = BoundedObject::recompute_bound();

  if (bound->is_of_type(GeometricBoundingVolume::get_class_type())) {
    GeometricBoundingVolume *gbound;
    DCAST_INTO_R(gbound, bound, bound);

    // This makes the assumption that _a and _b are laid out
    // sequentially in memory.  It works because that's they way
    // they're defined in the class.
    nassertr(&_a + 1 == &_b, bound);
    gbound->around(&_a, &_b + 1);
  }

  return bound;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::fill_viz_geom
//       Access: Protected, Virtual
//  Description: Fills the _viz_geom GeomNode up with Geoms suitable
//               for rendering this solid.
////////////////////////////////////////////////////////////////////
void CollisionSegment::
fill_viz_geom() {
  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "Recomputing viz for " << *this << "\n";
  }

  GeomLine *segment = new GeomLine;
  PTA_Vertexf verts;
  PTA_Colorf colors;
  verts.push_back(_a);
  verts.push_back(_b);
  colors.push_back(Colorf(1.0f, 1.0f, 1.0f, 1.0f));
  segment->set_coords(verts);
  segment->set_colors(colors, G_OVERALL);

  segment->set_num_prims(1);

  _viz_geom->add_geom(segment, get_other_viz_state());
  _bounds_viz_geom->add_geom(segment, get_other_bounds_viz_state());
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               CollisionSegment.
////////////////////////////////////////////////////////////////////
void CollisionSegment::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void CollisionSegment::
write_datagram(BamWriter *manager, Datagram &dg) {
  CollisionSolid::write_datagram(manager, dg);
  _a.write_datagram(dg);
  _b.write_datagram(dg);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type CollisionSegment is encountered
//               in the Bam file.  It should create the CollisionSegment
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *CollisionSegment::
make_from_bam(const FactoryParams &params) {
  CollisionSegment *node = new CollisionSegment();
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new CollisionSegment.
////////////////////////////////////////////////////////////////////
void CollisionSegment::
fillin(DatagramIterator &scan, BamReader *manager) {
  CollisionSolid::fillin(scan, manager);
  _a.read_datagram(scan);
  _b.read_datagram(scan);
}
