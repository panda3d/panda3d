// Filename: geomPoints.cxx
// Created by:  drose (22Mar05)
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

#include "geomPoints.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "graphicsStateGuardianBase.h"

TypeHandle GeomPoints::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomPoints::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomPoints::
GeomPoints(GeomPoints::UsageHint usage_hint) :
  GeomPrimitive(usage_hint)
{
}
 
////////////////////////////////////////////////////////////////////
//     Function: GeomPoints::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomPoints::
GeomPoints(const GeomPoints &copy) :
  GeomPrimitive(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPoints::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
GeomPoints::
~GeomPoints() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPoints::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PT(GeomPrimitive) GeomPoints::
make_copy() const {
  return new GeomPoints(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPoints::get_primitive_type
//       Access: Public, Virtual
//  Description: Returns the fundamental rendering type of this
//               primitive: whether it is points, lines, or polygons.
//
//               This is used to set up the appropriate antialiasing
//               settings when AntialiasAttrib::M_auto is in effect;
//               it also implies the type of primitive that will be
//               produced when decompose() is called.
////////////////////////////////////////////////////////////////////
GeomPrimitive::PrimitiveType GeomPoints::
get_primitive_type() const {
  return PT_points;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPoints::get_geom_rendering
//       Access: Published, Virtual
//  Description: Returns the set of GeomRendering bits that represent
//               the rendering properties required to properly render
//               this primitive.
////////////////////////////////////////////////////////////////////
int GeomPoints::
get_geom_rendering() const {
  // Fancy point attributes, if any, are based on whether the
  // appropriate columns are defined in the associated GeomVertexData;
  // these bits will be added by Geom::get_geom_rendering().
  if (is_indexed()) {
    return GR_point | GR_indexed_point;
  } else {
    return GR_point;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPoints::get_num_vertices_per_primitive
//       Access: Public, Virtual
//  Description: If the primitive type is a simple type in which all
//               primitives have the same number of vertices, like
//               points, returns the number of vertices per
//               primitive.  If the primitive type is a more complex
//               type in which different primitives might have
//               different numbers of vertices, for instance a
//               point strip, returns 0.
////////////////////////////////////////////////////////////////////
int GeomPoints::
get_num_vertices_per_primitive() const {
  return 1;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPoints::get_min_num_vertices_per_primitive
//       Access: Public, Virtual
//  Description: Returns the minimum number of vertices that must be
//               added before close_primitive() may legally be called.
////////////////////////////////////////////////////////////////////
int GeomPoints::
get_min_num_vertices_per_primitive() const {
  return 1;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPoints::draw
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the GSG to draw the
//               primitive.
////////////////////////////////////////////////////////////////////
bool GeomPoints::
draw(GraphicsStateGuardianBase *gsg, const GeomPrimitivePipelineReader *reader,
     bool force) const {
  return gsg->draw_points(reader, force);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPoints::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               Geom.
////////////////////////////////////////////////////////////////////
void GeomPoints::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPoints::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type Geom is encountered
//               in the Bam file.  It should create the Geom
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *GeomPoints::
make_from_bam(const FactoryParams &params) {
  GeomPoints *object = new GeomPoints(UH_unspecified);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
