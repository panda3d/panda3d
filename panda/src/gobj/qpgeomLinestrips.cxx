// Filename: qpgeomLinestrips.cxx
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

#include "qpgeomLinestrips.h"
#include "qpgeomLines.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle qpGeomLinestrips::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpGeomLinestrips::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomLinestrips::
qpGeomLinestrips(qpGeomUsageHint::UsageHint usage_hint) :
  qpGeomPrimitive(usage_hint)
{
}
 
////////////////////////////////////////////////////////////////////
//     Function: qpGeomLinestrips::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomLinestrips::
qpGeomLinestrips(const qpGeomLinestrips &copy) :
  qpGeomPrimitive(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomLinestrips::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomLinestrips::
~qpGeomLinestrips() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomLinestrips::make_copy
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PT(qpGeomPrimitive) qpGeomLinestrips::
make_copy() const {
  return new qpGeomLinestrips(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomLinestrips::get_primitive_type
//       Access: Published, Virtual
//  Description: Returns the fundamental rendering type of this
//               primitive: whether it is points, lines, or polygons.
//               This is used primarily to set up the appropriate
//               antialiasing settings when AntialiasAttrib::M_auto is
//               in effect.
////////////////////////////////////////////////////////////////////
qpGeomPrimitive::PrimitiveType qpGeomLinestrips::
get_primitive_type() const {
  return PT_lines;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomLinestrips::draw
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the GSG to draw the
//               primitive.
////////////////////////////////////////////////////////////////////
void qpGeomLinestrips::
draw(GraphicsStateGuardianBase *gsg) const {
  gsg->draw_linestrips(this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomLinestrips::decompose_impl
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
CPT(qpGeomPrimitive) qpGeomLinestrips::
decompose_impl() const {
  PT(qpGeomLines) lines = new qpGeomLines(get_usage_hint());
  lines->set_shade_model(get_shade_model());
  CPTA_ushort vertices = get_vertices();
  CPTA_int ends = get_ends();

  int vi = 0;
  int li = 0;
  while (li < (int)ends.size()) {
    int end = ends[li];
    nassertr(vi + 1 <= end, lines.p());
    nassertr(vi < (int)vertices.size(), this);
    int v0 = vertices[vi];
    ++vi;
    while (vi < end) {
      lines->add_vertex(v0);
      lines->add_vertex(vertices[vi]);
      nassertr(vi < (int)vertices.size(), this);
      v0 = vertices[vi];
      lines->close_primitive();
      ++vi;
    }
    ++li;
  }
  nassertr(vi == (int)vertices.size(), lines.p());

  return lines.p();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomLinestrips::rotate_impl
//       Access: Protected, Virtual
//  Description: The virtual implementation of do_rotate().
////////////////////////////////////////////////////////////////////
CPTA_ushort qpGeomLinestrips::
rotate_impl() const {
  // To rotate a line strip, we just reverse the vertices.
  CPTA_ushort vertices = get_vertices();
  CPTA_int ends = get_ends();
  PTA_ushort new_vertices;
  new_vertices.reserve(vertices.size());

  int begin = 0;
  CPTA_int::const_iterator ei;
  for (ei = ends.begin(); ei != ends.end(); ++ei) {
    int end = (*ei);
    int num_vertices = end - begin;

    for (int vi = end - 1; vi >= begin; --vi) {
      new_vertices.push_back(vertices[vi]);
    }

    begin = end;
  }
  nassertr(new_vertices.size() == vertices.size(), vertices);

  return new_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomLinestrips::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               qpGeom.
////////////////////////////////////////////////////////////////////
void qpGeomLinestrips::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomLinestrips::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type qpGeom is encountered
//               in the Bam file.  It should create the qpGeom
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *qpGeomLinestrips::
make_from_bam(const FactoryParams &params) {
  qpGeomLinestrips *object = new qpGeomLinestrips(qpGeomUsageHint::UH_client);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
