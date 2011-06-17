// Filename: geomTriangles.cxx
// Created by:  drose (06Mar05)
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

#include "geomTriangles.h"
#include "geomVertexRewriter.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "graphicsStateGuardianBase.h"

TypeHandle GeomTriangles::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomTriangles::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomTriangles::
GeomTriangles(GeomTriangles::UsageHint usage_hint) :
  GeomPrimitive(usage_hint)
{
}
 
////////////////////////////////////////////////////////////////////
//     Function: GeomTriangles::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomTriangles::
GeomTriangles(const GeomTriangles &copy) :
  GeomPrimitive(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTriangles::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
GeomTriangles::
~GeomTriangles() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTriangles::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PT(GeomPrimitive) GeomTriangles::
make_copy() const {
  return new GeomTriangles(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTriangles::get_primitive_type
//       Access: Public, Virtual
//  Description: Returns the fundamental rendering type of this
//               primitive: whether it is points, lines, or polygons.
//
//               This is used to set up the appropriate antialiasing
//               settings when AntialiasAttrib::M_auto is in effect;
//               it also implies the type of primitive that will be
//               produced when decompose() is called.
////////////////////////////////////////////////////////////////////
GeomPrimitive::PrimitiveType GeomTriangles::
get_primitive_type() const {
  return PT_polygons;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTriangles::get_num_vertices_per_primitive
//       Access: Public, Virtual
//  Description: If the primitive type is a simple type in which all
//               primitives have the same number of vertices, like
//               triangles, returns the number of vertices per
//               primitive.  If the primitive type is a more complex
//               type in which different primitives might have
//               different numbers of vertices, for instance a
//               triangle strip, returns 0.
////////////////////////////////////////////////////////////////////
int GeomTriangles::
get_num_vertices_per_primitive() const {
  return 3;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTriangles::draw
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the GSG to draw the
//               primitive.
////////////////////////////////////////////////////////////////////
bool GeomTriangles::
draw(GraphicsStateGuardianBase *gsg, const GeomPrimitivePipelineReader *reader,
     bool force) const {
  return gsg->draw_triangles(reader, force);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTriangles::doubleside_impl
//       Access: Protected, Virtual
//  Description: The virtual implementation of doubleside().
////////////////////////////////////////////////////////////////////
CPT(GeomPrimitive) GeomTriangles::
doubleside_impl() const {
  Thread *current_thread = Thread::get_current_thread();
  PT(GeomTriangles) reversed = new GeomTriangles(*this);

  GeomPrimitivePipelineReader from(this, current_thread);

  // This is like reverse(), except we don't clear the vertices first.
  // That way we double the vertices up.

  // First, rotate the original copy, if necessary, so the
  // flat-first/flat-last nature of the vertices is consistent
  // throughout the primitive.
  bool needs_rotate = false;
  switch (from.get_shade_model()) {
  case SM_flat_first_vertex:
  case SM_flat_last_vertex:
    reversed = (GeomTriangles *)DCAST(GeomTriangles, reversed->rotate());
    needs_rotate = true;

  default:
    break;
  }

  // Now append all the new vertices, in reverse order.
  for (int i = from.get_num_vertices() - 1; i >= 0; --i) {
    reversed->add_vertex(from.get_vertex(i));
  }

  // Finally, re-rotate the whole thing to get back to the original
  // shade model.
  if (needs_rotate) {
    reversed = (GeomTriangles *)DCAST(GeomTriangles, reversed->rotate());
  }

  return reversed.p();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTriangles::reverse_impl
//       Access: Protected, Virtual
//  Description: The virtual implementation of reverse().
////////////////////////////////////////////////////////////////////
CPT(GeomPrimitive) GeomTriangles::
reverse_impl() const {
  Thread *current_thread = Thread::get_current_thread();
  PT(GeomTriangles) reversed = new GeomTriangles(*this);

  GeomPrimitivePipelineReader from(this, current_thread);
  reversed->clear_vertices();

  for (int i = from.get_num_vertices() - 1; i >= 0; --i) {
    reversed->add_vertex(from.get_vertex(i));
  }

  switch (from.get_shade_model()) {
  case SM_flat_first_vertex:
    reversed->set_shade_model(SM_flat_last_vertex);
    reversed = (GeomTriangles *)DCAST(GeomTriangles, reversed->rotate());
    break;

  case SM_flat_last_vertex:
    reversed->set_shade_model(SM_flat_first_vertex);
    reversed = (GeomTriangles *)DCAST(GeomTriangles, reversed->rotate());
    break;

  default:
    break;
  }

  return reversed.p();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTriangles::rotate_impl
//       Access: Protected, Virtual
//  Description: The virtual implementation of rotate().
////////////////////////////////////////////////////////////////////
CPT(GeomVertexArrayData) GeomTriangles::
rotate_impl() const {
  // To rotate triangles, we just move one vertex from the front to
  // the back, or vice-versa; but we have to know what direction we're
  // going.
  ShadeModel shade_model = get_shade_model();
  int num_vertices = get_num_vertices();
    
  PT(GeomVertexArrayData) new_vertices = make_index_data();
  new_vertices->set_num_rows(num_vertices);

  if (is_indexed()) {
    CPT(GeomVertexArrayData) vertices = get_vertices();
    GeomVertexReader from(vertices, 0);
    GeomVertexWriter to(new_vertices, 0);
    
    switch (shade_model) {
    case SM_flat_first_vertex:
      // Move the first vertex to the end.
      {
        for (int begin = 0; begin < num_vertices; begin += 3) {
          from.set_row_unsafe(begin + 1);
          to.set_data1i(from.get_data1i());
          to.set_data1i(from.get_data1i());
          from.set_row_unsafe(begin);
          to.set_data1i(from.get_data1i());
        }
      }
      break;
      
    case SM_flat_last_vertex:
      // Move the last vertex to the front.
      {
        for (int begin = 0; begin < num_vertices; begin += 3) {
          from.set_row_unsafe(begin + 2);
          to.set_data1i(from.get_data1i());
          from.set_row_unsafe(begin);
          to.set_data1i(from.get_data1i());
          to.set_data1i(from.get_data1i());
        }
      }
      break;
      
    default:
      // This shouldn't get called with any other shade model.
      nassertr(false, vertices);
    }
    
    nassertr(to.is_at_end(), NULL);

  } else {
    // Nonindexed case.
    int first_vertex = get_first_vertex();
    GeomVertexWriter to(new_vertices, 0);
    
    switch (shade_model) {
    case SM_flat_first_vertex:
      // Move the first vertex to the end.
      {
        for (int begin = 0; begin < num_vertices; begin += 3) {
          to.set_data1i(begin + 1 + first_vertex);
          to.set_data1i(begin + 2 + first_vertex);
          to.set_data1i(begin + first_vertex);
        }
      }
      break;
      
    case SM_flat_last_vertex:
      // Move the last vertex to the front.
      {
        for (int begin = 0; begin < num_vertices; begin += 3) {
          to.set_data1i(begin + 2 + first_vertex);
          to.set_data1i(begin + first_vertex);
          to.set_data1i(begin + 1 + first_vertex);
        }
      }
      break;
      
    default:
      // This shouldn't get called with any other shade model.
      nassertr(false, NULL);
    }
    
    nassertr(to.is_at_end(), NULL);
  }

  return new_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTriangles::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               Geom.
////////////////////////////////////////////////////////////////////
void GeomTriangles::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTriangles::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type Geom is encountered
//               in the Bam file.  It should create the Geom
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *GeomTriangles::
make_from_bam(const FactoryParams &params) {
  GeomTriangles *object = new GeomTriangles(UH_unspecified);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
