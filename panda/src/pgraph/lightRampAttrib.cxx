// Filename: lightRampAttrib.cxx
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

#include "lightRampAttrib.h"
#include "attribSlots.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle LightRampAttrib::_type_handle;
CPT(RenderAttrib) LightRampAttrib::_identity;

////////////////////////////////////////////////////////////////////
//     Function: LightRampAttrib::make_identity
//       Access: Published, Static
//  Description: Constructs a new LightRampAttrib object.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightRampAttrib::
make_identity() {
  if (_identity == 0) {
    LightRampAttrib *attrib = new LightRampAttrib();
    _identity = return_new(attrib);
  }
  return _identity;
}

////////////////////////////////////////////////////////////////////
//     Function: LightRampAttrib::make_single_threshold
//       Access: Published, Static
//  Description: Constructs a new LightRampAttrib object.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightRampAttrib::
make_single_threshold(float val0, float val1, float thresh0) {
  LightRampAttrib *attrib = new LightRampAttrib();
  attrib->_mode = LRT_single_threshold;
  attrib->_level[0] = val0;
  attrib->_level[1] = val1;
  attrib->_threshold[0] = thresh0;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: LightRampAttrib::make_double_threshold
//       Access: Published, Static
//  Description: Constructs a new LightRampAttrib object.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightRampAttrib::
make_double_threshold(float val0, float val1, float val2, float thresh0, float thresh1) {
  LightRampAttrib *attrib = new LightRampAttrib();
  attrib->_mode = LRT_single_threshold;
  attrib->_level[0] = val0;
  attrib->_level[1] = val1;
  attrib->_level[2] = val2;
  attrib->_threshold[0] = thresh0;
  attrib->_threshold[1] = thresh1;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: LightRampAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void LightRampAttrib::
output(ostream &out) const {
  out << get_type() << ":";
  switch (_mode) {
  case LRT_identity:
    out << "identity()";
    break;
  case LRT_single_threshold:
    out << "single_threshold(" << _level[0] << "," << _level[1] << "," << _threshold[0] << ")";
    break;
  case LRT_double_threshold:
    out << "double_threshold(" << _level[0] << "," << _level[1] << "," << _level[2] << "," << _threshold[0] << "," << _threshold[0] << ")";
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LightRampAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived LightRampAttrib
//               types to return a unique number indicating whether
//               this LightRampAttrib is equivalent to the other one.
//
//               This should return 0 if the two LightRampAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two LightRampAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int LightRampAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const LightRampAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  int compare_result = ((int)_mode - (int)ta->_mode) ;
  if (compare_result!=0) {
    return compare_result;
  }
  for (int i=0; i<3; i++) {
    compare_result = _level[i] - ta->_level[i];
    if (compare_result!=0) {
      return compare_result;
    }
  }
  for (int i=0; i<2; i++) {
    compare_result = _threshold[i] - ta->_threshold[i];
    if (compare_result!=0) {
      return compare_result;
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LightRampAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived LightRampAttrib
//               types to specify what the default property for a
//               LightRampAttrib of this type should be.
//
//               This should return a newly-allocated LightRampAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of LightRampAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *LightRampAttrib::
make_default_impl() const {
  return new LightRampAttrib;
}

////////////////////////////////////////////////////////////////////
//     Function: LightRampAttrib::store_into_slot
//       Access: Public, Virtual
//  Description: Stores this attrib into the appropriate slot of
//               an object of class AttribSlots.
////////////////////////////////////////////////////////////////////
void LightRampAttrib::
store_into_slot(AttribSlots *slots) const {
  slots->_light_ramp = this;
}

////////////////////////////////////////////////////////////////////
//     Function: LightRampAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               LightRampAttrib.
////////////////////////////////////////////////////////////////////
void LightRampAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: LightRampAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void LightRampAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_int8(_mode);
  for (int i=0; i<3; i++) {
    dg.add_float32(_level[i]);
  }
  for (int i=0; i<2; i++) {
    dg.add_float32(_threshold[i]);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LightRampAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type LightRampAttrib is encountered
//               in the Bam file.  It should create the LightRampAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *LightRampAttrib::
make_from_bam(const FactoryParams &params) {
  LightRampAttrib *attrib = new LightRampAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);
  
  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: LightRampAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new LightRampAttrib.
////////////////////////////////////////////////////////////////////
void LightRampAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _mode = (LightRampMode)scan.get_int8();
  for (int i=0; i<3; i++) {
    _level[i] = scan.get_float32();
  }
  for (int i=0; i<2; i++) {
    _threshold[i] = scan.get_float32();
  }
}
