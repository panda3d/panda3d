// Filename: collisionRay.cxx
// Created by:  drose (22Jun00)
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

#include "collisionRay.h"
#include "collisionHandler.h"
#include "collisionEntry.h"
#include "config_collide.h"
#include <geom.h>
#include <geomNode.h>
#include <geomLinestrip.h>
#include <boundingLine.h>
#include <lensNode.h>
#include <lens.h>

TypeHandle CollisionRay::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: CollisionRay::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CollisionSolid *CollisionRay::
make_copy() {
  return new CollisionRay(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRay::test_intersection
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int CollisionRay::
test_intersection(CollisionHandler *record, const CollisionEntry &entry,
                  const CollisionSolid *into) const {
  return into->test_intersection_from_ray(record, entry);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRay::xform
//       Access: Public, Virtual
//  Description: Transforms the solid by the indicated matrix.
////////////////////////////////////////////////////////////////////
void CollisionRay::
xform(const LMatrix4f &mat) {
  _origin = _origin * mat;
  _direction = _direction * mat;
  clear_viz_arcs();
  mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRay::get_collision_origin
//       Access: Public, Virtual
//  Description: Returns the point in space deemed to be the "origin"
//               of the solid for collision purposes.  The closest
//               intersection point to this origin point is considered
//               to be the most significant.
////////////////////////////////////////////////////////////////////
LPoint3f CollisionRay::
get_collision_origin() const {
  return get_origin();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRay::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionRay::
output(ostream &out) const {
  out << "ray, o (" << _origin << "), d (" << _direction << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRay::set_from_lens
//       Access: Public
//  Description: Accepts a LensNode and a 2-d point in the range
//               [-1,1].  Sets the CollisionRay so that it begins at
//               the LensNode's near plane and extends to
//               infinity, making it suitable for picking objects from
//               the screen given a camera and a mouse location.
//
//               Returns true if the point was acceptable, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool CollisionRay::
set_from_lens(LensNode *camera, const LPoint2f &point) {
  Lens *lens = camera->get_lens();

  bool success = true;
  LPoint3f near_point, far_point;
  if (!lens->extrude(point, near_point, far_point)) {
    _origin = LPoint3f::origin();
    _direction = LVector3f::forward();
    success = false;
  } else {
    _origin = near_point;
    _direction = far_point - near_point;
  }

  mark_bound_stale();
  mark_viz_stale();

  return success;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRay::recompute_bound
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
BoundingVolume *CollisionRay::
recompute_bound() {
  BoundedObject::recompute_bound();
  // Less than ideal: we throw away whatever we just allocated in
  // BoundedObject.
  return set_bound_ptr(new BoundingLine(_origin, _origin + _direction));
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRay::recompute_viz
//       Access: Public, Virtual
//  Description: Rebuilds the geometry that will be used to render a
//               visible representation of the collision solid.
////////////////////////////////////////////////////////////////////
void CollisionRay::
recompute_viz(Node *parent) {
  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "Recomputing viz for " << *this << " on " << *parent << "\n";
  }

  GeomLinestrip *ray = new GeomLinestrip;
  PTA_Vertexf verts;
  PTA_Colorf colors;
  PTA_int lengths;
  
  #define NUM_POINTS 100
  verts.reserve(NUM_POINTS);
  colors.reserve(NUM_POINTS);

  for (int i = 0; i < NUM_POINTS; i++) {
    verts.push_back(_origin + (double)i * _direction);
    colors.push_back(Colorf(1.0f, 1.0f, 1.0f, 1.0f)+
                     ((double)i / 100.0) * Colorf(0.0f, 0.0f, 0.0f, -1.0f));
  }
  ray->set_coords(verts);
  ray->set_colors(colors, G_PER_VERTEX);

  lengths.push_back(NUM_POINTS-1);
  ray->set_lengths(lengths);

  ray->set_num_prims(1);

  GeomNode *viz = new GeomNode("viz-ray");
  viz->add_geom(ray);
  add_other_viz(parent, viz);
}
