// Filename: qpgeomPoints.cxx
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

#include "qpgeomPoints.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle qpGeomPoints::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPoints::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomPoints::
qpGeomPoints(qpGeomUsageHint::UsageHint usage_hint) :
  qpGeomPrimitive(usage_hint)
{
}
 
////////////////////////////////////////////////////////////////////
//     Function: qpGeomPoints::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomPoints::
qpGeomPoints(const qpGeomPoints &copy) :
  qpGeomPrimitive(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPoints::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomPoints::
~qpGeomPoints() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPoints::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PT(qpGeomPrimitive) qpGeomPoints::
make_copy() const {
  return new qpGeomPoints(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPoints::get_primitive_type
//       Access: Public, Virtual
//  Description: Returns the fundamental rendering type of this
//               primitive: whether it is points, lines, or polygons.
//               This is used primarily to set up the appropriate
//               antialiasing settings when AntialiasAttrib::M_auto is
//               in effect.
////////////////////////////////////////////////////////////////////
qpGeomPrimitive::PrimitiveType qpGeomPoints::
get_primitive_type() const {
  return PT_points;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPoints::get_num_vertices_per_primitive
//       Access: Public, Virtual
//  Description: If the primitive type is a simple type in which all
//               primitives have the same number of vertices, like
//               points, returns the number of vertices per
//               primitive.  If the primitive type is a more complex
//               type in which different primitives might have
//               different numbers of vertices, for instance a
//               point strip, returns 0.
////////////////////////////////////////////////////////////////////
int qpGeomPoints::
get_num_vertices_per_primitive() const {
  return 1;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPoints::get_min_num_vertices_per_primitive
//       Access: Public, Virtual
//  Description: Returns the minimum number of vertices that must be
//               added before close_primitive() may legally be called.
////////////////////////////////////////////////////////////////////
int qpGeomPoints::
get_min_num_vertices_per_primitive() const {
  return 1;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPoints::draw
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the GSG to draw the
//               primitive.
////////////////////////////////////////////////////////////////////
void qpGeomPoints::
draw(GraphicsStateGuardianBase *gsg) const {
  gsg->draw_points(this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPoints::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               qpGeom.
////////////////////////////////////////////////////////////////////
void qpGeomPoints::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomPoints::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type qpGeom is encountered
//               in the Bam file.  It should create the qpGeom
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *qpGeomPoints::
make_from_bam(const FactoryParams &params) {
  qpGeomPoints *object = new qpGeomPoints(qpGeomUsageHint::UH_client);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
