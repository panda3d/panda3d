// Filename: depthTestTransition.cxx
// Created by:  mike (28Jan99)
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

#include "depthTestTransition.h"

#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>
#include <indent.h>

TypeHandle DepthTestTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DepthTestTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated DepthTestTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *DepthTestTransition::
make_copy() const {
  return new DepthTestTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestTransition::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated DepthTestTransition
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeTransition *DepthTestTransition::
make_initial() const {
  return new DepthTestTransition;
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestTransition::issue
//       Access: Public, Virtual
//  Description: This is called on scene graph rendering attributes
//               when it is time to issue the particular attribute to
//               the graphics engine.  It should call the appropriate
//               method on GraphicsStateGuardianBase.
////////////////////////////////////////////////////////////////////
void DepthTestTransition::
issue(GraphicsStateGuardianBase *gsgbase) {
  gsgbase->issue_depth_test(this);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another DepthTestTransition.
////////////////////////////////////////////////////////////////////
void DepthTestTransition::
set_value_from(const OnTransition *other) {
  const DepthTestTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestTransition::compare_values
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int DepthTestTransition::
compare_values(const OnTransition *other) const {
  const DepthTestTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void DepthTestTransition::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void DepthTestTransition::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestTransition::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a DepthTestTransition object
////////////////////////////////////////////////////////////////////
void DepthTestTransition::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_DepthTestTransition);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestTransition::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void DepthTestTransition::
write_datagram(BamWriter *manager, Datagram &me) {
  OnTransition::write_datagram(manager, me);
  _value.write_datagram(me);
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestTransition::make_DepthTestTransition
//       Access: Protected
//  Description: Factory method to generate a DepthTestTransition object
////////////////////////////////////////////////////////////////////
TypedWritable* DepthTestTransition::
make_DepthTestTransition(const FactoryParams &params) {
  DepthTestTransition *me = new DepthTestTransition;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: DepthTestTransition::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void DepthTestTransition::
fillin(DatagramIterator& scan, BamReader* manager) {
  OnTransition::fillin(scan, manager);
  _value.read_datagram(scan);
}
