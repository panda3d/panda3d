// Filename: cullFaceAttrib.cxx
// Created by:  drose (27Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "cullFaceAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle CullFaceAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttrib::make
//       Access: Published, Static
//  Description: Constructs a new CullFaceAttrib object that specifies
//               how to cull geometry.  By Panda convention, vertices
//               are ordered counterclockwise when seen from the
//               front, so the M_cull_clockwise will cull backfacing
//               polygons.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) CullFaceAttrib::
make(CullFaceAttrib::Mode mode) {
  CullFaceAttrib *attrib = new CullFaceAttrib(mode);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttrib::issue
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the indicated GSG
//               to issue the graphics commands appropriate to the
//               given attribute.  This is normally called
//               (indirectly) only from
//               GraphicsStateGuardian::set_state() or modify_state().
////////////////////////////////////////////////////////////////////
void CullFaceAttrib::
issue(GraphicsStateGuardianBase *gsg) const {
  gsg->issue_cull_face(this);
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void CullFaceAttrib::
output(ostream &out) const {
  out << get_type() << ":";
  switch (get_mode()) {
  case M_cull_none:
    out << "cull_none";
    break;
  case M_cull_clockwise:
    out << "cull_clockwise";
    break;
  case M_cull_counter_clockwise:
    out << "cull_counter_clockwise";
    break;
  case M_cull_all:
    out << "cull_all";
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived CullFaceAttrib
//               types to return a unique number indicating whether
//               this CullFaceAttrib is equivalent to the other one.
//
//               This should return 0 if the two CullFaceAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two CullFaceAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int CullFaceAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const CullFaceAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  return (int)_mode - (int)ta->_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived CullFaceAttrib
//               types to specify what the default property for a
//               CullFaceAttrib of this type should be.
//
//               This should return a newly-allocated CullFaceAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of CullFaceAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *CullFaceAttrib::
make_default_impl() const {
  return new CullFaceAttrib;
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               CullFaceAttrib.
////////////////////////////////////////////////////////////////////
void CullFaceAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void CullFaceAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_int8(_mode);
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type CullFaceAttrib is encountered
//               in the Bam file.  It should create the CullFaceAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *CullFaceAttrib::
make_from_bam(const FactoryParams &params) {
  CullFaceAttrib *attrib = new CullFaceAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: CullFaceAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new CullFaceAttrib.
////////////////////////////////////////////////////////////////////
void CullFaceAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _mode = (Mode)scan.get_int8();
}
