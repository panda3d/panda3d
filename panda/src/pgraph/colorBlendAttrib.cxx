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
//     Function: ColorBlendAttrib::make_off
//       Access: Published, Static
//  Description: Constructs a new ColorBlendAttrib object that
//               disables special-effect blending, allowing normal
//               transparency to be used instead.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ColorBlendAttrib::
make_off() {
  ColorBlendAttrib *attrib = new ColorBlendAttrib;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::make
//       Access: Published, Static
//  Description: Constructs a new ColorBlendAttrib object.  This
//               constructor is deprecated; use the one below, which
//               takes three or four parameters, instead.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ColorBlendAttrib::
make(ColorBlendAttrib::Mode mode) {
  ColorBlendAttrib *attrib = new ColorBlendAttrib(mode, O_one, O_one,
                                                  Colorf::zero());
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorBlendAttrib::make
//       Access: Published, Static
//  Description: Constructs a new ColorBlendAttrib object that enables
//               special-effect blending.  This supercedes
//               transparency.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ColorBlendAttrib::
make(ColorBlendAttrib::Mode mode, 
     ColorBlendAttrib::Operand a, ColorBlendAttrib::Operand b,
     const Colorf &color) {
  ColorBlendAttrib *attrib = new ColorBlendAttrib(mode, a, b, color);
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
  out << get_type() << ":" << get_mode();

  if (get_mode() != M_none) {
    out << "(" << get_operand_a()
        << "," << get_operand_b();
    if (involves_constant_color()) {
      out << "," << get_color();
    }
    out << ")";
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

  if (_mode != ta->_mode) {
    return (int)_mode - (int)ta->_mode;
  }

  if (_a != ta->_a) {
    return (int)_a - (int)ta->_a;
  }

  if (_b != ta->_b) {
    return (int)_b - (int)ta->_b;
  }

  return _color.compare_to(ta->_color);
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

  dg.add_uint8(_mode);
  dg.add_uint8(_a);
  dg.add_uint8(_b);
  _color.write_datagram(dg);
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

  _mode = (Mode)scan.get_uint8();
  _a = (Operand)scan.get_uint8();
  _b = (Operand)scan.get_uint8();
  _color.read_datagram(scan);

  _involves_constant_color = involves_constant_color(_a) || involves_constant_color(_b);
  _involves_color_scale = involves_color_scale(_a) || involves_color_scale(_b);
}

////////////////////////////////////////////////////////////////////
//     Function: ostream << ColorBlendAttrib::Mode
//  Description: 
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, ColorBlendAttrib::Mode mode) {
  switch (mode) {
  case ColorBlendAttrib::M_none:
    return out << "none";

  case ColorBlendAttrib::M_add:
    return out << "add";

  case ColorBlendAttrib::M_subtract:
    return out << "subtract";

  case ColorBlendAttrib::M_inv_subtract:
    return out << "inv_subtract";

  case ColorBlendAttrib::M_min:
    return out << "min";

  case ColorBlendAttrib::M_max:
    return out << "max";
  }

  return out << "**invalid ColorBlendAttrib::Mode(" << (int)mode << ")**";
}

////////////////////////////////////////////////////////////////////
//     Function: ostream << ColorBlendAttrib::Operand
//  Description: 
////////////////////////////////////////////////////////////////////
ostream &
operator << (ostream &out, ColorBlendAttrib::Operand operand) {
  switch (operand) {
  case ColorBlendAttrib::O_zero:
    return out << "0";

  case ColorBlendAttrib::O_one:
    return out << "1";

  case ColorBlendAttrib::O_incoming_color:
    return out << "ic";

  case ColorBlendAttrib::O_one_minus_incoming_color:
    return out << "1-ic";

  case ColorBlendAttrib::O_fbuffer_color:
    return out << "fc";

  case ColorBlendAttrib::O_one_minus_fbuffer_color:
    return out << "1-fc";

  case ColorBlendAttrib::O_incoming_alpha:
    return out << "ia";

  case ColorBlendAttrib::O_one_minus_incoming_alpha:
    return out << "1-ia";

  case ColorBlendAttrib::O_fbuffer_alpha:
    return out << "fa";

  case ColorBlendAttrib::O_one_minus_fbuffer_alpha:
    return out << "1-fa";

  case ColorBlendAttrib::O_constant_color:
    return out << "cc";

  case ColorBlendAttrib::O_one_minus_constant_color:
    return out << "1-cc";

  case ColorBlendAttrib::O_constant_alpha:
    return out << "ca";

  case ColorBlendAttrib::O_one_minus_constant_alpha:
    return out << "1-ca";

  case ColorBlendAttrib::O_incoming_color_saturate:
    return out << "ics";

  case ColorBlendAttrib::O_color_scale:
    return out << "cs";

  case ColorBlendAttrib::O_one_minus_color_scale:
    return out << "1-cs";

  case ColorBlendAttrib::O_alpha_scale:
    return out << "as";

  case ColorBlendAttrib::O_one_minus_alpha_scale:
    return out << "1-as";
  }

  return out << "**invalid ColorBlendAttrib::Operand(" << (int)operand << ")**";
}
