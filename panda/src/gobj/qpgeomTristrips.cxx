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
qpGeomTristrips(qpGeomUsageHint::UsageHint usage_hint) :
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
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PT(qpGeomPrimitive) qpGeomTristrips::
make_copy() const {
  return new qpGeomTristrips(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::get_primitive_type
//       Access: Published, Virtual
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
  CPTA_ushort vertices = get_vertices();
  CPTA_int ends = get_ends();

  // We need a slightly different algorithm for SM_flat_first_vertex
  // than for SM_flat_last_vertex, to preserve the key vertex in the
  // right place.  The remaining shade models can use either
  // algorithm.
  if (get_shade_model() == SM_flat_first_vertex) {
    // Preserve the first vertex of each component triangle as the
    // first vertex of each generated triangle.
    int vi = 0;
    int li = 0;
    while (li < (int)ends.size()) {
      int end = ends[li];
      nassertr(vi + 2 <= end, triangles.p());
      nassertr(vi < (int)vertices.size(), this);
      int v0 = vertices[vi];
      ++vi;
      nassertr(vi < (int)vertices.size(), this);
      int v1 = vertices[vi];
      ++vi;
      bool reversed = false;
      while (vi < end) {
        triangles->add_vertex(v0);
        if (reversed) {
          triangles->add_vertex(vertices[vi]);
          triangles->add_vertex(v1);
          reversed = false;
        } else {
          triangles->add_vertex(v1);
          triangles->add_vertex(vertices[vi]);
          reversed = true;
        }
        v0 = v1;
        nassertr(vi < (int)vertices.size(), this);
        v1 = vertices[vi];
        triangles->close_primitive();
        ++vi;
      }
      ++li;
    }
    nassertr(vi == (int)vertices.size(), triangles.p());

  } else {
    // Preserve the last vertex of each component triangle as the
    // last vertex of each generated triangle.
    int vi = 0;
    int li = 0;
    while (li < (int)ends.size()) {
      int end = ends[li];
      nassertr(vi + 2 <= end, triangles.p());
      nassertr(vi < (int)vertices.size(), this);
      int v0 = vertices[vi];
      ++vi;
      nassertr(vi < (int)vertices.size(), this);
      int v1 = vertices[vi];
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
        triangles->add_vertex(vertices[vi]);
        v0 = v1;
        nassertr(vi < (int)vertices.size(), this);
        v1 = vertices[vi];
        triangles->close_primitive();
        ++vi;
      }
      ++li;
    }
    nassertr(vi == (int)vertices.size(), triangles.p());
  }

  return triangles.p();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::rotate_impl
//       Access: Protected, Virtual
//  Description: The virtual implementation of do_rotate().
////////////////////////////////////////////////////////////////////
CPTA_ushort qpGeomTristrips::
rotate_impl() const {
  // To rotate a triangle strip with an even number of vertices, we
  // just reverse the vertices.  But if we have an odd number of
  // vertices, that doesn't work--in fact, nothing works (without also
  // changing the winding order), so we don't allow an odd number of
  // vertices in a flat-shaded tristrip.
  CPTA_ushort vertices = get_vertices();
  CPTA_int ends = get_ends();
  PTA_ushort new_vertices;
  new_vertices.reserve(vertices.size());

  bool any_odd = false;

  int begin = 0;
  CPTA_int::const_iterator ei;
  for (ei = ends.begin(); ei != ends.end(); ++ei) {
    int end = (*ei);
    int num_vertices = end - begin;

    if ((num_vertices & 1) == 0) {
      for (int vi = end - 1; vi >= begin; --vi) {
        new_vertices.push_back(vertices[vi]);
      }
    } else {
      any_odd = true;
    }

    begin = end;
  }
  nassertr(new_vertices.size() == vertices.size(), vertices);

  // If this assertion is triggered, there was a triangle strip with
  // an odd number of vertices and either SM_flat_first_vertex or
  // SM_flat_last_vertex specified--which is not allowed.
  nassertr(!any_odd, new_vertices);
  return new_vertices;
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
  qpGeomTristrips *object = new qpGeomTristrips(qpGeomUsageHint::UH_client);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
