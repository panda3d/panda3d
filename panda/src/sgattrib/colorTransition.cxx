// Filename: colorTransition.cxx
// Created by:  drose (28Jan99)
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

#include "colorTransition.h"

#include <indent.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle ColorTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *ColorTransition::
make_copy() const {
  return new ColorTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated ColorTransition
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeTransition *ColorTransition::
make_initial() const {
  return new ColorTransition;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void ColorTransition::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_color(this);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another ColorTransition.
////////////////////////////////////////////////////////////////////
void ColorTransition::
set_value_from(const OnOffTransition *other) {
  const ColorTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::compare_values
//       Access: Protected, Virtual
//  Description: Returns true if the two transitions have the same
//               value.  It is guaranteed that the other transition is
//               another ColorTransition, and that both are "on".
////////////////////////////////////////////////////////////////////
int ColorTransition::
compare_values(const OnOffTransition *other) const {
  const ColorTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void ColorTransition::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void ColorTransition::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::register_with_read_factory
//       Access: Public, Static
//  Description: Factory method to generate a ColorTransition object
////////////////////////////////////////////////////////////////////
void ColorTransition::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_ColorTransition);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void ColorTransition::
write_datagram(BamWriter *manager, Datagram &me) {
  OnOffTransition::write_datagram(manager, me);
  _value.write_datagram(me);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::make_ColorTransition
//       Access: Protected
//  Description: Factory method to generate a ColorTransition object
////////////////////////////////////////////////////////////////////
TypedWritable *ColorTransition::
make_ColorTransition(const FactoryParams &params) {
  ColorTransition *me = new ColorTransition;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorTransition::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void ColorTransition::
fillin(DatagramIterator& scan, BamReader* manager) {
  OnOffTransition::fillin(scan, manager);
  _value.read_datagram(scan);
}
