// Filename: collisionRay.cxx
// Created by:  drose (22Jun00)
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
#include <projectionNode.h>
#include <projection.h>

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
//     Function: CollisionRay::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CollisionRay::
output(ostream &out) const {
  out << "ray, o (" << _origin << "), d (" << _direction << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionRay::set_projection
//       Access: Public
//  Description: Accepts a ProjectionNode and a 2-d point in the range
//               [-1,1].  Sets the CollisionRay so that it begins at
//               the ProjectionNode's near plane and extends to
//               infinity, making it suitable for picking objects from
//               the screen given a camera and a mouse location.
//
//               Returns true if the point was acceptable, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool CollisionRay::
set_projection(ProjectionNode *camera, const LPoint2f &point) {
  Projection *proj = camera->get_projection();

  bool success = true;
  if (!proj->extrude(point, _origin, _direction)) {
    _origin = LPoint3f::origin();
    _direction = LVector3f::forward();
    success = false;
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
void CollisionRay::
recompute_bound() {
  BoundedObject::recompute_bound();
  // Less than ideal: we throw away whatever we just allocated in
  // BoundedObject.
  _bound = new BoundingLine(_origin, _origin + _direction);
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
  for (int i = 0; i < 100; i++) {
    verts.push_back(_origin + (double)i * _direction);
    colors.push_back(Colorf(1.0, 1.0, 1.0, 1.0) + 
		     ((double)i / 100.0) * Colorf(0.0, 0.0, 0.0, -1.0));
  }
  ray->set_coords(verts, G_PER_VERTEX);
  ray->set_colors(colors, G_PER_VERTEX);

  PTA_int lengths;
  lengths.push_back(99);
  ray->set_lengths(lengths);

  ray->set_num_prims(1);

  GeomNode *viz = new GeomNode("viz-ray");
  viz->add_geom(ray);
  add_other_viz(parent, viz);
}
