/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionRay.cxx
 * @author drose
 * @date 2000-06-22
 */

#include "collisionRay.h"
#include "collisionHandler.h"
#include "collisionEntry.h"
#include "config_collide.h"
#include "geom.h"
#include "geomNode.h"
#include "boundingLine.h"
#include "lensNode.h"
#include "lens.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "geom.h"
#include "geomLinestrips.h"
#include "geomVertexWriter.h"

TypeHandle CollisionRay::_type_handle;


/**
 *
 */
CollisionSolid *CollisionRay::
make_copy() {
  return new CollisionRay(*this);
}

/**
 *
 */
PT(CollisionEntry) CollisionRay::
test_intersection(const CollisionEntry &entry) const {
  return entry.get_into()->test_intersection_from_ray(entry);
}

/**
 * Transforms the solid by the indicated matrix.
 */
void CollisionRay::
xform(const LMatrix4 &mat) {
  _origin = _origin * mat;
  _direction = _direction * mat;

  CollisionSolid::xform(mat);
}

/**
 * Returns the point in space deemed to be the "origin" of the solid for
 * collision purposes.  The closest intersection point to this origin point is
 * considered to be the most significant.
 */
LPoint3 CollisionRay::
get_collision_origin() const {
  return get_origin();
}

/**
 *
 */
void CollisionRay::
output(std::ostream &out) const {
  out << "ray, o (" << get_origin() << "), d (" << get_direction() << ")";
}

/**
 * Accepts a LensNode and a 2-d point in the range [-1,1].  Sets the
 * CollisionRay so that it begins at the LensNode's near plane and extends to
 * infinity, making it suitable for picking objects from the screen given a
 * camera and a mouse location.
 *
 * Returns true if the point was acceptable, false otherwise.
 */
bool CollisionRay::
set_from_lens(LensNode *camera, const LPoint2 &point) {
  Lens *lens = camera->get_lens();

  bool success = true;
  LPoint3 near_point, far_point;
  if (!lens->extrude(point, near_point, far_point)) {
    _origin = LPoint3::origin();
    _direction = LVector3::forward();
    success = false;
  } else {
    _origin = near_point;
    _direction = far_point - near_point;
  }

  mark_internal_bounds_stale();
  mark_viz_stale();

  return success;
}

/**
 *
 */
PT(BoundingVolume) CollisionRay::
compute_internal_bounds() const {
  return new BoundingLine(_origin, _origin + _direction);
}

/**
 * Fills the _viz_geom GeomNode up with Geoms suitable for rendering this
 * solid.
 */
void CollisionRay::
fill_viz_geom() {
  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "Recomputing viz for " << *this << "\n";
  }

  static const int num_points = 100;
  static const double scale = 100.0;

  PT(GeomVertexData) vdata = new GeomVertexData
    ("collision", GeomVertexFormat::get_v3cp(),
     Geom::UH_static);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter color(vdata, InternalName::get_color());

  for (int i = 0; i < num_points; i++) {
    double t = ((double)i / (double)num_points);
    vertex.add_data3(get_origin() + t * scale * get_direction());

    color.add_data4(LColor(1.0f, 1.0f, 1.0f, 1.0f) +
                    t * LColor(0.0f, 0.0f, 0.0f, -1.0f));
  }

  PT(GeomLinestrips) line = new GeomLinestrips(Geom::UH_static);
  line->add_next_vertices(num_points);
  line->close_primitive();

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(line);

  _viz_geom->add_geom(geom, get_other_viz_state());
  _bounds_viz_geom->add_geom(geom, get_other_bounds_viz_state());
}

/**
 * Tells the BamReader how to create objects of type CollisionRay.
 */
void CollisionRay::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void CollisionRay::
write_datagram(BamWriter *manager, Datagram &dg) {
  CollisionSolid::write_datagram(manager, dg);
  _origin.write_datagram(dg);
  _direction.write_datagram(dg);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type CollisionRay is encountered in the Bam file.  It should create the
 * CollisionRay and extract its information from the file.
 */
TypedWritable *CollisionRay::
make_from_bam(const FactoryParams &params) {
  CollisionRay *node = new CollisionRay();
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new CollisionRay.
 */
void CollisionRay::
fillin(DatagramIterator &scan, BamReader *manager) {
  CollisionSolid::fillin(scan, manager);
  _origin.read_datagram(scan);
  _direction.read_datagram(scan);
}
