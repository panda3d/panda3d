// Filename: shadeModelAttrib.cxx
// Created by:  drose (14Mar05)
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

#include "shadeModelAttrib.h"
#include "attribSlots.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle ShadeModelAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ShadeModelAttrib::make
//       Access: Published, Static
//  Description: Constructs a new ShadeModelAttrib object that specifies
//               whether to draw polygons with flat shading or with
//               per-vertex (smooth) shading.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ShadeModelAttrib::
make(ShadeModelAttrib::Mode mode) {
  ShadeModelAttrib *attrib = new ShadeModelAttrib(mode);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ShadeModelAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void ShadeModelAttrib::
output(ostream &out) const {
  out << get_type() << ":";
  switch (get_mode()) {
  case M_flat:
    out << "flat";
    break;

  case M_smooth:
    out << "smooth";
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ShadeModelAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived ShadeModelAttrib
//               types to return a unique number indicating whether
//               this ShadeModelAttrib is equivalent to the other one.
//
//               This should return 0 if the two ShadeModelAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two ShadeModelAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int ShadeModelAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const ShadeModelAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  return (int)_mode - (int)ta->_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: ShadeModelAttrib::compose_impl
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
CPT(RenderAttrib) ShadeModelAttrib::
compose_impl(const RenderAttrib *other) const {
  const ShadeModelAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  Mode mode = ta->get_mode();
  return make(mode);
}

////////////////////////////////////////////////////////////////////
//     Function: ShadeModelAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived ShadeModelAttrib
//               types to specify what the default property for a
//               ShadeModelAttrib of this type should be.
//
//               This should return a newly-allocated ShadeModelAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of ShadeModelAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *ShadeModelAttrib::
make_default_impl() const {
  return new ShadeModelAttrib(M_smooth);
}

////////////////////////////////////////////////////////////////////
//     Function: ShadeModelAttrib::store_into_slot
//       Access: Public, Virtual
//  Description: Stores this attrib into the appropriate slot of
//               an object of class AttribSlots.
////////////////////////////////////////////////////////////////////
void ShadeModelAttrib::
store_into_slot(AttribSlots *slots) const {
  slots->_shade_model = this;
}

////////////////////////////////////////////////////////////////////
//     Function: ShadeModelAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               ShadeModelAttrib.
////////////////////////////////////////////////////////////////////
void ShadeModelAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: ShadeModelAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void ShadeModelAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_int8(_mode);
}

////////////////////////////////////////////////////////////////////
//     Function: ShadeModelAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type ShadeModelAttrib is encountered
//               in the Bam file.  It should create the ShadeModelAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *ShadeModelAttrib::
make_from_bam(const FactoryParams &params) {
  ShadeModelAttrib *attrib = new ShadeModelAttrib(M_smooth);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: ShadeModelAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new ShadeModelAttrib.
////////////////////////////////////////////////////////////////////
void ShadeModelAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _mode = (Mode)scan.get_int8();
}
