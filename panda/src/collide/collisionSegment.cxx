// Filename: collisionSegment.cxx
// Created by:  drose (30Jan01)
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


#include "collisionSegment.h"
#include "collisionHandler.h"
#include "collisionEntry.h"
#include "config_collide.h"

#include <geom.h>
#include <geomNode.h>
#include <lensNode.h>
#include <lens.h>

#include <geomLine.h>
#include <geometricBoundingVolume.h>

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
int CollisionSegment::
test_intersection(CollisionHandler *record, const CollisionEntry &entry,
                  const CollisionSolid *into) const {
  return into->test_intersection_from_segment(record, entry);
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
  clear_viz_arcs();
  mark_bound_stale();
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
void CollisionSegment::
recompute_bound() {
  BoundedObject::recompute_bound();

  if (_bound->is_of_type(GeometricBoundingVolume::get_class_type())) {
    GeometricBoundingVolume *gbound;
    DCAST_INTO_V(gbound, _bound);

    // This makes the assumption that _a and _b are laid out
    // sequentially in memory.  It works because that's they way
    // they're defined in the class.
    nassertv(&_a + 1 == &_b);
    gbound->around(&_a, &_b + 1);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSegment::recompute_viz
//       Access: Public, Virtual
//  Description: Rebuilds the geometry that will be used to render a
//               visible representation of the collision solid.
////////////////////////////////////////////////////////////////////
void CollisionSegment::
recompute_viz(Node *parent) {
  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "Recomputing viz for " << *this << " on " << *parent << "\n";
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

  GeomNode *viz = new GeomNode("viz-segment");
  viz->add_geom(segment);
  add_other_viz(parent, viz);
}
