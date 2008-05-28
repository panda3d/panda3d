// Filename: collisionDSSolid.cxx
// Created by:  Dave Schuyler (05Apr06)
// Based on collision tube by:  drose
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
#include "boundingSphere.h"
#include "config_collide.h"
#include "look_at.h"
#include "geom.h"
#include "geomNode.h"
#include "geometricBoundingVolume.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "cmath.h"
#include "transformState.h"
#include "geom.h"
#include "geomTristrips.h"
#include "geomTrifans.h"
#include "geomLinestrips.h"
#include "geomVertexWriter.h"

PStatCollector CollisionDSSolid::_volume_pcollector(
    "Collision Volumes:CollisionDSSolid");
PStatCollector CollisionDSSolid::_test_pcollector(
    "Collision Tests:CollisionDSSolid");
TypeHandle CollisionDSSolid::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CollisionSolid *CollisionDSSolid::
make_copy() {
  return new CollisionDSSolid(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::test_intersection
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionDSSolid::
test_intersection(const CollisionEntry &entry) const {
  return entry.get_into()->test_intersection_from_ds_solid(entry);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::xform
//       Access: Public, Virtual
//  Description: Transforms the solid by the indicated matrix.
////////////////////////////////////////////////////////////////////
void CollisionDSSolid::
xform(const LMatrix4f &mat) {
  _center_a = _center_a * mat;
  _center_b = _center_b * mat;
  _plane_a = _plane_a * mat;
  _plane_b = _plane_b * mat;

  // This is a little cheesy and fails miserably in the presence of a
  // non-uniform scale.
  LVector3f radius_v = LVector3f(_radius_a, 0.0f, 0.0f) * mat;
  _radius_a = length(radius_v);

  recalc_internals();
  CollisionSolid::xform(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::get_collision_origin
//       Access: Public, Virtual
//  Description: Returns the point in space deemed to be the "origin"
//               of the solid for collision purposes.  The closest
//               intersection point to this origin point is considered
//               to be the most significant.
////////////////////////////////////////////////////////////////////
LPoint3f CollisionDSSolid::
get_collision_origin() const {
  #if 1  
  LVector3f vec = _center_b - _center_a;
  float distance = vec.length();
  if (vec.normalize()) {
    // "Let" a few variables
    float dist_squared = distance * distance;
    float dist_doubled = 2.0f * distance;
    float radius_a_squared = _radius_a * _radius_a;
    float radius_b_squared = _radius_b * _radius_b;
    float n = dist_squared - radius_a_squared + radius_b_squared;
    
    // Get the lens center point on the intersection plane between
    // sphere A and sphere B
    LVector3f lens_center = _center_a + vec * (n / dist_doubled);
    return lens_center;
  } else {
    // Since both spheres are in the same place, just return
    // either of the centers
    return _center_a;
  }
  #else
  LVector3f vec = _center_b - _center_a;
  return _center_a + (vec * 0.5);
  #endif
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::get_volume_pcollector
//       Access: Public, Virtual
//  Description: Returns a PStatCollector that is used to count the
//               number of bounding volume tests made against a solid
//               of this type in a given frame.
////////////////////////////////////////////////////////////////////
PStatCollector &CollisionDSSolid::
get_volume_pcollector() {
  return _volume_pcollector;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::get_test_pcollector
//       Access: Public, Virtual
//  Description: Returns a PStatCollector that is used to count the
//               number of intersection tests made against a solid
//               of this type in a given frame.
////////////////////////////////////////////////////////////////////
PStatCollector &CollisionDSSolid::
get_test_pcollector() {
  return _test_pcollector;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void CollisionDSSolid::
output(ostream &out) const {
  out << "DSSolid, a (" 
      << _center_a << "), ra "
      << _radius_a << ", b (" 
      << _center_b << "), rb "
      << _radius_b << ", pa (" 
      << _plane_a << "), pb "
      << _plane_b << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::compute_internal_bounds
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(BoundingVolume) CollisionDSSolid::
compute_internal_bounds() const {
  PT(BoundingVolume) bound = CollisionSolid::compute_internal_bounds();

  if (bound->is_of_type(GeometricBoundingVolume::get_class_type())) {
    GeometricBoundingVolume *gbound;
    DCAST_INTO_R(gbound, bound, bound);

    LVector3f vec = _center_b - _center_a;
    float distance = vec.length();
    if (vec.normalize()) {
      // There is some distance between the centers, so the intersection
      // between them is some kind of lens.
      
      // "Let" a few variables
      float dist_squared = distance * distance;
      float dist_doubled = 2.0f * distance;
      float radius_a_squared = _radius_a * _radius_a;
      float radius_b_squared = _radius_b * _radius_b;
      float n = dist_squared - radius_a_squared + radius_b_squared;
      float m = 4.0f * dist_squared * radius_a_squared - (n * n);

      cerr<<"distance:"<<distance<<", _radius_a:"<<_radius_a
          <<", _radius_b"<<_radius_b<<", n:"<<n<<"\n";
      cerr<<"(1.0f / dist_doubled):"<<(1.0f / dist_doubled)
          <<", m:"<<m<<"\n";
      assert(m > 0.0f);
      
      // Get the lens center point on the intersection plane between
      // sphere A and sphere B
      LPoint3f lens_center = _center_a + vec * (n / dist_doubled);
      _lens_radius = (1.0f / dist_doubled) * sqrt(m);
      cerr<<"lens_center:"<<lens_center<<", lens_radius:"<<_lens_radius<<"\n";
      
      //TODO: account for cutting planes (which could make the sphere
      // smaller, which is an optimization).
      
      BoundingSphere sphere(lens_center, _lens_radius);
      gbound->extend_by(&sphere);
    } else {
      // Both endpoints are coincident; therefore, the bounding volume
      // is a sphere.
      //TODO: account for cutting planes (which could make the sphere
      // smaller, which is an optimization).
      BoundingSphere sphere(_center_a, _radius_a);
      gbound->extend_by(&sphere);
    }
  }

  return bound;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::test_intersection_from_sphere
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(CollisionEntry) CollisionDSSolid::
test_intersection_from_sphere(const CollisionEntry &entry) const {
  const CollisionSphere *sphere;
  DCAST_INTO_R(sphere, entry.get_from(), 0);
  cerr<<"CollisionDSSolid::test_intersection_from_ds_solid\n";

  CPT(TransformState) wrt_space = entry.get_wrt_space();

  const LMatrix4f &wrt_mat = wrt_space->get_mat();

  LPoint3f from_a = sphere->get_center() * wrt_mat;
  LPoint3f from_b = from_a;

  LVector3f from_direction = from_b - from_a;

  LPoint3f from_center = sphere->get_center() * wrt_mat;
  LVector3f from_radius_v =
    LVector3f(sphere->get_radius(), 0.0f, 0.0f) * wrt_mat;
  float from_radius = length(from_radius_v);

  LPoint3f sa_into_center = get_center_a();
  float sa_into_radius = get_radius_a();
  LVector3f sa_vec = from_center - sa_into_center;
  float sa_distance_squared = dot(sa_vec, sa_vec);
  float sa_and_from_radii_squared = (
      sa_into_radius + from_radius) * (sa_into_radius + from_radius);
  if (sa_distance_squared > sa_and_from_radii_squared) {
    // No intersection.
    return NULL;
  }

  LPoint3f sb_into_center = get_center_b();
  float sb_into_radius = get_radius_b();
  LVector3f sb_vec = from_center - sb_into_center;
  float sb_distance_squared = dot(sb_vec, sb_vec);
  float sb_and_from_radii_squared = (
      sb_into_radius + from_radius) * (sb_into_radius + from_radius);
  if (sb_distance_squared > sb_and_from_radii_squared) {
    // No intersection.
    return NULL;
  }

  float pa_distance = dist_to_plane_a(from_center);
  if (pa_distance > from_radius) {
    // No intersection.
    return NULL;
  }

  float pb_distance = dist_to_plane_b(from_center);
  if (pb_distance > from_radius) {
    // No intersection.
    return NULL;
  }

  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "intersection detected from " << entry.get_from_node_path()
      << " into " << entry.get_into_node_path() << "\n";
  }

  LVector3f surface_normal;
  LPoint3f surface_point;
  float spheres = sqrtf(sa_distance_squared) - sqrtf(sb_distance_squared);
  float planes = pa_distance - pb_distance;
  if (spheres > planes) {
    if (spheres > 0) {
      // sphere_a is the furthest
      cerr<<"sphere_a is the furthest"<<"\n";
      float vec_length = sa_vec.length();
      if (IS_NEARLY_ZERO(vec_length)) {
        // The centers are coincident, use an arbitrary normal.
        surface_normal.set(1.0, 0.0, 0.0);
      } else {
        surface_normal = sa_vec / vec_length;
      }
      surface_point = sa_into_center + surface_normal * sa_into_radius;
    } else {
      // sphere_b is the furthest
      cerr<<"sphere_b is the furthest"<<"\n";
      float vec_length = sb_vec.length();
      if (IS_NEARLY_ZERO(vec_length)) {
        // The centers are coincident, use an arbitrary normal.
        surface_normal.set(1.0, 0.0, 0.0);
      } else {
        surface_normal = sb_vec / vec_length;
      }
      surface_point = sb_into_center + surface_normal * sb_into_radius;
    }
  } else {
    if (planes > 0) {
      // plane_a is the furthest
      cerr<<"plane_a is the furthest"<<"\n";
      surface_normal = _plane_a.get_normal();
      surface_point = from_center - surface_normal * pa_distance;
    } else {
      // plane_b is the furthest
      cerr<<"plane_b is the furthest"<<"\n";
      surface_normal = _plane_b.get_normal();
      surface_point = from_center - surface_normal * pb_distance;
    }
  }

  PT(CollisionEntry) new_entry = new CollisionEntry(entry);
  new_entry->set_surface_normal(surface_normal);
  new_entry->set_surface_point(surface_point);
  new_entry->set_interior_point(from_center - surface_normal * from_radius);
  return new_entry;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::fill_viz_geom
//       Access: Protected, Virtual
//  Description: Fills the _viz_geom GeomNode up with Geoms suitable
//               for rendering this solid.
////////////////////////////////////////////////////////////////////
void CollisionDSSolid::
fill_viz_geom() {
  if (collide_cat.is_debug()) {
    collide_cat.debug()
      << "Recomputing viz for " << *this << "\n";
  }

  // Generate the vertices such that we draw a shape with one sphere
  // at (0, 0, 0), and another at (0, length, 0).  Then we'll rotate
  // and translate it into place with the appropriate look_at matrix.
  LVector3f direction = _center_b - _center_a;
  float half_length = direction.length() * 0.5f;
  
  // "Let" a few variables
  float distance = direction.length();
  float dist_squared = distance * distance;
  float dist_doubled = 2.0f * distance;
  float radius_a_squared = _radius_a * _radius_a;
  float radius_b_squared = _radius_b * _radius_b;
  float triangle_height = (
      dist_squared - radius_a_squared + radius_b_squared) / dist_doubled;
  
  // Get (half) the angle from _center_a to the lens edge on the
  // intersection plane between sphere A and sphere B
  float half_arc_angle_a = acos(triangle_height / _radius_a);
  float half_arc_angle_b = acos(triangle_height / _radius_b);

  PT(GeomVertexData) vdata = new GeomVertexData(
    "collision", GeomVertexFormat::get_v3(), Geom::UH_static);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  
  PT(GeomTristrips) strip = new GeomTristrips(Geom::UH_static);
  // Generate the first endcap.
  static const int num_slices = 8;
  static const int num_rings = 4;
  int ri, si;
  for (ri = 0; ri < num_rings; ++ri) {
    for (si = 0; si <= num_slices; ++si) {
      vertex.add_data3f(calc_sphere2_vertex(
          ri, si, num_rings, num_slices, -half_length, half_arc_angle_a));
      vertex.add_data3f(calc_sphere2_vertex(
          ri + 1, si, num_rings, num_slices, -half_length, half_arc_angle_a));
    }
    strip->add_next_vertices((num_slices + 1) * 2);
    strip->close_primitive();
  }
  
  // Add plane A
  calc_plane(_plane_a);
  
  // Add plane B
  calc_plane(_plane_b);
  
  // And the second endcap.
  for (ri = num_rings - 1; ri >= 0; --ri) {
    for (si = 0; si <= num_slices; ++si) {
      vertex.add_data3f(calc_sphere1_vertex(
          ri + 1, si, num_rings, num_slices, half_length, half_arc_angle_b));
      vertex.add_data3f(calc_sphere1_vertex(
          ri, si, num_rings, num_slices, half_length, half_arc_angle_b));
    }
    strip->add_next_vertices((num_slices + 1) * 2);
    strip->close_primitive();
  }
  
  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(strip);
  
  #if 0
  // Now transform the vertices to their actual location.
  LMatrix4f mat;
  look_at(mat, direction, LVector3f(0.0f, 0.0f, 1.0f), CS_zup_right);
  mat.set_row(3, _ceneter_a);
  geom->transform_vertices(mat);
  #endif
  
  _viz_geom->add_geom(geom, get_solid_viz_state());
  _bounds_viz_geom->add_geom(geom, get_solid_bounds_viz_state());
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::calc_plane
//       Access: Private
//  Description: Calculates a plane, for use in generating the
//               viz geometry.
////////////////////////////////////////////////////////////////////
void CollisionDSSolid::
calc_plane(const Planef &plane) {
  LPoint3f cp;
  LVector3f p1, p2, p3, p4;

  LVector3f normal = plane.get_normal();
  float D = plane[3];

  if (fabs(normal[0]) > fabs(normal[1]) &&
      fabs(normal[0]) > fabs(normal[2])) {
    // X has the largest coefficient.
    cp.set(-D / normal[0], 0.0f, 0.0f);
    p1 = LPoint3f(-(normal[1] + normal[2] + D)/normal[0], 1.0f, 1.0f) - cp;

  } else if (fabs(normal[1]) > fabs(normal[2])) {
    // Y has the largest coefficient.
    cp.set(0.0f, -D / normal[1], 0.0f);
    p1 = LPoint3f(1.0f, -(normal[0] + normal[2] + D)/normal[1], 1.0f) - cp;

  } else {
    // Z has the largest coefficient.
    cp.set(0.0f, 0.0f, -D / normal[2]);
    p1 = LPoint3f(1.0f, 1.0f, -(normal[0] + normal[1] + D)/normal[2]) - cp;
  }

  p1.normalize();
  p2 = cross(normal, p1);
  p3 = cross(normal, p2);
  p4 = cross(normal, p3);

  static const double plane_scale = 100.0;

  PT(GeomVertexData) vdata = new GeomVertexData
    ("collision", GeomVertexFormat::get_v3(),
     Geom::UH_static);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  
  vertex.add_data3f(cp + p1 * plane_scale);
  vertex.add_data3f(cp + p2 * plane_scale);
  vertex.add_data3f(cp + p3 * plane_scale);
  vertex.add_data3f(cp + p4 * plane_scale);
  
  PT(GeomTrifans) body = new GeomTrifans(Geom::UH_static);
  body->add_consecutive_vertices(0, 4);
  body->close_primitive();
  
  PT(GeomLinestrips) border = new GeomLinestrips(Geom::UH_static);
  border->add_consecutive_vertices(0, 4);
  border->add_vertex(0);
  border->close_primitive();
  
  PT(Geom) geom1 = new Geom(vdata);
  geom1->add_primitive(body);
  
  PT(Geom) geom2 = new Geom(vdata);
  geom2->add_primitive(border);
  
  _viz_geom->add_geom(geom1, get_solid_viz_state());
  _viz_geom->add_geom(geom2, get_wireframe_viz_state());
  
  _bounds_viz_geom->add_geom(geom1, get_solid_bounds_viz_state());
  _bounds_viz_geom->add_geom(geom2, get_wireframe_bounds_viz_state());
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::calc_sphere1_vertex
//       Access: Private
//  Description: Calculates a particular vertex on the surface of the
//               first endcap hemisphere, for use in generating the
//               viz geometry.
////////////////////////////////////////////////////////////////////
Vertexf CollisionDSSolid::
calc_sphere1_vertex(int ri, int si, int num_rings, int num_slices,
                    float length, float angle) {
  float r = (float)ri / (float)num_rings;
  float s = (float)si / (float)num_slices;

  // Find the point on the rim, based on the slice.
  float theta = s * 2.0f * MathNumbers::pi_f;
  float y_rim = ccos(theta);
  float z_rim = csin(theta);

  // Now pull that point in towards the pole, based on the ring.
  float phi = r * angle;
  float to_pole = csin(phi);

  float x = length -_radius_a * ccos(phi);
  float y = _radius_a * y_rim * to_pole;
  float z = _radius_a * z_rim * to_pole;

  return Vertexf(x, y, z);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::calc_sphere2_vertex
//       Access: Private
//  Description: Calculates a particular vertex on the surface of the
//               second endcap hemisphere, for use in generating the
//               viz geometry.
////////////////////////////////////////////////////////////////////
Vertexf CollisionDSSolid::
calc_sphere2_vertex(int ri, int si, int num_rings, int num_slices,
                    float length, float angle) {
  float r = (float)ri / (float)num_rings;
  float s = (float)si / (float)num_slices;

  // Find the point on the rim, based on the slice.
  float theta = s * 2.0f * MathNumbers::pi_f;
  float y_rim = ccos(theta);
  float z_rim = csin(theta);

  // Now pull that point in towards the pole, based on the ring.
  float phi = r * angle;
  float to_pole = csin(phi);

  float x = length + _radius_b * ccos(phi);
  float y = _radius_b * y_rim * to_pole;
  float z = _radius_b * z_rim * to_pole;

  return Vertexf(x, y, z);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::recalc_internals
//       Access: Private
//  Description: Should be called internally to recompute the matrix
//               and length when the properties of the tube have
//               changed.
////////////////////////////////////////////////////////////////////
void CollisionDSSolid::
recalc_internals() {
  mark_viz_stale();
  mark_internal_bounds_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               CollisionDSSolid.
////////////////////////////////////////////////////////////////////
void CollisionDSSolid::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void CollisionDSSolid::
write_datagram(BamWriter *manager, Datagram &dg) {
  CollisionSolid::write_datagram(manager, dg);
  _center_a.write_datagram(dg);
  dg.add_float32(_radius_a);
  _center_b.write_datagram(dg);
  dg.add_float32(_radius_b);
  _plane_a.write_datagram(dg);
  _plane_b.write_datagram(dg);
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type CollisionDSSolid is encountered
//               in the Bam file.  It should create the CollisionDSSolid
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *CollisionDSSolid::
make_from_bam(const FactoryParams &params) {
  CollisionDSSolid *node = new CollisionDSSolid();
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: CollisionDSSolid::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new CollisionDSSolid.
////////////////////////////////////////////////////////////////////
void CollisionDSSolid::
fillin(DatagramIterator &scan, BamReader *manager) {
  CollisionSolid::fillin(scan, manager);
  _center_a.read_datagram(scan);
  _radius_a = scan.get_float32();
  _center_b.read_datagram(scan);
  _radius_b = scan.get_float32();
  _plane_a.read_datagram(scan);
  _plane_b.read_datagram(scan);
  recalc_internals();
}
