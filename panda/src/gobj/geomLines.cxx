// Filename: geomLines.cxx
// Created by:  drose (22Mar05)
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

#include "geomLines.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "graphicsStateGuardianBase.h"
#include "geomVertexReader.h"
#include "geomVertexWriter.h"

TypeHandle GeomLines::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomLines::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomLines::
GeomLines(GeomLines::UsageHint usage_hint) :
  GeomPrimitive(usage_hint)
{
}
 
////////////////////////////////////////////////////////////////////
//     Function: GeomLines::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomLines::
GeomLines(const GeomLines &copy) :
  GeomPrimitive(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLines::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
GeomLines::
~GeomLines() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLines::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PT(GeomPrimitive) GeomLines::
make_copy() const {
  return new GeomLines(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLines::get_primitive_type
//       Access: Public, Virtual
//  Description: Returns the fundamental rendering type of this
//               primitive: whether it is points, lines, or polygons.
//
//               This is used to set up the appropriate antialiasing
//               settings when AntialiasAttrib::M_auto is in effect;
//               it also implies the type of primitive that will be
//               produced when decompose() is called.
////////////////////////////////////////////////////////////////////
GeomPrimitive::PrimitiveType GeomLines::
get_primitive_type() const {
  return PT_lines;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLines::get_num_vertices_per_primitive
//       Access: Public, Virtual
//  Description: If the primitive type is a simple type in which all
//               primitives have the same number of vertices, like
//               lines, returns the number of vertices per
//               primitive.  If the primitive type is a more complex
//               type in which different primitives might have
//               different numbers of vertices, for instance a
//               line strip, returns 0.
////////////////////////////////////////////////////////////////////
int GeomLines::
get_num_vertices_per_primitive() const {
  return 2;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLines::get_min_num_vertices_per_primitive
//       Access: Public, Virtual
//  Description: Returns the minimum number of vertices that must be
//               added before close_primitive() may legally be called.
////////////////////////////////////////////////////////////////////
int GeomLines::
get_min_num_vertices_per_primitive() const {
  return 2;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLines::draw
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the GSG to draw the
//               primitive.
////////////////////////////////////////////////////////////////////
bool GeomLines::
draw(GraphicsStateGuardianBase *gsg, const GeomPrimitivePipelineReader *reader,
     bool force) const {
  return gsg->draw_lines(reader, force);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLines::rotate_impl
//       Access: Protected, Virtual
//  Description: The virtual implementation of do_rotate().
////////////////////////////////////////////////////////////////////
CPT(GeomVertexArrayData) GeomLines::
rotate_impl() const {
  // To rotate lines, we just move reverse the pairs of vertices.
  int num_vertices = get_num_vertices();
    
  PT(GeomVertexArrayData) new_vertices = make_index_data();
  new_vertices->set_num_rows(num_vertices);

  if (is_indexed()) {
    CPT(GeomVertexArrayData) vertices = get_vertices();
    GeomVertexReader from(vertices, 0);
    GeomVertexWriter to(new_vertices, 0);

    for (int begin = 0; begin < num_vertices; begin += 2) {
      from.set_row_unsafe(begin + 1);
      to.set_data1i(from.get_data1i());
      from.set_row_unsafe(begin);
      to.set_data1i(from.get_data1i());
    }
  
    nassertr(to.is_at_end(), NULL);

  } else {
    // Nonindexed case.
    int first_vertex = get_first_vertex();
    GeomVertexWriter to(new_vertices, 0);

    for (int begin = 0; begin < num_vertices; begin += 2) {
      to.set_data1i(begin + 1 + first_vertex);
      to.set_data1i(begin + first_vertex);
    }
  
    nassertr(to.is_at_end(), NULL);
  }
    
  return new_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLines::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               Geom.
////////////////////////////////////////////////////////////////////
void GeomLines::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLines::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type Geom is encountered
//               in the Bam file.  It should create the Geom
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *GeomLines::
make_from_bam(const FactoryParams &params) {
  GeomLines *object = new GeomLines(UH_unspecified);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
