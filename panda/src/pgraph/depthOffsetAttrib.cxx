// Filename: depthOffsetAttrib.cxx
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

#include "depthOffsetAttrib.h"
#include "attribSlots.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle DepthOffsetAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DepthOffsetAttrib::make
//       Access: Published, Static
//  Description: Constructs a new DepthOffsetAttrib object that
//               indicates the relative amount of bias to write to the
//               depth buffer for subsequent geometry.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) DepthOffsetAttrib::
make(int offset) {
  DepthOffsetAttrib *attrib = new DepthOffsetAttrib(offset);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthOffsetAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void DepthOffsetAttrib::
output(ostream &out) const {
  out << get_type() << ":(" << get_offset() << ")";
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
  return _offset - ta->_offset;
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

  DepthOffsetAttrib *attrib = new DepthOffsetAttrib(new_offset);
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

  DepthOffsetAttrib *attrib = new DepthOffsetAttrib(new_offset);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthOffsetAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived DepthOffsetAttrib
//               types to specify what the default property for a
//               DepthOffsetAttrib of this type should be.
//
//               This should return a newly-allocated DepthOffsetAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of DepthOffsetAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *DepthOffsetAttrib::
make_default_impl() const {
  return new DepthOffsetAttrib(0);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthOffsetAttrib::store_into_slot
//       Access: Public, Virtual
//  Description: When attribs are stored in a slot-based attrib array,
//               this returns the index of the appropriate slot
//               for this attrib type.
////////////////////////////////////////////////////////////////////
void DepthOffsetAttrib::
store_into_slot(AttribSlots *slots) const {
  slots->_depth_offset = this;
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
  DepthOffsetAttrib *attrib = new DepthOffsetAttrib(0);
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
}
