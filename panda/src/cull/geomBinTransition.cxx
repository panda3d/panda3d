// Filename: geomBinTransition.cxx
// Created by:  drose (07Apr00)
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


#include "geomBinTransition.h"
#include "geomBinAttribute.h"
#include <indent.h>
#include <string.h>

TypeHandle GeomBinTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated GeomBinTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *GeomBinTransition::
make_copy() const {
  return new GeomBinTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated GeomBinAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *GeomBinTransition::
make_attrib() const {
  return new GeomBinAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::make_initial
//       Access: Public, Virtual
//  Description: Returns a newly allocated GeomBinTransition
//               corresponding to the default initial state.
////////////////////////////////////////////////////////////////////
NodeTransition *GeomBinTransition::
make_initial() const {
  return new GeomBinTransition;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another GeomBinTransition.
////////////////////////////////////////////////////////////////////
void GeomBinTransition::
set_value_from(const OnOffTransition *other) {
  const GeomBinTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
  _draw_order = ot->_draw_order;
  nassertv(!_value.empty());
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::compare_values
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int GeomBinTransition::
compare_values(const OnOffTransition *other) const {
  const GeomBinTransition *ot;
  DCAST_INTO_R(ot, other, false);
  if (_value != ot->_value) {
    return strcmp(_value.c_str(), ot->_value.c_str());
  }
  return _draw_order - ot->_draw_order;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void GeomBinTransition::
output_value(ostream &out) const {
  out << _value << ":" << _draw_order;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void GeomBinTransition::
write_value(ostream &out, int indent_level) const {
  indent(out, indent_level) << _value << ":" << _draw_order << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a GeomBinTransition object
////////////////////////////////////////////////////////////////////
void GeomBinTransition::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_GeomBinTransition);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void GeomBinTransition::
write_datagram(BamWriter *manager, Datagram &me) {
  OnOffTransition::write_datagram(manager, me);
  me.add_string(_value);
  me.add_int32(_draw_order);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::make_GeomBinTransition
//       Access: Public
//  Description: Factory method to generate a GeomBinTransition object
////////////////////////////////////////////////////////////////////
TypedWritable *GeomBinTransition::
make_GeomBinTransition(const FactoryParams &params) {
  GeomBinTransition *me = new GeomBinTransition;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinTransition::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void GeomBinTransition::
fillin(DatagramIterator &scan, BamReader *manager) {
  OnOffTransition::fillin(scan, manager);
  _value = scan.get_string();
  _draw_order = scan.get_int32();
}
