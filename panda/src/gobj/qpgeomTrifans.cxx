// Filename: qpgeomTrifans.cxx
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

#include "qpgeomTrifans.h"
#include "qpgeomTriangles.h"
#include "qpgeomVertexRewriter.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle qpGeomTrifans::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTrifans::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomTrifans::
qpGeomTrifans(qpGeomTrifans::UsageHint usage_hint) :
  qpGeomPrimitive(usage_hint)
{
}
 
////////////////////////////////////////////////////////////////////
//     Function: qpGeomTrifans::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomTrifans::
qpGeomTrifans(const qpGeomTrifans &copy) :
  qpGeomPrimitive(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTrifans::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomTrifans::
~qpGeomTrifans() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTrifans::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PT(qpGeomPrimitive) qpGeomTrifans::
make_copy() const {
  return new qpGeomTrifans(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTrifans::get_primitive_type
//       Access: Public, Virtual
//  Description: Returns the fundamental rendering type of this
//               primitive: whether it is points, lines, or polygons.
//
//               This is used to set up the appropriate antialiasing
//               settings when AntialiasAttrib::M_auto is in effect;
//               it also implies the type of primitive that will be
//               produced when decompose() is called.
////////////////////////////////////////////////////////////////////
qpGeomPrimitive::PrimitiveType qpGeomTrifans::
get_primitive_type() const {
  return PT_polygons;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTrifans::get_geom_rendering
//       Access: Published, Virtual
//  Description: Returns the set of GeomRendering bits that represent
//               the rendering properties required to properly render
//               this primitive.
////////////////////////////////////////////////////////////////////
int qpGeomTrifans::
get_geom_rendering() const {
  return GR_triangle_fan;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTrifans::draw
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the GSG to draw the
//               primitive.
////////////////////////////////////////////////////////////////////
void qpGeomTrifans::
draw(GraphicsStateGuardianBase *gsg) const {
  gsg->draw_trifans(this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTrifans::decompose_impl
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
CPT(qpGeomPrimitive) qpGeomTrifans::
decompose_impl() const {
  PT(qpGeomTriangles) triangles = new qpGeomTriangles(get_usage_hint());
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
//     Function: qpGeomTrifans::rotate_impl
//       Access: Protected, Virtual
//  Description: The virtual implementation of do_rotate().
////////////////////////////////////////////////////////////////////
CPT(qpGeomVertexArrayData) qpGeomTrifans::
rotate_impl() const {
  // Actually, we can't rotate fans without chaging the winding order.
  // It's an error to define a flat shade model for a GeomTrifan.
  nassertr(false, NULL);
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTrifans::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               qpGeom.
////////////////////////////////////////////////////////////////////
void qpGeomTrifans::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTrifans::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type qpGeom is encountered
//               in the Bam file.  It should create the qpGeom
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *qpGeomTrifans::
make_from_bam(const FactoryParams &params) {
  qpGeomTrifans *object = new qpGeomTrifans(UH_unspecified);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
