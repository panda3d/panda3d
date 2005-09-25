// Filename: fogAttrib.cxx
// Created by:  drose (14Mar02)
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

#include "fogAttrib.h"
#include "attribSlots.h"
#include "graphicsStateGuardianBase.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle FogAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FogAttrib::make
//       Access: Published, Static
//  Description: Constructs a new FogAttrib object suitable for
//               rendering the indicated fog onto geometry.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) FogAttrib::
make(Fog *fog) {
  FogAttrib *attrib = new FogAttrib;
  attrib->_fog = fog;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: FogAttrib::make_off
//       Access: Published, Static
//  Description: Constructs a new FogAttrib object suitable for
//               rendering unfogd geometry.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) FogAttrib::
make_off() {
  FogAttrib *attrib = new FogAttrib;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: FogAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void FogAttrib::
output(ostream &out) const {
  out << get_type() << ":";
  if (is_off()) {
    out << "(off)";
  } else {
    out << *_fog;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FogAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived FogAttrib
//               types to return a unique number indicating whether
//               this FogAttrib is equivalent to the other one.
//
//               This should return 0 if the two FogAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two FogAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int FogAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const FogAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  // Comparing pointers by subtraction is problematic.  Instead of
  // doing this, we'll just depend on the built-in != and < operators
  // for comparing pointers.
  if (_fog != ta->_fog) {
    return _fog < ta->_fog ? -1 : 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: FogAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived FogAttrib
//               types to specify what the default property for a
//               FogAttrib of this type should be.
//
//               This should return a newly-allocated FogAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of FogAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *FogAttrib::
make_default_impl() const {
  return new FogAttrib;
}

////////////////////////////////////////////////////////////////////
//     Function: FogAttrib::store_into_slot
//       Access: Public, Virtual
//  Description: Stores this attrib into the appropriate slot of
//               an object of class AttribSlots.
////////////////////////////////////////////////////////////////////
void FogAttrib::
store_into_slot(AttribSlots *slots) const {
  slots->_fog = this;
}

////////////////////////////////////////////////////////////////////
//     Function: FogAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               FogAttrib.
////////////////////////////////////////////////////////////////////
void FogAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: FogAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void FogAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  manager->write_pointer(dg, _fog);
}

////////////////////////////////////////////////////////////////////
//     Function: FogAttrib::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int FogAttrib::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderAttrib::complete_pointers(p_list, manager);

  TypedWritable *fog = p_list[pi++];
  if (fog != (TypedWritable *)NULL) {
    _fog = DCAST(Fog, fog);
  }

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: FogAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type FogAttrib is encountered
//               in the Bam file.  It should create the FogAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *FogAttrib::
make_from_bam(const FactoryParams &params) {
  FogAttrib *attrib = new FogAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: FogAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new FogAttrib.
////////////////////////////////////////////////////////////////////
void FogAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  // Read the _fog pointer.
  manager->read_pointer(scan);
}
