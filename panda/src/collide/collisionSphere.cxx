// Filename: collisionSphere.cxx
// Created by:  drose (24Apr00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


#include "collisionDSSolid.h"
#include "collisionSphere.h"
#include "collisionLine.h"
#include "collisionRay.h"
#include "collisionSegment.h"
#include "collisionHandler.h"
#include "collisionEntry.h"
#include "config_collide.h"
#include "boundingSphere.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "nearly_zero.h"
#include "cmath.h"
#include "mathNumbers.h"
#include "geom.h"
#include "geomTristrips.h"
#include "geomVertexWriter.h"

PStatCollector CollisionSphere::_volume_pcollector(
  "Collision Volumes:CollisionSphere");
PStatCollector CollisionSphere::_test_pcollector(
  "Collision Tests:CollisionSphere");
TypeHandle CollisionSphere::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CollisionSolid *CollisionSphere::
make_copy() {
  return new CollisionSphere(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::test_intersection
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionSphere::
test_intersection(const CollisionEntry &entry) const {
  return entry.get_into()->test_intersection_from_sphere(entry);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::xform
//       Access: Public, Virtual
//  Description: Transforms the solid by the indicated matrix.
////////////////////////////////////////////////////////////////////
void CollisionSphere::
xform(const LMatrix4f &mat) {
  _center = _center * mat;

  // This is a little cheesy and fails miserably in the presence of a
  // non-uniform scale.
  LVector3f radius_v = LVector3f(_radius, 0.0f, 0.0f) * mat;
  _radius = length(radius_v);
  mark_viz_stale();
  mark_internal_bounds_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::get_collision_origin
//       Access: Public, Virtual
//  Description: Returns the point in space deemed to be the "origin"
//               of the solid for collision purposes.  The closest
//               intersection point to this origin point is considered
//               to be the most significant.
////////////////////////////////////////////////////////////////////
LPoint3f CollisionSphere::
get_collision_origin() const {
  return get_center();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::get_volume_pcollector
//       Access: Public, Virtual
//  Description: Returns a PStatCollector that is used to count the
//               number of bounding volume tests made against a solid
//               of this type in a given frame.
////////////////////////////////////////////////////////////////////
PStatCollector &CollisionSphere::
get_volume_pcollector() {
  return _volume_pcollector;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::get_test_pcollector
//       Access: Public, Virtual
//  Description: Returns a PStatCollector that is used to count the
//               number of intersection tests made against a solid
//               of this type in a given frame.
////////////////////////////////////////////////////////////////////
PStatCollector &CollisionSphere::
get_test_pcollector() {
  return _test_pcollector;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionSphere::
output(ostream &out) const {
  out << "sphere, c (" << get_center() << "), r " << get_radius();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::compute_internal_bounds
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(BoundingVolume) CollisionSphere::
compute_internal_bounds() const {
  return new BoundingSphere(_center, _radius);
}
#define USE_DS_SOLID_PLANES 1
////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::test_intersection_from_ds_solid
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionSphere::
test_intersection_from_ds_solid(const CollisionEntry &entry) const {
  const CollisionDSSolid *ds_solid;
  DCAST_INTO_R(ds_solid, entry.get_from(), 0);
  cerr<<"CollisionSphere::test_intersection_from_ds_solid\n";

  CPT(TransformState) wrt_space = entry.get_wrt_space();
  const LMatrix4f &wrt_mat = wrt_space->get_mat();

  LPoint3f into_center = get_center();
  float into_radius = get_radius();

  LPoint3f sa_center = ds_solid->get_center_a() * wrt_mat;
  float sa_radius = length(
    LVector3f(ds_solid->get_radius_a(), 0.0f, 0.0f) * wrt_mat);
  LVector3f sa_vec = sa_center - into_center;
  float sa_distance_squared = dot(sa_vec, sa_vec);
  float sa_and_into_radii_squared = (
    sa_radius + into_radius) * (sa_radius + into_radius);
  if (sa_distance_squared > sa_and_into_radii_squared) {
    // No intersection.
    return NULL;
  }

  LPoint3f sb_center = ds_solid->get_center_b() * wrt_mat;
  float sb_radius = length(
    LVector3f(ds_solid->get_radius_b(), 0.0f, 0.0f) * wrt_mat);
  LVector3f sb_vec = sb_center - into_center;
  float sb_distance_squared = dot(sb_vec, sb_vec);
  float sb_and_into_radii_squared = (
    sb_radius + into_radius) * (sb_radius + into_radius);
  if (sb_distance_squared > sb_and_into_radii_squared) {
    // No intersection.
    return NULL;
  }

  #if USE_DS_SOLID_PLANES
  CPT(TransformState) inv_wrt_space = entry.get_inv_wrt_space();
  const LMatrix4f &inv_wrt_mat = inv_wrt_space->get_mat();
  
  LPoint3f inv_into_center = get_center() * inv_wrt_mat;
  float inv_into_radius = length(
    LVector3f(get_radius(), 0.0f, 0.0f) * inv_wrt_mat);

  float pa_distance =
    ds_solid->dist_to_plane_a(inv_into_center) - inv_into_radius;
  if (pa_distance > 0.0f) {
    // No intersection.
    return NULL;
  }

  float pb_distance =
    ds_solid->dist_to_plane_b(inv_into_center) - inv_into_radius;
  if (pb_distance > 0.0f) {
    // No intersection.
    return NULL;
  }
  #endif

  LVector3f lens_center = ds_solid->get_collision_origin() * wrt_mat;
  float lens_radius = length(
    LVector3f(ds_solid->get_lens_radius(), 0.0f, 0.0f) * wrt_mat);
  LVector3f lens_vec = lens_center - into_center;
  //  float lens_distance_squared = dot(lens_vec, lens_vec);

  LVector3f surface_normal; // into
  //LPoint3f surface_point; // into
  LPoint3f interior_point; // from
  float sa_distance = sqrtf(sa_distance_squared) - sa_radius;
  float sb_distance = sqrtf(sb_distance_squared) - sb_radius;
  pa_distance = ds_solid->dist_to_plane_a(inv_into_center);
  pb_distance = ds_solid->dist_to_plane_b(inv_into_center);
  #if USE_DS_SOLID_PLANES
  cerr
    <<" sa_distance:"<<sa_distance
    <<" sb_distance:"<<sb_distance
    <<" pa_distance:"<<pa_distance
    <<" pb_distance:"<<pb_distance<<"\n";
  if ((sa_distance > pa_distance && sa_distance > pb_distance) ||
      (sb_distance > pa_distance && sb_distance > pb_distance)) {
  #else
  cerr
    <<" sa_distance:"<<sa_distance
    <<" sb_distance:"<<sb_distance<<"\n";
  #endif
    LVector3f *primary_vec;
    LPoint3f *primary_center;
    float primary_radius;
    LPoint3f *secondary_center;
    float secondary_radius;
    if (sa_distance > sb_distance) {
      // sphere_a is the furthest
      cerr<<"sphere_a is the furthest\n";
      primary_vec = &sa_vec;
      primary_center = &sa_center;
      primary_radius = sa_radius;
      secondary_center = &sb_center;
      secondary_radius = sb_radius;
    } else {
      // sphere_b is the furthest
      cerr<<"sphere_b is the furthest\n";
      primary_vec = &sb_vec;
      primary_center = &sb_center;
      primary_radius = sb_radius;
      secondary_center = &sa_center;
      secondary_radius = sa_radius;
    }

    float vec_length = primary_vec->length();
    if (IS_NEARLY_ZERO(vec_length)) {
      // The centers are coincident, use an arbitrary normal.
      surface_normal.set(1.0, 0.0, 0.0);
    } else {
      // Lens face
      surface_normal = *primary_vec / vec_length;
    }
    interior_point = *primary_center - surface_normal * primary_radius;

    float temp_length_squared =
      (interior_point - *secondary_center).length_squared();
    if (temp_length_squared > (secondary_radius * secondary_radius)) {
      cerr<<"foo\n";
      LVector3f a = (*primary_center - lens_center).normalize();
      LVector3f b =
        cross(cross(a, (into_center - lens_center).normalize()), a).normalize();
      interior_point = lens_center + b * lens_radius;
      surface_normal = (interior_point - into_center).normalize();
    }
  #if USE_DS_SOLID_PLANES
  } else {
    if (pa_distance > pb_distance) {
      // plane_a is the furthest
      cerr<<"plane_a is the furthest\n";
      surface_normal = -(ds_solid->get_plane_a().get_normal() * wrt_mat);
      float d = length(
        LVector3f(pa_distance, 0.0f, 0.0f) * wrt_mat);
      interior_point = into_center + surface_normal * d;
    } else {
      // plane_b is the furthest
      cerr<<"plane_b is the furthest\n";
      //surface_normal = -(ds_solid->get_plane_b().get_normal() * wrt_mat);
      surface_normal = -ds_solid->get_plane_b().get_normal();
      surface_normal = surface_normal * wrt_mat;
      float d = length(
        LVector3f(pb_distance, 0.0f, 0.0f) * wrt_mat);
      interior_point = into_center + surface_normal * d;
    }
  }
  #endif

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }

  PT(CollisionEntry) new_entry = new CollisionEntry(entry);
  new_entry->set_surface_normal(surface_normal);
  new_entry->set_surface_point(into_center + surface_normal * into_radius);
  new_entry->set_interior_point(interior_point);
  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::test_intersection_from_sphere
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionSphere::
test_intersection_from_sphere(const CollisionEntry &entry) const {
  const CollisionSphere *sphere;
  DCAST_INTO_R(sphere, entry.get_from(), 0);

  CPT(TransformState) wrt_space = entry.get_wrt_space();

  const LMatrix4f &wrt_mat = wrt_space->get_mat();

  LPoint3f from_b = sphere->get_center() * wrt_mat;

  LPoint3f into_center(get_center());
  float into_radius(get_radius());

  LVector3f from_radius_v =
    LVector3f(sphere->get_radius(), 0.0f, 0.0f) * wrt_mat;
  float from_radius = length(from_radius_v);

  LPoint3f into_intersection_point(from_b);
  double t1, t2;
  LPoint3f contact_point(into_intersection_point);
  float actual_t = 0.0f;

  LVector3f vec = from_b - into_center;
  float dist2 = dot(vec, vec);
  if (dist2 > (into_radius + from_radius) * (into_radius + from_radius)) {
    // No intersection with the current position.  Check the delta
    // from the previous frame.
    CPT(TransformState) wrt_prev_space = entry.get_wrt_prev_space();
    LPoint3f from_a = sphere->get_center() * wrt_prev_space->get_mat();

    if (!from_a.almost_equal(from_b)) {
      LVector3f from_direction = from_b - from_a;
      if (!intersects_line(t1, t2, from_a, from_direction, from_radius)) {
        // No intersection.
        return NULL;
      }
      
      if (t2 < 0.0 || t1 > 1.0) {
        // Both intersection points are before the start of the segment or
        // after the end of the segment.
        return NULL;
      }
      
      // doubles, not floats, to satisfy min and max templates.
      actual_t = min(1.0, max(0.0, t1));
      contact_point = from_a + actual_t * (from_b - from_a);
      
      if (t1 < 0.0) {
        // Point a is within the sphere.  The first intersection point is
        // point a itself.
        into_intersection_point = from_a;
      } else {
        // Point a is outside the sphere, and point b is either inside the
        // sphere or beyond it.  The first intersection point is at t1.
        into_intersection_point = from_a + t1 * from_direction;
      }
    } else {
      // No delta, therefore no intersection.
      return NULL;
    }
  }
  
  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3f from_center = sphere->get_center() * wrt_mat;

  LVector3f surface_normal;
  LVector3f v(into_intersection_point - into_center);
  float vec_length = v.length();
  if (IS_NEARLY_ZERO(vec_length)) {
    // If we don't have a collision normal (e.g. the centers are
    // exactly coincident), then make up an arbitrary normal--any one
    // is as good as any other.
    surface_normal.set(1.0, 0.0, 0.0);
  } else {
    surface_normal = v / vec_length;
  }

  LVector3f eff_normal = (has_effective_normal() && sphere->get_respect_effective_normal()) ? get_effective_normal() : surface_normal;

  LVector3f contact_normal;
  LVector3f v2 = contact_point - into_center;
  float v2_len = v2.length();
  if (IS_NEARLY_ZERO(v2_len)) {
    // If we don't have a collision normal (e.g. the centers are
    // exactly coincident), then make up an arbitrary normal--any one
    // is as good as any other.
    contact_normal.set(1.0, 0.0, 0.0);
  } else {
    contact_normal = v2 / v2_len;
  }

  new_entry->set_surface_normal(eff_normal);
  new_entry->set_surface_point(into_center + surface_normal * into_radius);
  new_entry->set_interior_point(from_center - surface_normal * from_radius);
  new_entry->set_contact_pos(contact_point);
  new_entry->set_contact_normal(contact_normal);
  new_entry->set_t(actual_t);

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::test_intersection_from_line
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionSphere::
test_intersection_from_line(const CollisionEntry &entry) const {
  const CollisionLine *line;
  DCAST_INTO_R(line, entry.get_from(), 0);

  const LMatrix4f &wrt_mat = entry.get_wrt_mat();

  LPoint3f from_origin = line->get_origin() * wrt_mat;
  LVector3f from_direction = line->get_direction() * wrt_mat;

  double t1, t2;
  if (!intersects_line(t1, t2, from_origin, from_direction, 0.0f)) {
    // No intersection.
    return NULL;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3f into_intersection_point = from_origin + t1 * from_direction;
  new_entry->set_surface_point(into_intersection_point);

  if (has_effective_normal() && line->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3f normal = into_intersection_point - get_center();
    normal.normalize();
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::test_intersection_from_box
//       Access: Public, Virtual
//  Description: Double dispatch point for box as a FROM object
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionSphere::
test_intersection_from_box(const CollisionEntry &entry) const {
  const CollisionBox *box;
  DCAST_INTO_R(box, entry.get_from(), 0);

  CPT(TransformState) wrt_space = entry.get_wrt_space();
  CPT(TransformState) wrt_prev_space = entry.get_wrt_prev_space();

  const LMatrix4f &wrt_mat = wrt_space->get_mat();

  CollisionBox local_b( *box );
  local_b.xform( wrt_mat );

  LPoint3f from_center = local_b.get_center();

  LPoint3f orig_center = get_center();
  LPoint3f to_center = orig_center;
  bool moved_from_center = false;
  float t = 1.0f;
  LPoint3f contact_point(from_center);
  float actual_t = 1.0f;

  float to_radius = get_radius();
  float to_radius_2 = to_radius * to_radius;

  int ip;
  float max_dist,dist;
  bool intersect;
  Planef plane;
  LVector3f normal;

  for( ip = 0, intersect=false; ip < 6 && !intersect; ip++ ){
    plane = local_b.get_plane( ip );
    if (local_b.get_plane_points(ip).size() < 3) {
      continue;
    }
    normal = (has_effective_normal() && box->get_respect_effective_normal()) ? get_effective_normal() : plane.get_normal();
    
    #ifndef NDEBUG
    /*
    if (!IS_THRESHOLD_EQUAL(normal.length_squared(), 1.0f, 0.001), NULL) {
      collide_cat.info()
      << "polygon being collided with " << entry.get_into_node_path()
      << " has normal " << normal << " of length " << normal.length()
      << "\n";
      normal.normalize();
    }
    */
    #endif

    // The nearest point within the plane to our center is the
    // intersection of the line (center, center - normal) with the plane.

    if (!plane.intersects_line(dist, to_center, -(plane.get_normal()))) {
      // No intersection with plane?  This means the plane's effective
      // normal was within the plane itself.  A useless polygon.
      continue;
    }

    if (dist > to_radius || dist < -to_radius) {
      // No intersection with the plane.
      continue;
    }

    LPoint2f p = local_b.to_2d(to_center - dist * plane.get_normal(), ip);
    float edge_dist = 0.0f;

    edge_dist = local_b.dist_to_polygon(p, local_b.get_plane_points(ip));

    if(edge_dist < 0) {
      intersect = true;
      continue;
    }

    if((edge_dist > 0) && 
      ((edge_dist * edge_dist + dist * dist) > to_radius_2)) {
      // No intersection; the circle is outside the polygon.
      continue;
    }

    // The sphere appears to intersect the polygon.  If the edge is less
    // than to_radius away, the sphere may be resting on an edge of
    // the polygon.  Determine how far the center of the sphere must
    // remain from the plane, based on its distance from the nearest
    // edge.

    max_dist = to_radius;
    if (edge_dist >= 0.0f) {
      float max_dist_2 = max(to_radius_2 - edge_dist * edge_dist, 0.0f);
      max_dist = csqrt(max_dist_2);
    }

    if (dist > max_dist) {
      // There's no intersection: the sphere is hanging off the edge.
      continue;
    }
    intersect = true;
  }
  if( !intersect )
    return NULL;

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }

  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  float into_depth = max_dist - dist;

  new_entry->set_surface_normal(normal);
  new_entry->set_surface_point(to_center - normal * dist);
  new_entry->set_interior_point(to_center - normal * (dist + into_depth));
  new_entry->set_contact_pos(contact_point);
  new_entry->set_contact_normal(plane.get_normal());
  new_entry->set_t(actual_t);

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::test_intersection_from_ray
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionSphere::
test_intersection_from_ray(const CollisionEntry &entry) const {
  const CollisionRay *ray;
  DCAST_INTO_R(ray, entry.get_from(), 0);

  const LMatrix4f &wrt_mat = entry.get_wrt_mat();

  LPoint3f from_origin = ray->get_origin() * wrt_mat;
  LVector3f from_direction = ray->get_direction() * wrt_mat;

  double t1, t2;
  if (!intersects_line(t1, t2, from_origin, from_direction, 0.0f)) {
    // No intersection.
    return NULL;
  }

  if (t2 < 0.0) {
    // Both intersection points are before the start of the ray.
    return NULL;
  }

  t1 = max(t1, 0.0);

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3f into_intersection_point = from_origin + t1 * from_direction;
  new_entry->set_surface_point(into_intersection_point);

  if (has_effective_normal() && ray->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3f normal = into_intersection_point - get_center();
    normal.normalize();
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::test_intersection_from_segment
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionSphere::
test_intersection_from_segment(const CollisionEntry &entry) const {
  const CollisionSegment *segment;
  DCAST_INTO_R(segment, entry.get_from(), 0);

  const LMatrix4f &wrt_mat = entry.get_wrt_mat();

  LPoint3f from_a = segment->get_point_a() * wrt_mat;
  LPoint3f from_b = segment->get_point_b() * wrt_mat;
  LVector3f from_direction = from_b - from_a;

  double t1, t2;
  if (!intersects_line(t1, t2, from_a, from_direction, 0.0f)) {
    // No intersection.
    return NULL;
  }

  if (t2 < 0.0 || t1 > 1.0) {
    // Both intersection points are before the start of the segment or
    // after the end of the segment.
    return NULL;
  }

  t1 = max(t1, 0.0);

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3f into_intersection_point = from_a + t1 * from_direction;
  new_entry->set_surface_point(into_intersection_point);

  if (has_effective_normal() && segment->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3f normal = into_intersection_point - get_center();
    normal.normalize();
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::test_intersection_from_parabola
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionSphere::
test_intersection_from_parabola(const CollisionEntry &entry) const {
  const CollisionParabola *parabola;
  DCAST_INTO_R(parabola, entry.get_from(), 0);

  const LMatrix4f &wrt_mat = entry.get_wrt_mat();

  // Convert the parabola into local coordinate space.
  Parabolaf local_p(parabola->get_parabola());
  local_p.xform(wrt_mat);

  double t;
  if (!intersects_parabola(t, local_p, parabola->get_t1(), parabola->get_t2(),
                           local_p.calc_point(parabola->get_t1()),
                           local_p.calc_point(parabola->get_t2()))) {
    // No intersection.
    return NULL;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }
  PT(CollisionEntry) new_entry = new CollisionEntry(entry);

  LPoint3f into_intersection_point = local_p.calc_point(t);
  new_entry->set_surface_point(into_intersection_point);

  if (has_effective_normal() && parabola->get_respect_effective_normal()) {
    new_entry->set_surface_normal(get_effective_normal());
  } else {
    LVector3f normal = into_intersection_point - get_center();
    normal.normalize();
    new_entry->set_surface_normal(normal);
  }

  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::fill_viz_geom
//       Access: Protected, Virtual
//  Description: Fills the _viz_geom GeomNode up with Geoms suitable
//               for rendering this solid.
////////////////////////////////////////////////////////////////////
void CollisionSphere::
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
    float longitude0 = (float)sl / (float)num_slices;
    float longitude1 = (float)(sl + 1) / (float)num_slices;
    vertex.add_data3f(compute_point(0.0, longitude0));
    for (int st = 1; st < num_stacks; ++st) {
      float latitude = (float)st / (float)num_stacks;
      vertex.add_data3f(compute_point(latitude, longitude0));
      vertex.add_data3f(compute_point(latitude, longitude1));
    }
    vertex.add_data3f(compute_point(1.0, longitude0));
    
    strip->add_next_vertices(num_stacks * 2);
    strip->close_primitive();
  }
  
  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(strip);
  
  _viz_geom->add_geom(geom, get_solid_viz_state());
  _bounds_viz_geom->add_geom(geom, get_solid_bounds_viz_state());
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::intersects_line
//       Access: Protected
//  Description: Determine the point(s) of intersection of a parametric
//               line with the sphere.  The line is infinite in both
//               directions, and passes through "from" and from+delta.
//               If the line does not intersect the sphere, the
//               function returns false, and t1 and t2 are undefined.
//               If it does intersect the sphere, it returns true, and
//               t1 and t2 are set to the points along the equation
//               from+t*delta that correspond to the two points of
//               intersection.
////////////////////////////////////////////////////////////////////
bool CollisionSphere::
intersects_line(double &t1, double &t2,
                const LPoint3f &from, const LVector3f &delta,
                float inflate_radius) const {
  // Solve the equation for the intersection of a line with a sphere
  // using the quadratic equation.

  // A line segment from f to f+d is defined as all P such that
  // P = f + td for 0 <= t <= 1.

  // A sphere with radius r about point c is defined as all P such
  // that r^2 = (P - c)^2.

  // Substituting P in the above we have:

  // r^2 = (f + td - c)^2 =
  // (f^2 + ftd - fc + ftd + t^2d^2 - tdc - fc - tdc + c^2) =
  // t^2(d^2) + t(fd + fd - dc - dc) + (f^2 - fc - fc + c^2) =
  // t^2(d^2) + t(2d(f - c)) + (f - c)^2

  // Thus, the equation is quadratic in t, and we have
  // at^2 + bt + c = 0

  // Where  a = d^2
  //        b = 2d(f - c)
  //        c = (f - c)^2 - r^2

  // Solving for t using the quadratic equation gives us the point of
  // intersection along the line segment.  Actually, there are two
  // solutions (since it is quadratic): one for the front of the
  // sphere, and one for the back.  In the case where the line is
  // tangent to the sphere, there is only one solution (and the
  // radical is zero).

  double A = dot(delta, delta);

  nassertr(A != 0.0, false);

  LVector3f fc = from - get_center();
  double B = 2.0f* dot(delta, fc);
  double fc_d2 = dot(fc, fc);
  double radius = get_radius() + inflate_radius;
  double C = fc_d2 - radius * radius;

  double radical = B*B - 4.0*A*C;

  if (IS_NEARLY_ZERO(radical)) {
    // Tangent.
    t1 = t2 = -B /(2.0*A);
    return true;

  } else if (radical < 0.0) {
    // No real roots: no intersection with the line.
    return false;
  }

  double reciprocal_2A = 1.0/(2.0*A);
  double sqrt_radical = sqrtf(radical);
  t1 = ( -B - sqrt_radical ) * reciprocal_2A;
  t2 = ( -B + sqrt_radical ) * reciprocal_2A;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::intersects_parabola
//       Access: Protected
//  Description: Determine a point of intersection of a parametric
//               parabola with the sphere.
//
//               We only consider the segment of the parabola between
//               t1 and t2, which has already been computed as
//               corresponding to points p1 and p2.  If there is an
//               intersection, t is set to the parametric point of
//               intersection, and true is returned; otherwise, false
//               is returned.
////////////////////////////////////////////////////////////////////
bool CollisionSphere::
intersects_parabola(double &t, const Parabolaf &parabola,
                    double t1, double t2,
                    const LPoint3f &p1, const LPoint3f &p2) const {
  if (t1 == t2) {
    // Special case: a single point.
    if ((p1 - _center).length_squared() > _radius * _radius) {
      // No intersection.
      return false;
    }
    t = t1;
    return true;
  }

  // To directly test for intersection between a parabola (quadratic)
  // and a sphere (also quadratic) requires solving a quartic
  // equation.  Doable, but hard, and I'm a programmer, not a
  // mathematician.  So I'll solve it the programmer's way instead, by
  // approximating the parabola with a series of line segments.
  // Hence, this function works by recursively subdividing the
  // parabola as necessary.

  // First, see if the line segment (p1 - p2) comes sufficiently close
  // to the parabola.  Do this by computing the parametric intervening
  // point and comparing its distance from the linear intervening
  // point.
  double tmid = (t1 + t2) * 0.5;
  if (tmid != t1 && tmid != t2) {
    LPoint3f pmid = parabola.calc_point(tmid);
    LPoint3f pmid2 = (p1 + p2) * 0.5f;
  
    if ((pmid - pmid2).length_squared() > 0.001f) {
      // Subdivide.
      if (intersects_parabola(t, parabola, t1, tmid, p1, pmid)) {
        return true;
      }
      return intersects_parabola(t, parabola, tmid, t2, pmid, p2);
    }
  }

  // The line segment is sufficiently close; compare the segment itself.
  double t1a, t2a;
  if (!intersects_line(t1a, t2a, p1, p2 - p1, 0.0f)) {
    return false;
  }

  if (t2a < 0.0 || t1a > 1.0) {
    return false;
  }

  t = max(t1a, 0.0);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::compute_point
//       Access: Protected
//  Description: Returns a point on the surface of the sphere.
//               latitude and longitude range from 0.0 to 1.0.  This
//               is used by fill_viz_geom() to create a visible
//               representation of the sphere.
////////////////////////////////////////////////////////////////////
Vertexf CollisionSphere::
compute_point(float latitude, float longitude) const {
  float s1, c1;
  csincos(latitude * MathNumbers::pi_f, &s1, &c1);

  float s2, c2;
  csincos(longitude * 2.0f * MathNumbers::pi_f, &s2, &c2);

  Vertexf p(s1 * c2, s1 * s2, c1);
  return p * get_radius() + get_center();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::register_with_read_factory
//       Access: Public, Static
//  Description: Factory method to generate a CollisionSphere object
////////////////////////////////////////////////////////////////////
void CollisionSphere::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_CollisionSphere);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void CollisionSphere::
write_datagram(BamWriter *manager, Datagram &me) {
  CollisionSolid::write_datagram(manager, me);
  _center.write_datagram(me);
  me.add_float32(_radius);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::make_CollisionSphere
//       Access: Protected
//  Description: Factory method to generate a CollisionSphere object
////////////////////////////////////////////////////////////////////
TypedWritable *CollisionSphere::
make_CollisionSphere(const FactoryParams &params) {
  CollisionSphere *me = new CollisionSphere;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionSphere::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void CollisionSphere::
fillin(DatagramIterator& scan, BamReader* manager) {
  CollisionSolid::fillin(scan, manager);
  _center.read_datagram(scan);
  _radius = scan.get_float32();
}
