// Filename: textureApplyTransition.cxx
// Created by:  drose (06Oct99)
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

#include "textureApplyTransition.h"
#include "textureApplyAttribute.h"

#include <indent.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle TextureApplyTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated TextureApplyTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *TextureApplyTransition::
make_copy() const {
  return new TextureApplyTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated TextureApplyAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *TextureApplyTransition::
make_attrib() const {
  return new TextureApplyAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another TextureApplyTransition.
////////////////////////////////////////////////////////////////////
void TextureApplyTransition::
set_value_from(const OnTransition *other) {
  const TextureApplyTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyTransition::compare_values
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int TextureApplyTransition::
compare_values(const OnTransition *other) const {
  const TextureApplyTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return _value.compare_to(ot->_value);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void TextureApplyTransition::
output_value(ostream &out) const {
  out << _value;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void TextureApplyTransition::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyTransition::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void TextureApplyTransition::
write_datagram(BamWriter *manager, Datagram &me)
{
  OnTransition::write_datagram(manager, me);
  _value.write_datagram(me);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyTransition::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void TextureApplyTransition::
fillin(DatagramIterator& scan, BamReader* manager)
{
  OnTransition::fillin(scan, manager);
  _value.read_datagram(scan);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyTransition::make_TextureApplyTransition
//       Access: Protected
//  Description: Factory method to generate a TextureApplyTransition object
////////////////////////////////////////////////////////////////////
TypedWritable* TextureApplyTransition::
make_TextureApplyTransition(const FactoryParams &params)
{
  TextureApplyTransition *me = new TextureApplyTransition;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureApplyTransition::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a TextureApplyTransition object
////////////////////////////////////////////////////////////////////
void TextureApplyTransition::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_TextureApplyTransition);
}



