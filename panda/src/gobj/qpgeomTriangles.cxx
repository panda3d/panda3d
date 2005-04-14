// Filename: qpgeomTriangles.cxx
// Created by:  drose (06Mar05)
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

#include "qpgeomTriangles.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle qpGeomTriangles::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTriangles::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomTriangles::
qpGeomTriangles(qpGeomTriangles::UsageHint usage_hint) :
  qpGeomPrimitive(usage_hint)
{
}
 
////////////////////////////////////////////////////////////////////
//     Function: qpGeomTriangles::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomTriangles::
qpGeomTriangles(const qpGeomTriangles &copy) :
  qpGeomPrimitive(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTriangles::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomTriangles::
~qpGeomTriangles() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTriangles::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PT(qpGeomPrimitive) qpGeomTriangles::
make_copy() const {
  return new qpGeomTriangles(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTriangles::get_primitive_type
//       Access: Public, Virtual
//  Description: Returns the fundamental rendering type of this
//               primitive: whether it is points, lines, or polygons.
//               This is used primarily to set up the appropriate
//               antialiasing settings when AntialiasAttrib::M_auto is
//               in effect.
////////////////////////////////////////////////////////////////////
qpGeomPrimitive::PrimitiveType qpGeomTriangles::
get_primitive_type() const {
  return PT_polygons;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTriangles::get_num_vertices_per_primitive
//       Access: Public, Virtual
//  Description: If the primitive type is a simple type in which all
//               primitives have the same number of vertices, like
//               triangles, returns the number of vertices per
//               primitive.  If the primitive type is a more complex
//               type in which different primitives might have
//               different numbers of vertices, for instance a
//               triangle strip, returns 0.
////////////////////////////////////////////////////////////////////
int qpGeomTriangles::
get_num_vertices_per_primitive() const {
  return 3;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTriangles::draw
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the GSG to draw the
//               primitive.
////////////////////////////////////////////////////////////////////
void qpGeomTriangles::
draw(GraphicsStateGuardianBase *gsg) const {
  gsg->draw_triangles(this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTriangles::rotate_impl
//       Access: Protected, Virtual
//  Description: The virtual implementation of do_rotate().
////////////////////////////////////////////////////////////////////
CPTA_ushort qpGeomTriangles::
rotate_impl() const {
  // To rotate triangles, we just move one vertex from the front to
  // the back, or vice-versa; but we have to know what direction we're
  // going.
  CPTA_ushort vertices = get_vertices();
  ShadeModel shade_model = get_shade_model();

  PTA_ushort new_vertices;
  new_vertices.reserve(vertices.size());

  switch (shade_model) {
  case SM_flat_first_vertex:
    // Move the first vertex to the end.
    {
      for (int begin = 0; begin < (int)vertices.size(); begin += 3) {
        new_vertices.push_back(vertices[begin + 1]);
        new_vertices.push_back(vertices[begin + 2]);
        new_vertices.push_back(vertices[begin]);
      }
    }
    break;
    
  case SM_flat_last_vertex:
    // Move the last vertex to the front.
    {
      for (int begin = 0; begin < (int)vertices.size(); begin += 3) {
        new_vertices.push_back(vertices[begin + 2]);
        new_vertices.push_back(vertices[begin]);
        new_vertices.push_back(vertices[begin + 1]);
      }
    }
    break;
      
  default:
    // This shouldn't get called with any other shade model.
    nassertr(false, vertices);
  }
  
  nassertr(new_vertices.size() == vertices.size(), vertices);
  return new_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTriangles::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               qpGeom.
////////////////////////////////////////////////////////////////////
void qpGeomTriangles::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomTriangles::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type qpGeom is encountered
//               in the Bam file.  It should create the qpGeom
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *qpGeomTriangles::
make_from_bam(const FactoryParams &params) {
  qpGeomTriangles *object = new qpGeomTriangles(UH_unspecified);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
