// Filename: auxBitplaneAttrib.cxx
// Created by:  drose (04Mar02)
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

#include "auxBitplaneAttrib.h"
#include "attribSlots.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle AuxBitplaneAttrib::_type_handle;
CPT(RenderAttrib) AuxBitplaneAttrib::_default;

////////////////////////////////////////////////////////////////////
//     Function: AuxBitplaneAttrib::make
//       Access: Published, Static
//  Description: Constructs a default AuxBitplaneAttrib object.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) AuxBitplaneAttrib::
make() {
  if (_default == 0) {
    AuxBitplaneAttrib *attrib = new AuxBitplaneAttrib(ABO_color);
    _default = return_new(attrib);
  }
  return _default;
}

////////////////////////////////////////////////////////////////////
//     Function: AuxBitplaneAttrib::make
//       Access: Published, Static
//  Description: Constructs a specified AuxBitplaneAttrib object.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) AuxBitplaneAttrib::
make(int outputs) {
  AuxBitplaneAttrib *attrib = new AuxBitplaneAttrib(outputs);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: AuxBitplaneAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void AuxBitplaneAttrib::
output(ostream &out) const {
  out << get_type() << "(" << _outputs << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: AuxBitplaneAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived AuxBitplaneAttrib
//               types to return a unique number indicating whether
//               this AuxBitplaneAttrib is equivalent to the other one.
//
//               This should return 0 if the two AuxBitplaneAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two AuxBitplaneAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int AuxBitplaneAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const AuxBitplaneAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  int compare_result = _outputs - ta->_outputs;
  if (compare_result!=0) {
    return compare_result;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: AuxBitplaneAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived AuxBitplaneAttrib
//               types to specify what the default property for a
//               AuxBitplaneAttrib of this type should be.
//
//               This should return a newly-allocated AuxBitplaneAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of AuxBitplaneAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *AuxBitplaneAttrib::
make_default_impl() const {
  return new AuxBitplaneAttrib(ABO_color);
}

////////////////////////////////////////////////////////////////////
//     Function: AuxBitplaneAttrib::store_into_slot
//       Access: Public, Virtual
//  Description: Stores this attrib into the appropriate slot of
//               an object of class AttribSlots.
////////////////////////////////////////////////////////////////////
void AuxBitplaneAttrib::
store_into_slot(AttribSlots *slots) const {
  slots->_aux_bitplane = this;
}

////////////////////////////////////////////////////////////////////
//     Function: AuxBitplaneAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               AuxBitplaneAttrib.
////////////////////////////////////////////////////////////////////
void AuxBitplaneAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: AuxBitplaneAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void AuxBitplaneAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_int32(_outputs);
}

////////////////////////////////////////////////////////////////////
//     Function: AuxBitplaneAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type AuxBitplaneAttrib is encountered
//               in the Bam file.  It should create the AuxBitplaneAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *AuxBitplaneAttrib::
make_from_bam(const FactoryParams &params) {
  AuxBitplaneAttrib *attrib = new AuxBitplaneAttrib(0);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);
  
  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: AuxBitplaneAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new AuxBitplaneAttrib.
////////////////////////////////////////////////////////////////////
void AuxBitplaneAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _outputs = scan.get_int32();
}
