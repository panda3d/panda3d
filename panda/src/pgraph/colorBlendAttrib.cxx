// Filename: colorBlendAttrib.cxx
// Created by:  drose (29Mar02)
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

#include "colorBlendAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle ColorBlendAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::make
//       Access: Published, Static
//  Description: Constructs a new ColorBlendAttrib object.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ColorBlendAttrib::
make(ColorBlendAttrib::Mode mode) {
  ColorBlendAttrib *attrib = new ColorBlendAttrib(mode);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::issue
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the indicated GSG
//               to issue the graphics commands appropriate to the
//               given attribute.  This is normally called
//               (indirectly) only from
//               GraphicsStateGuardian::set_state() or modify_state().
////////////////////////////////////////////////////////////////////
void ColorBlendAttrib::
issue(GraphicsStateGuardianBase *gsg) const {
  gsg->issue_color_blend(this);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void ColorBlendAttrib::
output(ostream &out) const {
  out << get_type() << ":";
  switch (get_mode()) {
  case M_none:
    out << "none";
    break;

  case M_multiply:
    out << "multiply";
    break;

  case M_add:
    out << "add";
    break;

  case M_multiply_add:
    out << "multiply_add";
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived ColorBlendAttrib
//               types to return a unique number indicating whether
//               this ColorBlendAttrib is equivalent to the other one.
//
//               This should return 0 if the two ColorBlendAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two ColorBlendAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int ColorBlendAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const ColorBlendAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  return (int)_mode - (int)ta->_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived ColorBlendAttrib
//               types to specify what the default property for a
//               ColorBlendAttrib of this type should be.
//
//               This should return a newly-allocated ColorBlendAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of ColorBlendAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *ColorBlendAttrib::
make_default_impl() const {
  return new ColorBlendAttrib;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               ColorBlendAttrib.
////////////////////////////////////////////////////////////////////
void ColorBlendAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void ColorBlendAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_int8(_mode);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type ColorBlendAttrib is encountered
//               in the Bam file.  It should create the ColorBlendAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *ColorBlendAttrib::
make_from_bam(const FactoryParams &params) {
  ColorBlendAttrib *attrib = new ColorBlendAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new ColorBlendAttrib.
////////////////////////////////////////////////////////////////////
void ColorBlendAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _mode = (Mode)scan.get_int8();
}
