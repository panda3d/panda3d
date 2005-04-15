// Filename: qpgeomLines.cxx
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

#include "qpgeomLines.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle qpGeomLines::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpGeomLines::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomLines::
qpGeomLines(qpGeomLines::UsageHint usage_hint) :
  qpGeomPrimitive(usage_hint)
{
}
 
////////////////////////////////////////////////////////////////////
//     Function: qpGeomLines::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomLines::
qpGeomLines(const qpGeomLines &copy) :
  qpGeomPrimitive(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomLines::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomLines::
~qpGeomLines() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomLines::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PT(qpGeomPrimitive) qpGeomLines::
make_copy() const {
  return new qpGeomLines(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomLines::get_primitive_type
//       Access: Public, Virtual
//  Description: Returns the fundamental rendering type of this
//               primitive: whether it is points, lines, or polygons.
//               This is used primarily to set up the appropriate
//               antialiasing settings when AntialiasAttrib::M_auto is
//               in effect.
////////////////////////////////////////////////////////////////////
qpGeomPrimitive::PrimitiveType qpGeomLines::
get_primitive_type() const {
  return PT_lines;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomLines::get_num_vertices_per_primitive
//       Access: Public, Virtual
//  Description: If the primitive type is a simple type in which all
//               primitives have the same number of vertices, like
//               lines, returns the number of vertices per
//               primitive.  If the primitive type is a more complex
//               type in which different primitives might have
//               different numbers of vertices, for instance a
//               line strip, returns 0.
////////////////////////////////////////////////////////////////////
int qpGeomLines::
get_num_vertices_per_primitive() const {
  return 2;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomLines::get_min_num_vertices_per_primitive
//       Access: Public, Virtual
//  Description: Returns the minimum number of vertices that must be
//               added before close_primitive() may legally be called.
////////////////////////////////////////////////////////////////////
int qpGeomLines::
get_min_num_vertices_per_primitive() const {
  return 2;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomLines::draw
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the GSG to draw the
//               primitive.
////////////////////////////////////////////////////////////////////
void qpGeomLines::
draw(GraphicsStateGuardianBase *gsg) const {
  gsg->draw_lines(this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomLines::rotate_impl
//       Access: Protected, Virtual
//  Description: The virtual implementation of do_rotate().
////////////////////////////////////////////////////////////////////
CPT(qpGeomVertexArrayData) qpGeomLines::
rotate_impl() const {
  // To rotate lines, we just move reverse the pairs of vertices.
  CPT(qpGeomVertexArrayData) vertices = get_vertices();

  PT(qpGeomVertexArrayData) new_vertices = 
    new qpGeomVertexArrayData(*vertices);
  qpGeomVertexReader from(vertices, 0);
  qpGeomVertexWriter to(new_vertices, 0);

  int num_vertices = vertices->get_num_rows();

  for (int begin = 0; begin < num_vertices; begin += 2) {
    from.set_row(begin + 1);
    to.set_data1i(from.get_data1i());
    from.set_row(begin);
    to.set_data1i(from.get_data1i());
  }
  
  nassertr(to.is_at_end(), NULL);
  return new_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomLines::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               qpGeom.
////////////////////////////////////////////////////////////////////
void qpGeomLines::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomLines::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type qpGeom is encountered
//               in the Bam file.  It should create the qpGeom
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *qpGeomLines::
make_from_bam(const FactoryParams &params) {
  qpGeomLines *object = new qpGeomLines(UH_unspecified);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
