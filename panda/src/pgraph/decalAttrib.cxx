// Filename: decalAttrib.cxx
// Created by:  drose (04Mar02)
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

#include "decalAttrib.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle DecalAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DecalAttrib::make
//       Access: Published, Static
//  Description: Constructs a new DecalAttrib object.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) DecalAttrib::
make() {
  DecalAttrib *attrib = new DecalAttrib;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: DecalAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived DecalAttrib
//               types to return a unique number indicating whether
//               this DecalAttrib is equivalent to the other one.
//
//               This should return 0 if the two DecalAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two DecalAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int DecalAttrib::
compare_to_impl(const RenderAttrib *other) const {
  // All DecalAttribs are equivalent--there are no properties to
  // store.
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DecalAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived DecalAttrib
//               types to specify what the default property for a
//               DecalAttrib of this type should be.
//
//               This should return a newly-allocated DecalAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of DecalAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *DecalAttrib::
make_default_impl() const {
  return new DecalAttrib;
}

////////////////////////////////////////////////////////////////////
//     Function: DecalAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               DecalAttrib.
////////////////////////////////////////////////////////////////////
void DecalAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: DecalAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void DecalAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);
}

////////////////////////////////////////////////////////////////////
//     Function: DecalAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type DecalAttrib is encountered
//               in the Bam file.  It should create the DecalAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *DecalAttrib::
make_from_bam(const FactoryParams &params) {
  DecalAttrib *attrib = new DecalAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: DecalAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new DecalAttrib.
////////////////////////////////////////////////////////////////////
void DecalAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);
}
