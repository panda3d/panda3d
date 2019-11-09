/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file collisionInvSphere.cxx
 * @author drose
 * @date 2005-01-05
 */

#include "collisionInvSphere.h"

#include "collisionSphere.h"
#include "collisionCapsule.h"
#include "collisionLine.h"
#include "collisionParabola.h"
#include "collisionRay.h"
#include "collisionSegment.h"
#include "collisionHandler.h"
#include "collisionEntry.h"
#include "config_collide.h"
#include "omniBoundingVolume.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "nearly_zero.h"
#include "geom.h"
#include "geomTristrips.h"
#include "geomVertexWriter.h"

PStatCollector CollisionInvSphere::_volume_pcollector("Collision Volumes:CollisionInvSphere");
PStatCollector CollisionInvSphere::_test_pcollector("Collision Tests:CollisionInvSphere");
TypeHandle CollisionInvSphere::_type_handle;

/**
 *
 */
CollisionSolid *CollisionInvSphere::
make_copy() {
  return new CollisionInvSphere(*this);
}

/**
 *
 */
PT(CollisionEntry) CollisionInvSphere::
test_intersection(const CollisionEntry &) const {
  report_undefined_from_intersection(get_type());
  return nullptr;
}

/**
 * Returns a PStatCollector that is used to count the number of bounding
 * volume tests made against a solid of this type in a given frame.
 */
PStatCollector &CollisionInvSphere::
get_volume_pcollector() {
  return _volume_pcollector;
}

/**
 * Returns a PStatCollector that is used to count the number of intersection
 * tests made against a solid of this type in a given frame.
 */
PStatCollector &CollisionInvSphere::
get_test_pcollector() {
  return _test_pcollector;
}

/**
 *
 */
void CollisionInvSphere::
output(std::ostream &out) const {
  out << "invsphere, c (" << get_center() << "), r " << get_radius();
}

/**
 *
 */
PT(BoundingVolume) CollisionInvSphere::
compute_internal_bounds() const {
  // An inverse sphere always has an infinite bounding volume, since
  // everything outside the sphere is solid matter.
  return new OmniBoundingVolume();
}

/**
 *
 */
PT(CollisionEntry) CollisionInvSphere::
test_intersection_from_sphere(const CollisionEntry &entry) const {
  const CollisionSphere *sphere;
  DCAST_INTO_R(sphere, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_center = sphere->get_center() * wrt_mat;
  LVector3 from_radius_v =
    LVector3(sphere->get_radius(), 0.0f, 0.0f) * wrt_mat;
  PN_stdfloat from_radius = length(from_radius_v);

  LPoint3 into_center = get_center();
  PN_stdfloat into_radius = get_radius();

  LVector3 vec = from_center - into_center;
  PN_stdfloat dist2 = dot(vec, vec);
  if (dist2 < (into_radius - from_radius) * (into_radius - from_radius)) {
    // No intersection--the sphere is within the hollow.
    return nullptr;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LVector3 surface_normal;
  PN_stdfloat vec_length = vec.length();
  if (IS_NEARLY_ZERO(vec_length)) {
    // If we don't have a collision normal (e.g.  the centers are exactly
    // coincident), then make up an arbitrary normal--any one is as good as
    // any other.
    surface_normal.set(1.0, 0.0, 0.0);
  } else {
    surface_normal = vec / -vec_length;
  }

  LVector3 normal = (has_effective_normal() && sphere->get_respect_effective_normal()) ? get_effective_normal() : surface_normal;

  new_entry->set_surface_normal(normal);
  new_entry->set_surface_point(into_center - surface_normal * into_radius);
  new_entry->set_interior_point(from_center - surface_normal * from_radius);

  return new_entry;
}

/**
 *
 */
PT(CollisionEntry) CollisionInvSphere::
test_intersection_from_line(const CollisionEntry &entry) const {
  const CollisionLine *line;
  DCAST_INTO_R(line, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_origin = line->get_origin() * wrt_mat;
  LVector3 from_direction = line->get_direction() * wrt_mat;

  double t1, t2;
  if (!intersects_line(t1, t2, from_origin, from_direction, 0.0f)) {
    // The line is in the middle of space, and therefore intersects the
    // sphere.
    t1 = t2 = 0.0;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3 into_intersection_point = from_origin + t2 * from_direction;
  new_entry->set_surface_point(into_intersection_point);

  if (has_effective_normal() && line->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3 normal = into_intersection_point - get_center();
    normal.normalize();
    new_entry->set_surface_normal(-normal);
  }

  return new_entry;
}

/**
 *
 */
PT(CollisionEntry) CollisionInvSphere::
test_intersection_from_ray(const CollisionEntry &entry) const {
  const CollisionRay *ray;
  DCAST_INTO_R(ray, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_origin = ray->get_origin() * wrt_mat;
  LVector3 from_direction = ray->get_direction() * wrt_mat;

  double t1, t2;
  if (!intersects_line(t1, t2, from_origin, from_direction, 0.0f)) {
    // The ray is in the middle of space, and therefore intersects the sphere.
    t1 = t2 = 0.0;
  }

  t2 = std::max(t2, 0.0);

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3 into_intersection_point;
  into_intersection_point = from_origin + t2 * from_direction;
  new_entry->set_surface_point(into_intersection_point);

  if (has_effective_normal() && ray->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3 normal = into_intersection_point - get_center();
    normal.normalize();
    new_entry->set_surface_normal(-normal);
  }

  return new_entry;
}

/**
 *
 */
PT(CollisionEntry) CollisionInvSphere::
test_intersection_from_segment(const CollisionEntry &entry) const {
  const CollisionSegment *segment;
  DCAST_INTO_R(segment, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_a = segment->get_point_a() * wrt_mat;
  LPoint3 from_b = segment->get_point_b() * wrt_mat;
  LVector3 from_direction = from_b - from_a;

  double t1, t2;
  if (!intersects_line(t1, t2, from_a, from_direction, 0.0f)) {
    // The segment is in the middle of space, and therefore intersects the
    // sphere.
    t1 = t2 = 0.0;
  }

  double t;
  if (t2 <= 0.0) {
    // The segment is completely below the shell.
    t = 0.0;

  } else if (t1 >= 1.0) {
    // The segment is completely above the shell.
    t = 1.0;

  } else if (t2 <= 1.0) {
    // The bottom edge of the segment intersects the shell.
    t = std::min(t2, 1.0);

  } else if (t1 >= 0.0) {
    // The top edge of the segment intersects the shell.
    t = std::max(t1, 0.0);

  } else {
    // Neither edge of the segment intersects the shell.  It follows that both
    // intersection points are within the hollow center of the sphere;
    // therefore, there is no intersection.
    return nullptr;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3 into_intersection_point = from_a + t * from_direction;
  new_entry->set_surface_point(into_intersection_point);

  if (has_effective_normal() && segment->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3 normal = into_intersection_point - get_center();
    normal.normalize();
    new_entry->set_surface_normal(-normal);
  }

  return new_entry;
}


PT(CollisionEntry) CollisionInvSphere::
test_intersection_from_parabola(const CollisionEntry &entry) const {
  const CollisionParabola *parabola;
  DCAST_INTO_R(parabola, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  // Convert the parabola into local coordinate space
  LParabola local_p(parabola->get_parabola());
  local_p.xform(wrt_mat);

  double t;
  LPoint3 into_intersection_point;
  if (!intersects_parabola(t, local_p, parabola->get_t1(), parabola->get_t2(),
                           local_p.calc_point(parabola->get_t1()),
                           local_p.calc_point(parabola->get_t2()), into_intersection_point)) {
    // No intersection.
    return nullptr;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }

  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LVector3 normal = into_intersection_point - get_center();
  normal.normalize();
  if (has_effective_normal() && parabola->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    new_entry->set_surface_normal(-normal);
  }

  LPoint3 surface_point = normal * get_radius() + get_center();
  new_entry->set_surface_point(surface_point);

  return new_entry;
}


bool CollisionInvSphere::
intersects_parabola(double &t, const LParabola &parabola,
                    double t1, double t2,
                    const LPoint3 &p1, const LPoint3 &p2, LPoint3 &into_intersection_point) const {

  /* The method is pretty much identical to CollisionSphere::intersects_parabola:
   * recursively divide the parabola into "close enough" line segments, and test
   * their intersection with the sphere using tests similar to
   * CollisionInvSphere::test_intersection_from_segment. Returns the
   * point of intersection via pass-by-reference.
   */

  if (t1 == t2) {
    // Special case: a single point.
    if ((p1 - get_center()).length_squared() < get_radius() * get_radius()) {
      // No intersection.
      return false;
    }
    t = t1;
    return true;
  }

  double tmid = (t1 + t2) * 0.5;
  if (tmid != t1 && tmid != t2) {
    LPoint3 pmid = parabola.calc_point(tmid);
    LPoint3 pmid2 = (p1 + p2) * 0.5f;

    if ((pmid - pmid2).length_squared() > 0.001f) {
      if (intersects_parabola(t, parabola, t1, tmid, p1, pmid, into_intersection_point)) {
        return true;
      }
      return intersects_parabola(t, parabola, tmid, t2, pmid, p2, into_intersection_point);
    }
  }

  // Line segment is close enough to parabola. Test for intersections
  double t1a, t2a;
  if (!intersects_line(t1a, t2a, p1, p2 - p1, 0.0f)) {
    // segment is somewhere outside the sphere, so
    // we have an intersection, simply return the first point
    t = 0.0;
  }
  else {
    if (t2a <= 0.0) {
      // segment completely below sphere
      t = 0.0;
    }
    else if (t1a >= 1.0) {
      // segment acompletely bove sphere
      t = 1.0;
    }
    else if (t2a <= 1.0) {
      // bottom edge intersects sphere
      t = t2a;
    }
    else if (t1a >= 0.0) {
      // top edge intersects sphere
      t = t1a;
    }
    else {
      // completely inside sphere, no intersection
      return false;
    }
  }
  into_intersection_point = p1 + t * (p2 - p1);
  return true;
}

/**
 *
 */
PT(CollisionEntry) CollisionInvSphere::
test_intersection_from_capsule(const CollisionEntry &entry) const {
  const CollisionCapsule *capsule;
  DCAST_INTO_R(capsule, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 from_a = capsule->get_point_a() * wrt_mat;
  LPoint3 from_b = capsule->get_point_b() * wrt_mat;

  LVector3 from_radius_v =
    LVector3(capsule->get_radius(), 0.0f, 0.0f) * wrt_mat;
  PN_stdfloat from_radius = from_radius_v.length();

  LPoint3 center = get_center();
  PN_stdfloat radius = get_radius();

  // Check which one of the points lies furthest inside the sphere.
  PN_stdfloat dist_a = (from_a - center).length();
  PN_stdfloat dist_b = (from_b - center).length();
  if (dist_b > dist_a) {
    // Store the furthest point into from_a/dist_a.
    dist_a = dist_b;
    from_a = from_b;
  }

  // from_a now contains the furthest point.  Is it inside?
  if (dist_a < radius - from_radius) {
    return nullptr;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LVector3 normal = center - from_a;
  normal.normalize();
  new_entry->set_surface_point(get_center() - normal * radius);
  new_entry->set_interior_point(from_a - normal * from_radius);

  if (has_effective_normal() && capsule->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

/**
 * Double dispatch point for box as a FROM object
 */
PT(CollisionEntry) CollisionInvSphere::
test_intersection_from_box(const CollisionEntry &entry) const {
  const CollisionBox *box;
  DCAST_INTO_R(box, entry.get_from(), nullptr);

  const LMatrix4 &wrt_mat = entry.get_wrt_mat();

  LPoint3 center = get_center();
  PN_stdfloat radius_sq = get_radius();
  radius_sq *= radius_sq;

  // Just figure out which box point is furthest from the center.  If it
  // exceeds the radius, the furthest point wins.

  PN_stdfloat max_dist_sq = -1.0;
  LPoint3 deepest_vertex;

  for (int i = 0; i < 8; ++i) {
    LPoint3 point = wrt_mat.xform_point(box->get_point(i));

    PN_stdfloat dist_sq = (point - center).length_squared();
    if (dist_sq > max_dist_sq) {
      deepest_vertex = point;
      max_dist_sq = dist_sq;
    }
  }

  if (max_dist_sq < radius_sq) {
    // The point furthest away from the center is still inside the sphere.
    // Therefore, no collision.
    return nullptr;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }

  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  // The interior point is just the deepest cube vertex.
  new_entry->set_interior_point(deepest_vertex);

  // Now extrapolate the surface point and normal from that.
  LVector3 normal = center - deepest_vertex;
  normal.normalize();
  new_entry->set_surface_point(center - normal * get_radius());
  new_entry->set_surface_normal(
    (has_effective_normal() && box->get_respect_effective_normal())
    ? get_effective_normal() : normal);

  return new_entry;
}

/**
 * Fills the _viz_geom GeomNode up with Geoms suitable for rendering this
 * solid.
 */
void CollisionInvSphere::
fill_viz_geom() {
  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "Recomputing viz for " << *this << "\n";
  }

  static const int num_slices = 16;
  static const int num_stacks = 8;

  PT(GeomVertexData) vdata = new GeomVertexData
    ("collision", GeomVertexFormat::get_v3(),
     Geom::UH_static);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());

  PT(GeomTristrips) strip = new GeomTristrips(Geom::UH_static);
  for (int sl = 0; sl < num_slices; ++sl) {
    PN_stdfloat longitude0 = (PN_stdfloat)sl / (PN_stdfloat)num_slices;
    PN_stdfloat longitude1 = (PN_stdfloat)(sl + 1) / (PN_stdfloat)num_slices;
    vertex.add_data3(compute_point(0.0, longitude0));
    for (int st = 1; st < num_stacks; ++st) {
      PN_stdfloat latitude = (PN_stdfloat)st / (PN_stdfloat)num_stacks;
      vertex.add_data3(compute_point(latitude, longitude1));
      vertex.add_data3(compute_point(latitude, longitude0));
    }
    vertex.add_data3(compute_point(1.0, longitude0));

    strip->add_next_vertices(num_stacks * 2);
    strip->close_primitive();
  }

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(strip);

  _viz_geom->add_geom(geom, get_solid_viz_state());
}

/**
 * Factory method to generate a CollisionInvSphere object
 */
void CollisionInvSphere::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_CollisionInvSphere);
}

/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
void CollisionInvSphere::
write_datagram(BamWriter *manager, Datagram &me) {
  CollisionSphere::write_datagram(manager, me);
}

/**
 * Factory method to generate a CollisionInvSphere object
 */
TypedWritable *CollisionInvSphere::
make_CollisionInvSphere(const FactoryParams &params) {
  CollisionInvSphere *me = new CollisionInvSphere;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
void CollisionInvSphere::
fillin(DatagramIterator& scan, BamReader* manager) {
  CollisionSphere::fillin(scan, manager);
}
