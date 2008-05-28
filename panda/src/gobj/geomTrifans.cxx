// Filename: geomTrifans.cxx
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

#include "geomTrifans.h"
#include "geomTriangles.h"
#include "geomVertexRewriter.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "graphicsStateGuardianBase.h"

TypeHandle GeomTrifans::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomTrifans::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomTrifans::
GeomTrifans(GeomTrifans::UsageHint usage_hint) :
  GeomPrimitive(usage_hint)
{
}
 
////////////////////////////////////////////////////////////////////
//     Function: GeomTrifans::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomTrifans::
GeomTrifans(const GeomTrifans &copy) :
  GeomPrimitive(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTrifans::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
GeomTrifans::
~GeomTrifans() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTrifans::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PT(GeomPrimitive) GeomTrifans::
make_copy() const {
  return new GeomTrifans(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTrifans::get_primitive_type
//       Access: Public, Virtual
//  Description: Returns the fundamental rendering type of this
//               primitive: whether it is points, lines, or polygons.
//
//               This is used to set up the appropriate antialiasing
//               settings when AntialiasAttrib::M_auto is in effect;
//               it also implies the type of primitive that will be
//               produced when decompose() is called.
////////////////////////////////////////////////////////////////////
GeomPrimitive::PrimitiveType GeomTrifans::
get_primitive_type() const {
  return PT_polygons;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTrifans::get_geom_rendering
//       Access: Published, Virtual
//  Description: Returns the set of GeomRendering bits that represent
//               the rendering properties required to properly render
//               this primitive.
////////////////////////////////////////////////////////////////////
int GeomTrifans::
get_geom_rendering() const {
  if (is_indexed()) {
    return GR_triangle_fan | GR_indexed_other;
  } else {
    return GR_triangle_fan;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTrifans::draw
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the GSG to draw the
//               primitive.
////////////////////////////////////////////////////////////////////
bool GeomTrifans::
draw(GraphicsStateGuardianBase *gsg, const GeomPrimitivePipelineReader *reader,
     bool force) const {
  return gsg->draw_trifans(reader, force);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTrifans::decompose_impl
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
CPT(GeomPrimitive) GeomTrifans::
decompose_impl() const {
  PT(GeomTriangles) triangles = new GeomTriangles(get_usage_hint());
  triangles->set_shade_model(get_shade_model());
  CPTA_int ends = get_ends();

  int num_vertices = get_num_vertices();

  int vi = 0;
  int li = 0;
  while (li < (int)ends.size()) {
    int end = ends[li];
    nassertr(vi + 2 <= end, triangles.p());
    int v0 = get_vertex(vi);
    ++vi;
    int v1 = get_vertex(vi);
    ++vi;
    while (vi < end) {
      int v2 = get_vertex(vi);
      ++vi;
      triangles->add_vertex(v0);
      triangles->add_vertex(v1);
      triangles->add_vertex(v2);
      v1 = v2;
      triangles->close_primitive();
    }
    ++li;
  }

  nassertr(vi == num_vertices, NULL);

  return triangles.p();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTrifans::rotate_impl
//       Access: Protected, Virtual
//  Description: The virtual implementation of do_rotate().
////////////////////////////////////////////////////////////////////
CPT(GeomVertexArrayData) GeomTrifans::
rotate_impl() const {
  // Actually, we can't rotate fans without chaging the winding order.
  // It's an error to define a flat shade model for a GeomTrifan.
  nassertr(false, NULL);
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTrifans::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               Geom.
////////////////////////////////////////////////////////////////////
void GeomTrifans::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTrifans::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type Geom is encountered
//               in the Bam file.  It should create the Geom
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *GeomTrifans::
make_from_bam(const FactoryParams &params) {
  GeomTrifans *object = new GeomTrifans(UH_unspecified);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
