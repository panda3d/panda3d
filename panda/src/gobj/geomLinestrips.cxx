// Filename: geomLinestrips.cxx
// Created by:  drose (22Mar05)
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

#include "geomLinestrips.h"
#include "geomLines.h"
#include "geomVertexRewriter.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle GeomLinestrips::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomLinestrips::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomLinestrips::
GeomLinestrips(GeomLinestrips::UsageHint usage_hint) :
  GeomPrimitive(usage_hint)
{
}
 
////////////////////////////////////////////////////////////////////
//     Function: GeomLinestrips::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomLinestrips::
GeomLinestrips(const GeomLinestrips &copy) :
  GeomPrimitive(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLinestrips::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
GeomLinestrips::
~GeomLinestrips() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLinestrips::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PT(GeomPrimitive) GeomLinestrips::
make_copy() const {
  return new GeomLinestrips(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLinestrips::get_primitive_type
//       Access: Public, Virtual
//  Description: Returns the fundamental rendering type of this
//               primitive: whether it is points, lines, or polygons.
//
//               This is used to set up the appropriate antialiasing
//               settings when AntialiasAttrib::M_auto is in effect;
//               it also implies the type of primitive that will be
//               produced when decompose() is called.
////////////////////////////////////////////////////////////////////
GeomPrimitive::PrimitiveType GeomLinestrips::
get_primitive_type() const {
  return PT_lines;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLinestrips::get_geom_rendering
//       Access: Published, Virtual
//  Description: Returns the set of GeomRendering bits that represent
//               the rendering properties required to properly render
//               this primitive.
////////////////////////////////////////////////////////////////////
int GeomLinestrips::
get_geom_rendering() const {
  if (is_indexed()) {
    return GR_line_strip | GR_indexed_other;
  } else {
    return GR_line_strip;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLinestrips::get_min_num_vertices_per_primitive
//       Access: Public, Virtual
//  Description: Returns the minimum number of vertices that must be
//               added before close_primitive() may legally be called.
////////////////////////////////////////////////////////////////////
int GeomLinestrips::
get_min_num_vertices_per_primitive() const {
  return 2;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLinestrips::draw
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the GSG to draw the
//               primitive.
////////////////////////////////////////////////////////////////////
void GeomLinestrips::
draw(GraphicsStateGuardianBase *gsg) const {
  gsg->draw_linestrips(this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLinestrips::decompose_impl
//       Access: Protected, Virtual
//  Description: Decomposes a complex primitive type into a simpler
//               primitive type, for instance line strips to
//               lines, and returns a pointer to the new primitive
//               definition.  If the decomposition cannot be
//               performed, this might return the original object.
//
//               This method is useful for application code that wants
//               to iterate through the set of lines on the
//               primitive without having to write handlers for each
//               possible kind of primitive type.
////////////////////////////////////////////////////////////////////
CPT(GeomPrimitive) GeomLinestrips::
decompose_impl() const {
  PT(GeomLines) lines = new GeomLines(get_usage_hint());
  lines->set_shade_model(get_shade_model());
  CPTA_int ends = get_ends();

  int vi = 0;
  int li = 0;
  while (li < (int)ends.size()) {
    int end = ends[li];
    nassertr(vi + 1 <= end, lines.p());
    int v0 = get_vertex(vi);
    ++vi;
    while (vi < end) {
      int v1 = get_vertex(vi);
      ++vi;
      lines->add_vertex(v0);
      lines->add_vertex(v1);
      v0 = v1;
      lines->close_primitive();
    }
    ++li;
  }
  nassertr(vi == get_num_vertices(), NULL);

  return lines.p();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLinestrips::rotate_impl
//       Access: Protected, Virtual
//  Description: The virtual implementation of do_rotate().
////////////////////////////////////////////////////////////////////
CPT(GeomVertexArrayData) GeomLinestrips::
rotate_impl() const {
  // To rotate a line strip, we just reverse the vertices.
  CPTA_int ends = get_ends();
  PT(GeomVertexArrayData) new_vertices = make_index_data();
  new_vertices->set_num_rows(get_num_vertices());

  if (is_indexed()) {
    CPT(GeomVertexArrayData) vertices = get_vertices();
    GeomVertexReader from(vertices, 0);
    GeomVertexWriter to(new_vertices, 0);
    
    int begin = 0;
    CPTA_int::const_iterator ei;
    for (ei = ends.begin(); ei != ends.end(); ++ei) {
      int end = (*ei);
      for (int vi = end - 1; vi >= begin; --vi) {
        from.set_row(vi);
        to.set_data1i(from.get_data1i());
      }
      begin = end;
    }
    
    nassertr(to.is_at_end(), NULL);

  } else {
    // Nonindexed case.
    int first_vertex = get_first_vertex();
    GeomVertexWriter to(new_vertices, 0);
    
    int begin = 0;
    CPTA_int::const_iterator ei;
    for (ei = ends.begin(); ei != ends.end(); ++ei) {
      int end = (*ei);
      for (int vi = end - 1; vi >= begin; --vi) {
        to.set_data1i(vi + first_vertex);
      }
      begin = end;
    }
    
    nassertr(to.is_at_end(), NULL);
  }
  return new_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLinestrips::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               Geom.
////////////////////////////////////////////////////////////////////
void GeomLinestrips::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLinestrips::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type Geom is encountered
//               in the Bam file.  It should create the Geom
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *GeomLinestrips::
make_from_bam(const FactoryParams &params) {
  GeomLinestrips *object = new GeomLinestrips(UH_unspecified);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
