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
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle qpGeomTrifans::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTrifans::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomTrifans::
qpGeomTrifans(qpGeomUsageHint::UsageHint usage_hint) :
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
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PT(qpGeomPrimitive) qpGeomTrifans::
make_copy() const {
  return new qpGeomTrifans(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTrifans::get_primitive_type
//       Access: Published, Virtual
//  Description: Returns the fundamental rendering type of this
//               primitive: whether it is points, lines, or polygons.
//               This is used primarily to set up the appropriate
//               antialiasing settings when AntialiasAttrib::M_auto is
//               in effect.
////////////////////////////////////////////////////////////////////
qpGeomPrimitive::PrimitiveType qpGeomTrifans::
get_primitive_type() const {
  return PT_polygons;
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
  CPTA_ushort vertices = get_vertices();
  CPTA_int ends = get_ends();

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
    while (vi < end) {
      triangles->add_vertex(v0);
      triangles->add_vertex(v1);
      triangles->add_vertex(vertices[vi]);
      nassertr(vi < (int)vertices.size(), this);
      v1 = vertices[vi];
      triangles->close_primitive();
      ++vi;
    }
    ++li;
  }

  nassertr(vi == (int)vertices.size(), triangles.p());

  return triangles.p();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTrifans::rotate_impl
//       Access: Protected, Virtual
//  Description: The virtual implementation of do_rotate().
////////////////////////////////////////////////////////////////////
CPTA_ushort qpGeomTrifans::
rotate_impl() const {
  // Actually, we can't rotate fans without chaging the winding order.
  // It's an error to define a flat shade model for a GeomTrifan.
  nassertr(false, get_vertices());
  return get_vertices();
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
  qpGeomTrifans *object = new qpGeomTrifans(qpGeomUsageHint::UH_client);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
