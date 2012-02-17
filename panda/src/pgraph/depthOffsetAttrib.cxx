// Filename: depthOffsetAttrib.cxx
// Created by:  drose (14Mar02)
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

#include "depthOffsetAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle DepthOffsetAttrib::_type_handle;
int DepthOffsetAttrib::_attrib_slot;

////////////////////////////////////////////////////////////////////
//     Function: DepthOffsetAttrib::make
//       Access: Published, Static
//  Description: Constructs a new DepthOffsetAttrib object that
//               indicates the relative amount of bias to write to the
//               depth buffer for subsequent geometry.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) DepthOffsetAttrib::
make(int offset) {
  DepthOffsetAttrib *attrib = new DepthOffsetAttrib(offset, 0.0f, 1.0f);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthOffsetAttrib::make
//       Access: Published, Static
//  Description: Constructs a new DepthOffsetAttrib object that
//               indicates the bias, and also specifies a minimum and
//               maximum (or, more precisely, nearest and farthest)
//               values to write to the depth buffer, in the range 0
//               .. 1.  This range is 0, 1 by default; setting it to
//               some other range can be used to create additional
//               depth buffer effects.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) DepthOffsetAttrib::
make(int offset, PN_stdfloat min_value, PN_stdfloat max_value) {
  nassertr(min_value >= 0.0f && min_value <= 1.0f, NULL);
  nassertr(max_value >= 0.0f && max_value <= 1.0f, NULL);
  DepthOffsetAttrib *attrib = new DepthOffsetAttrib(offset, min_value, max_value);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthOffsetAttrib::make_default
//       Access: Published, Static
//  Description: Returns a RenderAttrib that corresponds to whatever
//               the standard default properties for render attributes
//               of this type ought to be.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) DepthOffsetAttrib::
make_default() {
  return return_new(new DepthOffsetAttrib(0, 0.0f, 1.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: DepthOffsetAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DepthOffsetAttrib::
output(ostream &out) const {
  out << get_type() << ":(" << get_offset() << ", " << get_min_value()
      << ", " << get_max_value() << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: DepthOffsetAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived DepthOffsetAttrib
//               types to return a unique number indicating whether
//               this DepthOffsetAttrib is equivalent to the other one.
//
//               This should return 0 if the two DepthOffsetAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two DepthOffsetAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int DepthOffsetAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const DepthOffsetAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  if (_offset != ta->_offset) {
    return _offset - ta->_offset;
  }
  if (_min_value != ta->_min_value) {
    return _min_value < ta->_min_value ? -1 : 1;
  }
  if (_max_value != ta->_max_value) {
    return _max_value < ta->_max_value ? -1 : 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: DepthOffsetAttrib::get_hash_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to return a unique hash for these particular
//               properties.  RenderAttribs that compare the same with
//               compare_to_impl(), above, should return the same
//               hash; RenderAttribs that compare differently should
//               return a different hash.
////////////////////////////////////////////////////////////////////
size_t DepthOffsetAttrib::
get_hash_impl() const {
  size_t hash = 0;
  hash = int_hash::add_hash(hash, _offset);
  hash = float_hash().add_hash(hash, _min_value);
  hash = float_hash().add_hash(hash, _max_value);
  return hash;
}

////////////////////////////////////////////////////////////////////
//     Function: DepthOffsetAttrib::compose_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               This should return the result of applying the other
//               RenderAttrib to a node in the scene graph below this
//               RenderAttrib, which was already applied.  In most
//               cases, the result is the same as the other
//               RenderAttrib (that is, a subsequent RenderAttrib
//               completely replaces the preceding one).  On the other
//               hand, some kinds of RenderAttrib (for instance,
//               ColorTransformAttrib) might combine in meaningful
//               ways.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) DepthOffsetAttrib::
compose_impl(const RenderAttrib *other) const {
  const DepthOffsetAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  int new_offset = ta->_offset + _offset;

  DepthOffsetAttrib *attrib = new DepthOffsetAttrib(new_offset, ta->_min_value, ta->_max_value);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthOffsetAttrib::invert_compose_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               See invert_compose() and compose_impl().
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) DepthOffsetAttrib::
invert_compose_impl(const RenderAttrib *other) const {
  const DepthOffsetAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  int new_offset = ta->_offset - _offset;

  DepthOffsetAttrib *attrib = new DepthOffsetAttrib(new_offset, ta->_min_value, ta->_max_value);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthOffsetAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               DepthOffsetAttrib.
////////////////////////////////////////////////////////////////////
void DepthOffsetAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthOffsetAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void DepthOffsetAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_int32(_offset);
  dg.add_stdfloat(_min_value);
  dg.add_stdfloat(_max_value);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthOffsetAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type DepthOffsetAttrib is encountered
//               in the Bam file.  It should create the DepthOffsetAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *DepthOffsetAttrib::
make_from_bam(const FactoryParams &params) {
  DepthOffsetAttrib *attrib = new DepthOffsetAttrib(0, 0.0f, 1.0f);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: DepthOffsetAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new DepthOffsetAttrib.
////////////////////////////////////////////////////////////////////
void DepthOffsetAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _offset = scan.get_int32();
  _min_value = 0.0f;
  _max_value = 1.0f;
  if (manager->get_file_minor_ver() >= 31) {
    _min_value = scan.get_stdfloat();
    _max_value = scan.get_stdfloat();
  }
}
