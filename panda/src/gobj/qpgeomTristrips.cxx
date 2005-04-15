// Filename: qpgeomTristrips.cxx
// Created by:  drose (08Mar05)
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

#include "qpgeomTristrips.h"
#include "qpgeomTriangles.h"
#include "qpgeomVertexRewriter.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle qpGeomTristrips::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomTristrips::
qpGeomTristrips(qpGeomTristrips::UsageHint usage_hint) :
  qpGeomPrimitive(usage_hint)
{
}
 
////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomTristrips::
qpGeomTristrips(const qpGeomTristrips &copy) :
  qpGeomPrimitive(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomTristrips::
~qpGeomTristrips() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PT(qpGeomPrimitive) qpGeomTristrips::
make_copy() const {
  return new qpGeomTristrips(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::get_primitive_type
//       Access: Public, Virtual
//  Description: Returns the fundamental rendering type of this
//               primitive: whether it is points, lines, or polygons.
//               This is used primarily to set up the appropriate
//               antialiasing settings when AntialiasAttrib::M_auto is
//               in effect.
////////////////////////////////////////////////////////////////////
qpGeomPrimitive::PrimitiveType qpGeomTristrips::
get_primitive_type() const {
  return PT_polygons;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::get_geom_rendering
//       Access: Published, Virtual
//  Description: Returns the set of GeomRendering bits that represent
//               the rendering properties required to properly render
//               this primitive.
////////////////////////////////////////////////////////////////////
int qpGeomTristrips::
get_geom_rendering() const {
  return GR_triangle_strip;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::get_num_unused_vertices_per_primitive
//       Access: Public, Virtual
//  Description: Returns the number of vertices that are added between
//               primitives that aren't, strictly speaking, part of
//               the primitives themselves.  This is used, for
//               instance, to define degenerate triangles to connect
//               otherwise disconnected triangle strips.
////////////////////////////////////////////////////////////////////
int qpGeomTristrips::
get_num_unused_vertices_per_primitive() const {
  return 2;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::draw
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the GSG to draw the
//               primitive.
////////////////////////////////////////////////////////////////////
void qpGeomTristrips::
draw(GraphicsStateGuardianBase *gsg) const {
  gsg->draw_tristrips(this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::decompose_impl
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
CPT(qpGeomPrimitive) qpGeomTristrips::
decompose_impl() const {
  PT(qpGeomTriangles) triangles = new qpGeomTriangles(get_usage_hint());
  triangles->set_shade_model(get_shade_model());
  CPT(qpGeomVertexArrayData) vertices = get_vertices();
  qpGeomVertexReader index(vertices, 0);
  CPTA_int ends = get_ends();

  int num_vertices = vertices->get_num_vertices();

  // We need a slightly different algorithm for SM_flat_first_vertex
  // than for SM_flat_last_vertex, to preserve the key vertex in the
  // right place.  The remaining shade models can use either
  // algorithm.
  if (get_shade_model() == SM_flat_first_vertex) {
    // Preserve the first vertex of each component triangle as the
    // first vertex of each generated triangle.
    int vi = -2;
    int li = 0;
    while (li < (int)ends.size()) {
      // Skip unused vertices between tristrips.
      vi += 2;
      index.set_vertex(vi);
      int end = ends[li];
      nassertr(vi + 2 <= end, NULL);
      int v0 = index.get_data1i();
      ++vi;
      int v1 = index.get_data1i();
      ++vi;
      bool reversed = false;
      while (vi < end) {
        triangles->add_vertex(v0);
        int v2 = index.get_data1i();
        ++vi;
        if (reversed) {
          triangles->add_vertex(v2);
          triangles->add_vertex(v1);
          reversed = false;
        } else {
          triangles->add_vertex(v1);
          triangles->add_vertex(v2);
          reversed = true;
        }
        v0 = v1;
        v1 = v2;
        triangles->close_primitive();
      }
      ++li;
    }
    nassertr(vi == num_vertices && index.is_at_end(), NULL);

  } else {
    // Preserve the last vertex of each component triangle as the
    // last vertex of each generated triangle.
    int vi = -2;
    int li = 0;
    while (li < (int)ends.size()) {
      // Skip unused vertices between tristrips.
      vi += 2;
      index.set_vertex(vi);
      int end = ends[li];
      nassertr(vi + 2 <= end, NULL);
      int v0 = index.get_data1i();
      ++vi;
      int v1 = index.get_data1i();
      ++vi;
      bool reversed = false;
      while (vi < end) {
        if (reversed) {
          triangles->add_vertex(v1);
          triangles->add_vertex(v0);
          reversed = false;
        } else {
          triangles->add_vertex(v0);
          triangles->add_vertex(v1);
          reversed = true;
        }
        int v2 = index.get_data1i();
        ++vi;
        triangles->add_vertex(v2);
        v0 = v1;
        v1 = v2;
        triangles->close_primitive();
      }
      ++li;
    }
    nassertr(vi == num_vertices && index.is_at_end(), NULL);
  }

  return triangles.p();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::rotate_impl
//       Access: Protected, Virtual
//  Description: The virtual implementation of do_rotate().
////////////////////////////////////////////////////////////////////
CPT(qpGeomVertexArrayData) qpGeomTristrips::
rotate_impl() const {
  // To rotate a triangle strip with an even number of vertices, we
  // just reverse the vertices.  But if we have an odd number of
  // vertices, that doesn't work--in fact, nothing works (without also
  // changing the winding order), so we don't allow an odd number of
  // vertices in a flat-shaded tristrip.
  CPT(qpGeomVertexArrayData) vertices = get_vertices();
  CPTA_int ends = get_ends();
  PT(qpGeomVertexArrayData) new_vertices = 
    new qpGeomVertexArrayData(*vertices);
  qpGeomVertexReader from(vertices, 0);
  qpGeomVertexWriter to(new_vertices, 0);

  int begin = 0;
  int last_added = 0;
  CPTA_int::const_iterator ei;
  for (ei = ends.begin(); ei != ends.end(); ++ei) {
    int end = (*ei);
    int num_vertices = end - begin;

    if (begin != 0) {
      // Copy in the unused vertices between tristrips.
      to.set_data1i(last_added);
      from.set_vertex(end - 1);
      to.set_data1i(from.get_data1i());
      begin += 2;
    }

    // If this assertion is triggered, there was a triangle strip with
    // an odd number of vertices, which is not allowed.
    nassertr((num_vertices & 1) == 0, NULL);
    for (int vi = end - 1; vi >= begin; --vi) {
      from.set_vertex(vi);
      last_added = from.get_data1i();
      to.set_data1i(last_added);
    }

    begin = end;
  }

  nassertr(to.is_at_end(), NULL);
  return new_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::append_unused_vertices
//       Access: Protected, Virtual
//  Description: Called when a new primitive is begun (other than the
//               first primitive), this should add some degenerate
//               vertices between primitives, if the primitive type
//               requires that.  The second parameter is the first
//               vertex that begins the new primitive.
////////////////////////////////////////////////////////////////////
void qpGeomTristrips::
append_unused_vertices(qpGeomVertexArrayData *vertices, int vertex) {
  qpGeomVertexReader from(vertices, 0);
  from.set_vertex(from.get_num_vertices() - 1);
  int prev = from.get_data1i();

  qpGeomVertexWriter to(vertices, 0);
  to.set_vertex(to.get_num_vertices());

  to.add_data1i(prev);
  to.add_data1i(vertex);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               qpGeom.
////////////////////////////////////////////////////////////////////
void qpGeomTristrips::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type qpGeom is encountered
//               in the Bam file.  It should create the qpGeom
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *qpGeomTristrips::
make_from_bam(const FactoryParams &params) {
  qpGeomTristrips *object = new qpGeomTristrips(UH_unspecified);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
