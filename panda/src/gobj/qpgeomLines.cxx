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
qpGeomLines(qpGeomUsageHint::UsageHint usage_hint) :
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
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PT(qpGeomPrimitive) qpGeomLines::
make_copy() const {
  return new qpGeomLines(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomLines::get_primitive_type
//       Access: Published, Virtual
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
//       Access: Published, Virtual
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
CPTA_ushort qpGeomLines::
rotate_impl() const {
  // To rotate lines, we just move reverse the pairs of vertices.
  CPTA_ushort vertices = get_vertices();
  ShadeModel shade_model = get_shade_model();

  PTA_ushort new_vertices;
  new_vertices.reserve(vertices.size());

  for (int begin = 0; begin < (int)vertices.size(); begin += 2) {
    new_vertices.push_back(vertices[begin + 1]);
    new_vertices.push_back(vertices[begin]);
  }
  
  nassertr(new_vertices.size() == vertices.size(), vertices);
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
  qpGeomLines *object = new qpGeomLines(qpGeomUsageHint::UH_client);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
