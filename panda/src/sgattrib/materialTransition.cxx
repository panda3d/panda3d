// Filename: materialTransition.cxx
// Created by:  mike (06Feb99)
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

#include "materialTransition.h"
#include "materialAttribute.h"
#include "config_sgattrib.h"

#include <indent.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle MaterialTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly allocated MaterialTransition just like
//               this one.
////////////////////////////////////////////////////////////////////
NodeTransition *MaterialTransition::
make_copy() const {
  return new MaterialTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::make_attrib
//       Access: Public, Virtual
//  Description: Returns a newly allocated MaterialAttribute.
////////////////////////////////////////////////////////////////////
NodeAttribute *MaterialTransition::
make_attrib() const {
  return new MaterialAttribute;
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::set_value_from
//       Access: Protected, Virtual
//  Description: Copies the value from the other transition pointer,
//               which is guaranteed to be another MaterialTransition.
////////////////////////////////////////////////////////////////////
void MaterialTransition::
set_value_from(const OnOffTransition *other) {
  const MaterialTransition *ot;
  DCAST_INTO_V(ot, other);
  _value = ot->_value;
  nassertv(_value != (const Material *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::compare_values
//       Access: Protected, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int MaterialTransition::
compare_values(const OnOffTransition *other) const {
  const MaterialTransition *ot;
  DCAST_INTO_R(ot, other, false);
  return (int)(_value.p() - ot->_value.p());
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::output_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on one line.
////////////////////////////////////////////////////////////////////
void MaterialTransition::
output_value(ostream &out) const {
  nassertv(_value != (const Material *)NULL);
  out << *_value;
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::write_value
//       Access: Protected, Virtual
//  Description: Formats the value for human consumption on multiple
//               lines if necessary.
////////////////////////////////////////////////////////////////////
void MaterialTransition::
write_value(ostream &out, int indent_level) const {
  nassertv(_value != (const Material *)NULL);
  indent(out, indent_level) << *_value << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::register_with_read_factory
//       Access: Public, Static
//  Description: Factory method to generate a MaterialTransition object
////////////////////////////////////////////////////////////////////
void MaterialTransition::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_MaterialTransition);
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void MaterialTransition::
write_datagram(BamWriter *manager, Datagram &me) {
  OnOffTransition::write_datagram(manager, me);
  manager->write_pointer(me, (Material *)_value.p());
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::complete_pointers
//       Access: Public
//  Description: Takes in a vector of pointes to TypedWritable
//               objects that correspond to all the requests for
//               pointers that this object made to BamReader.
////////////////////////////////////////////////////////////////////
int MaterialTransition::
complete_pointers(vector_typedWritable &plist, BamReader *) {
  if (plist[0] == TypedWritable::Null) {
    if (sgattrib_cat.is_debug()) {
      sgattrib_cat->debug()
    << get_type().get_name() << " received null Material,"
    << " turning off" << endl;
    }
    _value = (const Material *)NULL;
    set_off();

  } else {
    _value = DCAST(Material, plist[0]);
  }

  return 1;
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::make_MaterialTransition
//       Access: Protected
//  Description: Factory method to generate a MaterialTransition object
////////////////////////////////////////////////////////////////////
TypedWritable* MaterialTransition::
make_MaterialTransition(const FactoryParams &params) {
  MaterialTransition *me = new MaterialTransition;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialTransition::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void MaterialTransition::
fillin(DatagramIterator& scan, BamReader* manager) {
  OnOffTransition::fillin(scan, manager);
  manager->read_pointer(scan, this);
}
