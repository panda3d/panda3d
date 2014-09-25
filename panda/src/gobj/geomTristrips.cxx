// Filename: geomTristrips.cxx
// Created by:  drose (08Mar05)
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

#include "geomTristrips.h"
#include "geomTriangles.h"
#include "geomVertexRewriter.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "graphicsStateGuardianBase.h"

TypeHandle GeomTristrips::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrips::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomTristrips::
GeomTristrips(GeomTristrips::UsageHint usage_hint) :
  GeomPrimitive(usage_hint)
{
}
 
////////////////////////////////////////////////////////////////////
//     Function: GeomTristrips::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomTristrips::
GeomTristrips(const GeomTristrips &copy) :
  GeomPrimitive(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrips::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
GeomTristrips::
~GeomTristrips() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrips::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PT(GeomPrimitive) GeomTristrips::
make_copy() const {
  return new GeomTristrips(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrips::get_primitive_type
//       Access: Public, Virtual
//  Description: Returns the fundamental rendering type of this
//               primitive: whether it is points, lines, or polygons.
//
//               This is used to set up the appropriate antialiasing
//               settings when AntialiasAttrib::M_auto is in effect;
//               it also implies the type of primitive that will be
//               produced when decompose() is called.
////////////////////////////////////////////////////////////////////
GeomPrimitive::PrimitiveType GeomTristrips::
get_primitive_type() const {
  return PT_polygons;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrips::get_geom_rendering
//       Access: Published, Virtual
//  Description: Returns the set of GeomRendering bits that represent
//               the rendering properties required to properly render
//               this primitive.
////////////////////////////////////////////////////////////////////
int GeomTristrips::
get_geom_rendering() const {
  if (is_indexed()) {
    return GR_triangle_strip | GR_indexed_other;
  } else {
    return GR_triangle_strip;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrips::get_min_num_vertices_per_primitive
//       Access: Public, Virtual
//  Description: Returns the minimum number of vertices that must be
//               added before close_primitive() may legally be called.
////////////////////////////////////////////////////////////////////
int GeomTristrips::
get_min_num_vertices_per_primitive() const {
  return 3;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrips::get_num_unused_vertices_per_primitive
//       Access: Public, Virtual
//  Description: Returns the number of vertices that are added between
//               primitives that aren't, strictly speaking, part of
//               the primitives themselves.  This is used, for
//               instance, to define degenerate triangles to connect
//               otherwise disconnected triangle strips.
////////////////////////////////////////////////////////////////////
int GeomTristrips::
get_num_unused_vertices_per_primitive() const {
  return 2;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrips::draw
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the GSG to draw the
//               primitive.
////////////////////////////////////////////////////////////////////
bool GeomTristrips::
draw(GraphicsStateGuardianBase *gsg, const GeomPrimitivePipelineReader *reader,
     bool force) const {
  return gsg->draw_tristrips(reader, force);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrips::decompose_impl
//       Access: Protected, Virtual
//  Description: Decomposes a complex primitive type into a simpler
//               primitive type, for instance triangle strips to
//               triangles, and returns a pointer to the new primitive
//               definition.  If the decomposition cannot be
//               performed, this might return the original object.
//
//               This method is useful for application code that wants
//               to iterate through the set of triangles on the
//               primitive without having to write handlers for each
//               possible kind of primitive type.
////////////////////////////////////////////////////////////////////
CPT(GeomPrimitive) GeomTristrips::
decompose_impl() const {
  PT(GeomTriangles) triangles = new GeomTriangles(get_usage_hint());
  triangles->set_shade_model(get_shade_model());
  CPTA_int ends = get_ends();

  int num_vertices = get_num_vertices();
  int num_unused = get_num_unused_vertices_per_primitive();

  // We need a slightly different algorithm for SM_flat_first_vertex
  // than for SM_flat_last_vertex, to preserve the key vertex in the
  // right place.  The remaining shade models can use either
  // algorithm.
  if (get_shade_model() == SM_flat_first_vertex) {
    // Preserve the first vertex of each component triangle as the
    // first vertex of each generated triangle.
    int vi = -num_unused;
    int li = 0;
    while (li < (int)ends.size()) {
      // Skip unused vertices between tristrips.
      vi += num_unused;
      int end = ends[li];
      nassertr(vi + 2 <= end, NULL);
      int v0 = get_vertex(vi);
      ++vi;
      int v1 = get_vertex(vi);
      ++vi;
      bool reversed = false;
      while (vi < end) {
        int v2 = get_vertex(vi);
        ++vi;
        if (reversed) {
          if (v0 != v1 && v0 != v2 && v1 != v2) {
            triangles->add_vertex(v0);
            triangles->add_vertex(v2);
            triangles->add_vertex(v1);
            triangles->close_primitive();
          }
          reversed = false;
        } else {
          if (v0 != v1 && v0 != v2 && v1 != v2) {
            triangles->add_vertex(v0);
            triangles->add_vertex(v1);
            triangles->add_vertex(v2);
            triangles->close_primitive();
          }
          reversed = true;
        }
        v0 = v1;
        v1 = v2;
      }
      ++li;
    }
    nassertr(vi == num_vertices, NULL);

  } else {
    // Preserve the last vertex of each component triangle as the
    // last vertex of each generated triangle.
    int vi = -num_unused;
    int li = 0;
    while (li < (int)ends.size()) {
      // Skip unused vertices between tristrips.
      vi += num_unused;
      int end = ends[li];
      nassertr(vi + 2 <= end, NULL);
      int v0 = get_vertex(vi);
      ++vi;
      int v1 = get_vertex(vi);
      ++vi;
      bool reversed = false;
      while (vi < end) {
        int v2 = get_vertex(vi);
        if (reversed) {
          if (v0 != v1 && v0 != v2 && v1 != v2) {
            triangles->add_vertex(v1);
            triangles->add_vertex(v0);
            triangles->add_vertex(v2);
            triangles->close_primitive();
          }
          reversed = false;
        } else {
          if (v0 != v1 && v0 != v2 && v1 != v2) {
            triangles->add_vertex(v0);
            triangles->add_vertex(v1);
            triangles->add_vertex(v2);
            triangles->close_primitive();
          }
          reversed = true;
        }
        ++vi;
        v0 = v1;
        v1 = v2;
      }
      ++li;
    }
    nassertr(vi == num_vertices, NULL);
  }

  return triangles.p();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrips::doubleside_impl
//       Access: Protected, Virtual
//  Description: The virtual implementation of doubleside().
////////////////////////////////////////////////////////////////////
CPT(GeomPrimitive) GeomTristrips::
doubleside_impl() const {
  // TODO: implement this properly as triangle strips, without
  // requiring a decompose operation first.
  return decompose_impl()->doubleside();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrips::reverse_impl
//       Access: Protected, Virtual
//  Description: The virtual implementation of reverse().
////////////////////////////////////////////////////////////////////
CPT(GeomPrimitive) GeomTristrips::
reverse_impl() const {
  // TODO: implement this properly as triangle strips, without
  // requiring a decompose operation first.
  return decompose_impl()->reverse();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrips::rotate_impl
//       Access: Protected, Virtual
//  Description: The virtual implementation of do_rotate().
////////////////////////////////////////////////////////////////////
CPT(GeomVertexArrayData) GeomTristrips::
rotate_impl() const {
  // To rotate a triangle strip with an even number of vertices, we
  // just reverse the vertices.  But if we have an odd number of
  // vertices, that doesn't work--in fact, nothing works (without also
  // changing the winding order), so we don't allow an odd number of
  // vertices in a flat-shaded tristrip.
  CPTA_int ends = get_ends();

  PT(GeomVertexArrayData) new_vertices = make_index_data();
  new_vertices->set_num_rows(get_num_vertices());

  if (is_indexed()) {
    CPT(GeomVertexArrayData) vertices = get_vertices();
    GeomVertexReader from(vertices, 0);
    GeomVertexWriter to(new_vertices, 0);
    
    int begin = 0;
    int last_added = 0;
    CPTA_int::const_iterator ei;
    for (ei = ends.begin(); ei != ends.end(); ++ei) {
      int end = (*ei);
      int num_vertices = end - begin;
      
      if (begin != 0) {
        // Copy in the unused vertices between tristrips.
        to.set_data1i(last_added);
        from.set_row_unsafe(end - 1);
        to.set_data1i(from.get_data1i());
        begin += 2;
      }
      
      // If this assertion is triggered, there was a triangle strip with
      // an odd number of vertices, which is not allowed.
      nassertr((num_vertices & 1) == 0, NULL);
      for (int vi = end - 1; vi >= begin; --vi) {
        from.set_row_unsafe(vi);
        last_added = from.get_data1i();
        to.set_data1i(last_added);
      }
      
      begin = end;
    }

    nassertr(to.is_at_end(), NULL);

  } else {
    // Nonindexed case.
    int first_vertex = get_first_vertex();
    GeomVertexWriter to(new_vertices, 0);
    
    int begin = 0;
    int last_added = 0;
    CPTA_int::const_iterator ei;
    for (ei = ends.begin(); ei != ends.end(); ++ei) {
      int end = (*ei);
      int num_vertices = end - begin;
      
      if (begin != 0) {
        // Copy in the unused vertices between tristrips.
        to.set_data1i(last_added);
        to.set_data1i(end - 1 + first_vertex);
        begin += 2;
      }
      
      // If this assertion is triggered, there was a triangle strip with
      // an odd number of vertices, which is not allowed.
      nassertr((num_vertices & 1) == 0, NULL);
      for (int vi = end - 1; vi >= begin; --vi) {
        last_added = vi + first_vertex;
        to.set_data1i(last_added);
      }
      
      begin = end;
    }

    nassertr(to.is_at_end(), NULL);
  }
  return new_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrips::requires_unused_vertices
//       Access: Protected, Virtual
//  Description: Should be redefined to return true in any primitive
//               that implements append_unused_vertices().
////////////////////////////////////////////////////////////////////
bool GeomTristrips::
requires_unused_vertices() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrips::append_unused_vertices
//       Access: Protected, Virtual
//  Description: Called when a new primitive is begun (other than the
//               first primitive), this should add some degenerate
//               vertices between primitives, if the primitive type
//               requires that.  The second parameter is the first
//               vertex that begins the new primitive.
////////////////////////////////////////////////////////////////////
void GeomTristrips::
append_unused_vertices(GeomVertexArrayData *vertices, int vertex) {
  GeomVertexReader from(vertices, 0);
  from.set_row_unsafe(vertices->get_num_rows() - 1);
  int prev = from.get_data1i();

  GeomVertexWriter to(vertices, 0);
  to.set_row_unsafe(vertices->get_num_rows());

  to.add_data1i(prev);
  to.add_data1i(vertex);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrips::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               Geom.
////////////////////////////////////////////////////////////////////
void GeomTristrips::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTristrips::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type Geom is encountered
//               in the Bam file.  It should create the Geom
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *GeomTristrips::
make_from_bam(const FactoryParams &params) {
  GeomTristrips *object = new GeomTristrips(UH_unspecified);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
