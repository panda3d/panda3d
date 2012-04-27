// Filename: geomPatches.cxx
// Created by:  drose (27Apr12)
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

#include "geomPatches.h"
#include "geomVertexRewriter.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "graphicsStateGuardianBase.h"

TypeHandle GeomPatches::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomPatches::Constructor
//       Access: Published
//  Description: The number of vertices per patch must be specified to
//               the GeomPatches constructor, and it may not be
//               changed during the lifetime of the GeomPatches
//               object.  Create a new GeomPatches if you need to have
//               a different value.
////////////////////////////////////////////////////////////////////
GeomPatches::
GeomPatches(int num_vertices_per_patch, GeomPatches::UsageHint usage_hint) :
  GeomPrimitive(usage_hint),
  _num_vertices_per_patch(num_vertices_per_patch)
{
}
 
////////////////////////////////////////////////////////////////////
//     Function: GeomPatches::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
GeomPatches::
GeomPatches(const GeomPatches &copy) :
  GeomPrimitive(copy),
  _num_vertices_per_patch(copy._num_vertices_per_patch)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPatches::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
GeomPatches::
~GeomPatches() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPatches::make_copy
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PT(GeomPrimitive) GeomPatches::
make_copy() const {
  return new GeomPatches(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPatches::get_primitive_type
//       Access: Public, Virtual
//  Description: Returns the fundamental rendering type of this
//               primitive: whether it is points, lines, or polygons.
//
//               This is used to set up the appropriate antialiasing
//               settings when AntialiasAttrib::M_auto is in effect;
//               it also implies the type of primitive that will be
//               produced when decompose() is called.
////////////////////////////////////////////////////////////////////
GeomPrimitive::PrimitiveType GeomPatches::
get_primitive_type() const {
  return PT_patches;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPatches::get_num_vertices_per_primitive
//       Access: Public, Virtual
//  Description: If the primitive type is a simple type in which all
//               primitives have the same number of vertices, like
//               patches, returns the number of vertices per
//               primitive.  If the primitive type is a more complex
//               type in which different primitives might have
//               different numbers of vertices, for instance a
//               triangle strip, returns 0.
//
//               In the case of GeomPatches, this returns the fixed
//               number that was specified to the constructor.
////////////////////////////////////////////////////////////////////
int GeomPatches::
get_num_vertices_per_primitive() const {
  return _num_vertices_per_patch;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPatches::draw
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the GSG to draw the
//               primitive.
////////////////////////////////////////////////////////////////////
bool GeomPatches::
draw(GraphicsStateGuardianBase *gsg, const GeomPrimitivePipelineReader *reader,
     bool force) const {
  return gsg->draw_patches(reader, force);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPatches::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               Geom.
////////////////////////////////////////////////////////////////////
void GeomPatches::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPatches::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void GeomPatches::
write_datagram(BamWriter *manager, Datagram &dg) {
  GeomPrimitive::write_datagram(manager, dg);
  dg.add_uint16(_num_vertices_per_patch);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPatches::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new GeomPatches.
////////////////////////////////////////////////////////////////////
void GeomPatches::
fillin(DatagramIterator &scan, BamReader *manager) {
  GeomPrimitive::fillin(scan, manager);
  _num_vertices_per_patch = scan.get_uint16();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPatches::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type Geom is encountered
//               in the Bam file.  It should create the Geom
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *GeomPatches::
make_from_bam(const FactoryParams &params) {
  GeomPatches *object = new GeomPatches(0, UH_unspecified);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}
