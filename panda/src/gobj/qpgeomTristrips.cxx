// Filename: qpgeomTristrips.cxx
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

#include "qpgeomTristrips.h"
#include "qpgeomTriangles.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle qpGeomTristrips::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomTristrips::
qpGeomTristrips() {
}
 
////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomTristrips::
qpGeomTristrips(const qpGeomTristrips &copy) :
  qpGeomPrimitive(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomTristrips::
~qpGeomTristrips() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::draw
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the GSG to draw the
//               primitive.
////////////////////////////////////////////////////////////////////
void qpGeomTristrips::
draw(GraphicsStateGuardianBase *gsg) {
  gsg->draw_tristrips(this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::decompose_impl
//       Access: Published, Virtual
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
PT(qpGeomPrimitive) qpGeomTristrips::
decompose_impl() {
  PT(qpGeomTriangles) triangles = new qpGeomTriangles;
  CPTA_ushort vertices = get_vertices();
  CPTA_int lengths = get_lengths();

  CPTA_ushort::const_iterator vi;
  vi = vertices.begin();

  CPTA_int::const_iterator li;
  for (li = lengths.begin(); li != lengths.end(); ++li) {
    int length = (*li);
    nassertr(length >= 2, triangles.p());
    int v0 = (*vi);
    ++vi;
    int v1 = (*vi);
    ++vi;
    bool reversed = false;
    for (int i = 2; i < length; i++) {
      if (reversed) {
        triangles->add_vertex(v1);
        triangles->add_vertex(v0);
        reversed = false;
      } else {
        triangles->add_vertex(v0);
        triangles->add_vertex(v1);
        reversed = true;
      }
      triangles->add_vertex(*vi);
      v0 = v1;
      v1 = *vi;
      triangles->close_primitive();
      ++vi;
    }
  }

  return triangles.p();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               qpGeom.
////////////////////////////////////////////////////////////////////
void qpGeomTristrips::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTristrips::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type qpGeom is encountered
//               in the Bam file.  It should create the qpGeom
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *qpGeomTristrips::
make_from_bam(const FactoryParams &params) {
  qpGeomTristrips *object = new qpGeomTristrips;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
